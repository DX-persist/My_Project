#include "header.h"
#include <wiringPi.h>
#include <sys/time.h>

#define PIN 1

void sig_handler(int signum)
{
	static int i = 0;
	if(signum == 14)
		printf("receive a signal is SIGALRM,time out\n");

	if(++i % 2 == 0)		//当i对2取余等于0的时候，将引脚设置为高电平反之设置为低电平
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

		struct itimerval timer;
		timer.it_interval.tv_sec = 5;		//设定定时时间，支持微秒级
		timer.it_interval.tv_usec = 0;
		timer.it_value.tv_sec = 5;			//设定定时器在启动前经过的时间
		timer.it_value.tv_usec = 0;
		if(setitimer(ITIMER_REAL,&timer,NULL) == -1)		//设置定时器参数，使用真实时间计时器进行定时
		{
			perror("setitimer error");
			exit(EXIT_FAILURE);
		}
		
		wait(NULL);
	}

	return 0;
}

