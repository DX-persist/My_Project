#include "header.h"

int main(void)
{
	pid_t pid;

	printf("parent pid is %d\n",getpid());
	pid = fork();
	if(pid > 0)
	{
		printf("this is parent process, getpid: %d getppid: %d pid: %d\n",getpid(),getppid(),pid);
	}
	else if(pid == 0)
	{
		printf("this is child process,getpid: %d getppid: %d pid: %d\n",getpid(),getppid(),pid);
	}
	else
	{
		perror("fork error");
	}

	printf("pid is %d\n",getpid());

	return 0;
}
