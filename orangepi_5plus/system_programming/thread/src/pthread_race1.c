#include "header.h"

void* exec_func(void *arg)
{
	int cnt = *((int *)arg);
	int i;
	int tim;

	srand48(time(NULL));		//设置随机数种子
	
	for(i=1; i<=cnt; i++)
	{
		tim = drand48() * 200000;		//drand48()会获取0-1之间的随机数
		printf("pthread id:%lx i = %d\n",pthread_self(),i);
		usleep(tim);		//将tim作为参数传进去就是毫秒级延时
	}
	
	return NULL;
}

int main(void)
{
	int err = -1;
	int cnt = 50;
	pthread_t rabbit,turtle;
	
	//创建rabbit线程
	if((err = pthread_create(&rabbit, NULL, exec_func, (void*)&cnt)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	//创建turtle线程
	if((err = pthread_create(&turtle, NULL, exec_func, (void*)&cnt)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}
	
	printf("main control pthread id is %lx\n",pthread_self());

	//调用pthread_join函数的线程将会阻塞，直到所有的线程都退出被阻塞的线程才会由等待状态
	//转为就绪状态，然后根据系统调度变为运行状态
	pthread_join(rabbit, NULL);
	pthread_join(turtle, NULL);

	printf("finished\n");

	return 0;
}
