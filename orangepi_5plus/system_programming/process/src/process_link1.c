#include "header.h"

int main(int argc, char **argv)
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
		printf("this is parent process and the pid is %d,the father's pid is %d,and the return value is son's pid:%d\n",getpid(),getppid(),pid);
		sleep(1);
	}
	else
	{
		printf("this is son's process and the pid is %d,the father's pid is %d,and the return value is %d\n",getpid(),getppid(),pid);

		pid = fork();
		if(pid < 0)
		{
			perror("fork error");
			exit(EXIT_FAILURE);
		}
		else if(pid > 0)
		{
			printf("this is son1's process and the pid is %d,the father's pid is %d,and the return value is grandson's pid:%d\n",getpid(),getppid(),pid);
			sleep(1);
		}
		else
		{
			printf("this is grandson's process and the pid is %d,the father's pid is %d,and the return value is %d\n",getpid(),getppid(),pid);
		}
	}
	return 0;
}
