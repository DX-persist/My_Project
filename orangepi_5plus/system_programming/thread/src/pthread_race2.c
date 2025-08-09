#include "header.h"

typedef struct
{
	int start;
	int end;
	int time;
	char name[32];
}Arg;

void* exec_func(void *arg)
{
	Arg *r = (Arg*)arg;
	int i = r->start;
	
	for(; i <= r->end; i++)
	{
		printf("[thread name:%s thread id:%lx] i = %d\n",r->name,pthread_self(),i);
		usleep(r->time);
	}

	return NULL;
}

int main(void)
{
	int err = -1;
	pthread_t rabbit, turtle;

	srand48(time(NULL));		//设置随机数种子
	Arg r_g = {5, 55, drand48() * 100000, "rabbit thread"};		//定义兔子线程的起始、结束、延时时间、线程名称
	Arg t_g = {10, 60, drand48() * 150000, "turtle thread"};	//定义乌龟线程的起始、结束、延时时间、线程名称

	if((err = pthread_create(&rabbit, NULL, exec_func, (void*)&r_g)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	if((err = pthread_create(&turtle, NULL, exec_func, (void*)&t_g)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	printf("main control thread id is %lx\n",pthread_self());

	pthread_join(rabbit, NULL);
	pthread_join(turtle, NULL);

	printf("program finished.\n");

	return 0;
}
