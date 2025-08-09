#include "header.h"

void print_status(int status)
{
	if(WIFEXITED(status))
	{
		printf("child process exit normally and the status is %d\n",WEXITSTATUS(status));
	}
	else if(WIFSIGNALED(status))
	{
		printf("child process exit abnormally and the signal number is %d\n",WTERMSIG(status));
	}
	else if(WIFSTOPPED(status))
	{
		printf("child process stopped and the signal number is %d\n",WSTOPSIG(status));
	}
	else
	{
		printf("unknown status\n");
	}
}	

int main()
{
	pid_t pid;
	int status;

	pid = fork();
	if(pid < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{
		printf("this is child process,pid is %d and the parent pid is %d\n",getpid(),getppid());
		exit(EXIT_SUCCESS);
	}
	
	wait(&status);
	print_status(status);
	printf("--------------------------------------------------------------\n");

	pid = fork();
	if(pid < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{
		printf("this is child process,pid is %d and the parent pid is %d\n",getpid(),getppid());
//		int i = 3,j = 0;
//		printf("%d\n",i/j);
		char *p = NULL;
		*p = 'h';	
		puts(p);
	}

	wait(&status);
	print_status(status);
	printf("--------------------------------------------------------------\n");

	pid = fork();
	if(pid < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{
		printf("this is child process,pid is %d and the parent pid is %d\n",getpid(),getppid());
		pause();
	}
	wait(&status);
	print_status(status);

	return 0;
}
