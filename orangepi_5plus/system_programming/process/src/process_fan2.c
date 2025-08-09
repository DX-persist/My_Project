#include "header.h"

int main(int argc, char **argv)
{
	int i,cnt;
	pid_t pid;

	if(argc < 2)
		cnt = 2;
	else
		cnt = atoi(argv[1]);

	for(i=1;i<cnt;i++)
	{
		pid = fork();
		if(pid < 0)
		{
			perror("fork error");
			exit(EXIT_FAILURE);
		}	
		else if(pid == 0)		//进程扇的结构是父进程一直在创子进程，所以要保证循环中子进程不会再创建子进程
			break;
	}
	printf("pid is %d and the parent pid is %d\n",getpid(),getppid());
	while(1);
	return 0;
}
