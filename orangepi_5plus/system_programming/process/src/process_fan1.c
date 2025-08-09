#include "header.h"

int main(int argc, char **argv)
{
	int i,cnt;
	pid_t pid;

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
			printf("this is parent's process: %d and the father's pid is %d, the return value is %d\n",getpid(),getppid(),pid);
			sleep(1);
		}
		else
		{
			printf("this is son's process: %d and the father's pid is %d, the return value is %d\n",getpid(),getppid(),pid);
		}
	}
	else
	{
		cnt = atoi(argv[1]);
		printf("father's pid is %d and the bash pid is %d\n",getpid(),getppid());

		for(i=1;i<cnt;i++)
		{
			pid = fork();
			if(pid < 0)
			{
				perror("fork error");
				exit(EXIT_FAILURE);
			}
			else if(pid == 0)
			{
				break;
			}
		}	
	}
	printf("own pid is %d and the father's pid is %d\n",getpid(),getppid());

	while(1);

	return 0;
}
