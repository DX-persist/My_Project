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
	else if(pid == 0)
	{
		printf("pid is %d and the parent pid is %d\n",getpid(),getppid());	//子进程优于父进程先退出导致子进程仍保留了进程表项用来记载进程的状态
		exit(EXIT_FAILURE);
	}
	else			//父进程一在在做死循环
		wait(NULL);

	return 0;
}
