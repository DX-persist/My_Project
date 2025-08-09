#include "header.h"

int main()
{	
	pid_t pid;
	
	pid = fork();
	if(pid < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid > 0)
	{
		printf("parent's pid is %d and the son's pid is %d\n",getpid(),pid);
		exit(EXIT_SUCCESS);
	}
	else
	{
		sleep(3);		//休眠3秒，保证父进程先退出，使子进程变成孤儿进程，从而被init进程收留
		printf("son's pid is %d, and the parent pid is %d\n",getpid(),getppid());
	}

	return 0;
}
