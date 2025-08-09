#include "header.h"

int main(void)
{
	printf("pid: %d\n",getpid());
	printf("ppid: %d\n",getppid());
	printf("user id: %d\n",getuid());
	printf("effective user id: %d\n",geteuid());
	printf("user group id: %d\n",getgid());
	printf("effective user group id: %d\n",getegid());
	printf("process group id: %d\n",getpgrp());
	printf("process number %d belongs to process group id is %d\n",getpid(),getpgid(getpid()));
	printf("process's father number %d belongs to process group id is %d\n",getppid(),getpgid(getppid()));
	
	return 0;
}
