#include "header.h"

typedef struct
{
	int cnt;
	int time;
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
	ARG arg_c;
	ARG arg_m;
	pthread_t detach_tid;
	pthread_attr_t attr;

	srand48(time(NULL));		//设置随机数种子

	arg_c.cnt = 9;
	arg_c.time = drand48() * 1000000;
	arg_m.cnt = 9;
	arg_m.time = drand48() * 500000;
 
	pthread_attr_init(&attr);	//初始化线程属性
	
	set_stat(&attr, PTHREAD_CREATE_DETACHED);	//设置线程为分离线程

	printf("main control thread id is %lx\n",pthread_self());
	
	if((err = pthread_create(&detach_tid, &attr, (void*)exec_func, (void*)&arg_c)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}
	
	get_stat(&attr);		//获取线程属性

	/*if((err = pthread_join(detach_tid, NULL)) != 0)
	{
		fprintf(stderr, "%s|%s|%d|%s\n",__FILE__,__func__,__LINE__,strerror(err));
	}*/

	exec_func((void*)&arg_m);		//主线程也调用此函数，当子线程被设置为分离线程后，主线程也会运行，可以看到两个线程交替运行的情况

	pthread_attr_destroy(&attr);		//销毁线程属性

	return 0;
}
