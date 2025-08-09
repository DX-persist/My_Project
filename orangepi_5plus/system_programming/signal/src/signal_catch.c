#include "header.h"

void sig_handler(int signum)
{
	printf("catch signal number is %d\n",signum);
}

int main(void)
{
	printf("pid = %d\n",getpid());

	if(signal(SIGUSR1,sig_handler) == SIG_ERR)
	{
		perror("signal SIGUSR1 error");
	}

	if(signal(SIGUSR2,sig_handler) == SIG_ERR)
	{
		perror("signal SIGUSR2 error");
	}

	if(signal(SIGHUP,sig_handler) == SIG_ERR)
	{
		perror("signal SIGHUP error");
	}
	
	if(signal(SIGKILL,sig_handler) == SIG_ERR)
	{
		perror("signal  SIGUSR2 error");
	}

	if(signal(SIGSTOP,sig_handler) == SIG_ERR)
	{
		perror("signal  SIGUSR2 error");
	}

	while(1)
	{
		sleep(1);
	}

	return 0;
}
