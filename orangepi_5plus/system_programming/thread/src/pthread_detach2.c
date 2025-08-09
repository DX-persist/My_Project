#include "header.h"

void main_thread_exec_func(int cnt)
{
	int i;

	for(i=0;i<cnt;i++)
	{
		printf("main control thread id is %lx i = %d\n",pthread_self(),i);
		sleep(1);
	}
}

void* exec_func(void *arg)
{
	int cnt = *(int *)arg;
	int i;
	pthread_t *retval = (pthread_t *)malloc(sizeof(pthread_t));		//在堆区开辟内存空间，用于将子线程的id传给主线程
	if(retval == NULL)
	{
		perror("malloc error");
		exit(EXIT_FAILURE);
	}
	*retval = pthread_self();

	for(i=0;i<=cnt;i++)
	{
		printf("[thread:%lx] i = %d\n",*retval,i);
		sleep(1);
	}

	pthread_exit((void *)retval);
}

int main(void)
{
	int err = -1;
	int cnt = 8;
	pthread_t *retval = NULL;
	pthread_t default_tid, detach_tid;
	pthread_attr_t attr;		

	pthread_attr_init(&attr);		//对线程属性进行初始化


	if((err = pthread_create(&default_tid, &attr, exec_func, (void*)&cnt)) != 0)	//这里没有设置分离属性，所以它还是保持默认，后边要使用pthread_join函数回收
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}
	
	get_stat(&attr);		//获取分离分离属性状态

	set_stat(&attr, PTHREAD_CREATE_DETACHED);	//设置分离属性为分离线程

	if((err = pthread_create(&detach_tid, &attr, exec_func, (void*)&cnt)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	get_stat(&attr);

	//设置分离属性为分离的线程结束后会自动释放资源，如果此时再用pthread_join函数回收资源会报错
	/*if((err = pthread_join(detach_tid, (void*)retval)) != 0)
	{
		fprintf(stderr,"%s\n",strerror(err));
		exit(EXIT_FAILURE);
	}*/
	
	puts("detach thread");
	main_thread_exec_func(9);

	if((err = pthread_join(default_tid, (void **)&retval)) != 0)		//pthread_join函数等待子进程退出并回收其资源，同时接收来自子线程的返回值
	{
		fprintf(stderr,"%s\n",strerror(err));
	}
	else
	{
		if(retval != NULL)				//如果retval不为NULL，则说明已经获取到了子线程的返回值
		{
			printf("thread:%lx finished\n",*retval);
			free(retval);
			retval = NULL;
		}
	}

	main_thread_exec_func(9);
	
	pthread_attr_destroy(&attr);	//销毁线程属性

	return 0;
}
