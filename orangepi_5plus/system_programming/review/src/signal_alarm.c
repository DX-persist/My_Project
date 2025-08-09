#include "header.h"
#include <wiringPi.h>

#define PIN 1

void sig_handler(int signum)
{
	static int i = 0;
	if(signum == 14)
		printf("receive a signal is SIGALRM,time out\n");
	alarm(5);		//当进入到这个函数里就说明触发了SIGALRM信号，此时要再次设置定时时间来产生周期性的定时时间

	if(++i % 2 == 0)		//
		digitalWrite(PIN,HIGH);
	else
		digitalWrite(PIN,LOW);
}

int main(void)
{
	pid_t pid;

	//向内核登记SIGALRM信号处理函数，如果产生了这个信号就去执行相应的信号处理函数
	if(signal(SIGALRM, sig_handler) == SIG_ERR)
	{
		perror("signal error");
		exit(EXIT_FAILURE);
	}

	if(wiringPiSetup() == -1)		//初始化wiringPi库
	{
		perror("init wiring error");
		exit(EXIT_FAILURE);
	}

	//将引脚配置为输出模式
	pinMode(PIN,OUTPUT);

	if((pid = fork()) < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{
		printf("child's pid:%d ppid:%d\n",getpid(),getppid());
	
		int i = 0;

		while(i < 15)
		{
			printf("pid:%d i = %d\n",getpid(),++i);
			sleep(1);
		}	
	}
	else
	{
		printf("parent's pid:%d child's pid:%d\n",getpid(),pid);
		alarm(5);		//设定5秒后产生SIGALRM信号
		wait(NULL);
	}

	return 0;
}
