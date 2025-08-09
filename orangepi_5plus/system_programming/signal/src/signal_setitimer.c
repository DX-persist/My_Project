#include "header.h"

void sig_handler(int signum)
{
	if(signum == SIGALRM)
		printf("receive a signal is SIGALRM and time out\n");
}

int main(void)
{
	pid_t pid;

	if(signal(SIGALRM, sig_handler) == SIG_ERR)
	{
		perror("signal error");
	}
	
	if((pid = fork()) < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{
		printf("this is child process,pid: %d ppid: %d\n",getpid(),getppid());

		int i = 0;

		while(i < 15)
		{
			printf("pid:%d i = %d\n",getpid(),++i);
			sleep(1);
		}
	}
	else
	{
		printf("this is parent process,pid:%d ,child's pid:%d\n",getpid(),pid);
		
		struct itimerval timer;

		timer.it_interval.tv_sec = 5;
		timer.it_interval.tv_usec = 0;

		timer.it_value.tv_sec = 5;
		timer.it_value.tv_usec = 0;

		if(setitimer(ITIMER_REAL,&timer,NULL) != 0)
		{
			perror("setitimer error");
		}
		
		wait(NULL);
	}

	return 0;
}
