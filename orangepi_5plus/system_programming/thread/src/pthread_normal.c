#include "header.h"

void exec_func(void *arg)
{
	int cnt = *(int *)arg;
	int i;
	
	for(i=0;i<=cnt;i++)
	{
		printf("[thread id:%lx] i = %d\n",pthread_self(),i);
		sleep(1);
	}
	printf("[thread id:%lx] finished!\n",pthread_self());
	pthread_exit(NULL);
}

int main(void)
{
	int err = -1;
	int cnt = 8;
	pthread_t default_tid;
	pthread_attr_t attr;

	pthread_attr_init(&attr);		//初始化线程属性

	//设置正常启动线程(默认)，退出时需要主线程来回收子线程的资源	
	if((err = pthread_create(&default_tid, NULL, (void*)exec_func, (void*)&cnt)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}
	
	get_stat(&attr);		//将线程的分离属性打印出来查看

	if((err = pthread_join(default_tid, NULL)) != 0)
	{
		fprintf(stderr,"%s\n",strerror(err));
		exit(EXIT_FAILURE);
	}
	else
	{
		exec_func((void*)&cnt);			//主线程调用pthread_join函数被阻塞，只有当子线程退出主线程才会继续执行
	}
	
	pthread_attr_destroy(&attr);		//销毁线程属性

	return 0;
}

