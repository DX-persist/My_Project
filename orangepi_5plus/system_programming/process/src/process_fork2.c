#include "header.h"

int main()
{
	pid_t pid;

	pid = fork();

	if(pid > 0)
	{
		int i;
		for(i=0;i<10;i++)
		{
			printf("this is parent process, parent's pid is %d, parent's father's pid is %d, pid is %d\n",getpid(),getppid(),pid);
			sleep(1);
		}
	}
	else if(pid == 0)
	{
		int i;
		for(i=0;i<10;i++)
		{
			printf("this is child process, child's pid is %d, child's father's pid is %d, pid is %d\n",getpid(),getppid(),pid);
			sleep(1);
		}
	}
	else
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}

	return 0;
}
