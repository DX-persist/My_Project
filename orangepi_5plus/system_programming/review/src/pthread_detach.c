#include "header.h"

typedef struct
{
	int time;
	int cnt;
}ARG;

void exec_func(void *arg)
{
	ARG *r = (ARG*)arg;
	int i;

	for(i=0;i<=r->cnt;i++)
	{
		printf("[thread id:%lx] i = %d\n",pthread_self(),i);
		usleep(r->time);
	}
	printf("[thread id:%lx] finished\n",pthread_self());

	pthread_exit(NULL);
}

int main(void)
{
	int err = -1;
	ARG arg_s;
	ARG arg_m;
	pthread_t detach_tid;
	pthread_attr_t attr;

	srand48(time(NULL));		//设置随机数种子
	
	arg_s.cnt = 9;
	arg_s.time = drand48() * 1000000;
	arg_m.cnt = 9;
	arg_m.time = drand48() * 500000;

	pthread_attr_init(&attr);		//初始化线程属性
						
	set_stat(&attr, PTHREAD_CREATE_DETACHED);		//设置分离属性为分离线程

	if((err = pthread_create(&detach_tid, &attr, (void*)exec_func, (void*)&arg_s)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	printf("main control thread id is %lx\n",pthread_self());

	get_stat(&attr);		//获取线程的分离属性
	
	exec_func((void*)&arg_m);

	pthread_attr_destroy(&attr);

	return 0;
}
