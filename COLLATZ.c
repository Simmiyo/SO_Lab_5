#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
//#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>

int main(int argc,char *argv[])
{
	char sir[256],sir1[10];
	char shm_name[]="myshm";
	int shm_fd;
	shm_fd=shm_open(shm_name,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);
	if(shm_fd<0)
	{
		perror("shm");
		return errno;
	}
	size_t dim=(argc-1)*4096;
	if(ftruncate(shm_fd,dim)<0)
	{
		perror("ftruncate");
		shm_unlink(shm_name);
		return errno;
	}
	printf("Starting Parent %d\n",getpid());
	for(int i=1;i<argc;i++)
	{
		pid_t pid=fork();
		if(pid<0)
		{
			perror("copil");
			return errno;
		}
		else if(pid==0)
		{
			char *shm_ptr=mmap(0,4096,PROT_WRITE,MAP_SHARED,shm_fd,(i-1)*4096);
			if(shm_ptr==MAP_FAILED)
			{
				perror("mmap_W");
				return errno;
			}
			int n=atoi(argv[i]);
			strcpy(sir,argv[i]);
			strcat(sir,": ");
			while(n>1)
			{
				sprintf(sir1,"%d ",n);
				strcat(sir,sir1); 
				strcat(sir," ");
				if(n%2==0)
			   	     n=n/2;
				else
			  	     n=3*n+1;
			}
			strcat(sir,"1\n");
			sprintf(shm_ptr,"%s",sir);
			printf("Done Parent %d   Me %d\n",getppid(),getpid());
			munmap(shm_ptr,4096);
			exit(0);
		}
	}
	while(waitpid(-1,0,0)>0);
	for(int i=1;i<argc;i++)
	{
		char *shm_ptr=mmap(0,4096,PROT_READ,MAP_SHARED,shm_fd,(i-1)*4096);
		if(shm_ptr==MAP_FAILED)
		{
			perror("mmap_R");
			return errno;
		}
		printf("%s",shm_ptr);
		munmap(shm_ptr,4096);
	}
	shm_unlink(shm_name);
	return 0;
}
