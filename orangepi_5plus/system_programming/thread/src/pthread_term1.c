#include "header.h"

typedef struct
{
	int data1;
	int data2;
}ARG;

void* exec_func(void *arg)
{
	ARG *r = (ARG*)arg;		//将主线程传过来的参数转为ARG类型
	int *retval = (int *)malloc(sizeof(int));		//在堆上开辟空间，不会随线程退出而释放
	if(retval == NULL)
	{
		perror("malloc error");
		exit(EXIT_FAILURE);
	}

	*retval = r->data1 + r->data2;

	pthread_exit((void*)retval);		//将结果返回给主线程，主线程使用pthread_join函数进行接收
}

int main(void)
{
	int err = -1;
	int *retval = NULL;
	pthread_t tid;

	ARG arg = {10, 30};

	if((err = pthread_create(&tid, NULL, exec_func, (void*)&arg)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}
	
	if(pthread_join(tid, (void **)&retval) != 0)	
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}
	
	//经过pthread_join函数以后修改了retval指针变量的指向，指向了子线程40的那片空间	
	if(retval != NULL)
	{
		printf("receive the retval from the thread is %d\n",*retval);
		free(retval);		//当用完以后将堆空间释放，防止造成内存泄漏
		retval = NULL;
	}

	return 0;
}
