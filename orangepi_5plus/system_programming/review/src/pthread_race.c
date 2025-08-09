#include "header.h"

typedef struct
{
	int start;
	int end;
	int time;
	int distance;
	char name[32];
}ARG;

void* exec_func(void *arg)
{
	int i;
	ARG *r = (ARG*)arg;
	ARG *ret = (ARG*)malloc(sizeof(ARG));
	if(ret == NULL)
	{
		perror("malloc error");
		exit(EXIT_FAILURE);
	}
	
	*ret = *r;			//将栈空间r的结构体内容拷贝一份到堆空间的ret
	for(i = r->start; i<= r->end; i++)
	{
		printf("[%s id:%lx] i = %d\n",r->name,pthread_self(),i);
		usleep(r->time);
	}

	ret->distance = ret->end - ret->start;
	
	 pthread_exit((void*)ret);
}

int main(void)
{
	int err = -1;
	pthread_t rabbit,turtle;
	ARG *retval = NULL;

	srand48(time(NULL));		//设置随机数种子
	
	ARG r_g = {5, 50, drand48() * 100000, 0, "rabbit"};		//设置主线程传给兔子线程的参数
	ARG t_g = {1, 60, drand48() * 50000, 0, "turtle"};			//设置主线程传给乌龟线程的参数														
	//drand48()函数获取0-1之间的随机数，然后传给usleep函数作为延时时间
	
	if((err = pthread_create(&rabbit, NULL, exec_func, (void*)&r_g)) != 0)		//创建兔子线程
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	if((err = pthread_create(&turtle, NULL, exec_func, (void*)&t_g)) != 0)		//创建乌龟线程
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	printf("main control thread id is %lx\n",pthread_self());		//获取主线程的线程ID

	pthread_join(rabbit, (void**)&retval);				//等待子线程退出并回收子线程的资源同时获取子线程传给主线程的返回值
	if(retval != NULL)							//判断retval不为空则说明pthread_join函数将retval指向了子线程开辟的堆空间，用完后要使用free函数释放
	{	
		printf("rabbit distance is %d\n",retval->distance);			
		free(retval);
		retval = NULL;
	}

	pthread_join(turtle, (void**)&retval);
	if(retval != NULL)
	{
		printf("turtle distance is %d\n",retval->distance);
		free(retval);
		retval = NULL;
	}

	return 0;
}
