#include "header.h"

void cleanup_handler(void *arg)
{
	char *str = (char*)arg;

	printf("clean func: %s\n",str);
}

void* exec_func(void *arg)
{
	int execute = *(int *)arg;

	pthread_cleanup_push(cleanup_handler,"first execute...");		//将线程清理函数压入栈中
	pthread_cleanup_push(cleanup_handler,"second execute...");

	printf("thread is running\n");

	pthread_cleanup_pop(execute);			//将线程清理函数弹出栈中，并根据execute的值判断是否执行线程清理函数
	pthread_cleanup_pop(execute);

	return NULL;
}

int main(void)
{
	int err = -1;
	int execute1 = 1;
	int execute2 = 0;

	pthread_t tid1, tid2;
	
	if((err = pthread_create(&tid1, NULL, exec_func, (void*)&execute1)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	pthread_join(tid1, NULL);

	if((err = pthread_create(&tid2, NULL, exec_func, (void*)&execute2)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	pthread_join(tid2, NULL);

	return 0;
}
