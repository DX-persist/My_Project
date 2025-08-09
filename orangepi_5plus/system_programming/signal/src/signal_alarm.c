#include "header.h"
#include <wiringPi.h>

#define PIN 1

void sig_handler(int signum)
{
	static int i = 0;
	if(signum == 14)	
		printf("receive a signal is SIGALRM, time out\n");
	else if(signum == 17)
	{
		printf("child process exit\n");
		wait(NULL);
	}
	i++;

	if(i % 2 == 0)
		digitalWrite(PIN,HIGH);
	else
		digitalWrite(PIN,LOW);
	
	alarm(5);
}

int main(void)
{
	pid_t pid;

	//向内核注册信号处理函数，如果产生SIGALRM就立即去执行信号处理函数
	if(signal(SIGALRM, sig_handler) == SIG_ERR)
	{
		perror("signal error");
	}

	if(signal(SIGCHLD, sig_handler) == SIG_ERR)
	{
		perror("signal error");
	}

	if(wiringPiSetup() == -1)
	{
		perror("init wiringPi failed");
		exit(EXIT_FAILURE);
	}

	pinMode(PIN,OUTPUT);

	if((pid = fork()) < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{
		printf("this is child process,pid:%d ppid:%d\n",getpid(),getppid());
		//子进程一直在计数，计数时间为15秒	
		int i;
		for(i=0;i<15;i++)
		{
			printf("child process:%d i = %d\n",getpid(),i);
			sleep(1);
		}
	}
	else
	{
		printf("this is parent process,pid:%d child's pid:%d\n",getpid(),pid);
		alarm(5);	//父进程调用alarm函数设定发出SIGALRM信号为5秒

		int i;
		for(i=0;i<16;i++)
		{
			printf("parent process:%d i = %d\n",getpid(),i);
			sleep(1);
		}
	}

	return 0;
}
