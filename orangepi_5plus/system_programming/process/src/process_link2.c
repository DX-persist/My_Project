#include "header.h"

int main(int argc, char **argv)
{
	pid_t pid;
	int cnt;

	if(argc < 2)
	{
		pid = fork();
		if(pid < 0)
		{
			perror("fork error");
			exit(EXIT_FAILURE);
		}	
		else if(pid > 0)
		{
			printf("this is parent process and the pid is %d, the father's pid is %d, and the return value is %d\n",getpid(),getppid(),pid);
			sleep(1);
		}
		else
		{
			printf("this is child process and the pid is %d, the father's pid is %d, and the return value is %d\n",getpid(),getppid(),pid);
		}
	}
	else
	{
		int i;
		
		cnt = atoi(argv[1]);
		printf("first process pid is %d and the father's pid is %d\n",getpid(),getppid());
		for(i=1;i<cnt;i++)
		{
			pid = fork();
			if(pid < 0)
			{
				perror("fork error");
				exit(EXIT_FAILURE);
			}
			else if(pid > 0)
			{
				sleep(1);
				break;
			}

			printf("pid is %d, father's pid is %d\n",getpid(),getppid());
		}		
	}
	while(1);
	return 0;
}
