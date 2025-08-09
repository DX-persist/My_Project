#include "header.h"

void sig_handler(int signum)
{
	printf("process %d receive a signal is %d\n",getpid(),signum);
}

void exit_handler(int num)
{
	int i;

	for(i=0;i<num;i++)
	{
		printf("process %d data %d\n",getpid(),i);
		sleep(2);
	}
}

int main(void)
{
	//向内核登记信号处理函数
	if(signal(SIGCHLD, sig_handler) == SIG_ERR)
	{
		perror("signal error");
	}

	pid_t pid;

	if((pid = fork()) < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{
		exit_handler(10);
	}
	else
	{
		exit_handler(60);
	}

	return 0;
}
