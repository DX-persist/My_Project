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
		else if(pid > 0)		//进程链的结构是父创子，子创孙，所以要中间衍生出来的父进程要退出循环，不让父进程再创建子进程
			break;
	}
	printf("pid is %d and the parent pid is %d\n",getpid(),getppid());
	while(1);
	return 0;
}
