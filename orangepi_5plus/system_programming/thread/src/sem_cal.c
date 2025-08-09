#include "header.h"

typedef struct
{
	int result;
	sem_t sem;
}OperArg;

void* cal_func(void *arg)
{
	OperArg *s = (OperArg*)arg;
	int i = 1;
	
	for(; i<= 100; i++)
	{
		s->result += i;
	}
	printf("[cal thread id:%lx] write [%d] to the structure\n",pthread_self(),s->result);

	//将信号量作加1操作，使得被此信号量阻塞的线程能够执行，同时也表明当前
	//线程已经将结果计算出来并将结果存放到结构体中了
	sem_post(&s->sem);

	pthread_exit(NULL);
}

void* get_func(void *arg)
{
	OperArg *s = (OperArg*)arg;
	
	//由于信号量的初值为0，所以当执行到这里的时候就会阻塞，等待sem_post对信号量进行
	//加1操作来唤醒此线程，同时也表明计算结果线程已经完成了
	sem_wait(&s->sem);
	
	printf("[get thread id:%lx] read [%d] from the structure\n",pthread_self(),s->result);

	pthread_exit(NULL);
}

int main(void)
{
	int err = -1;
	pthread_t cal, get;
	OperArg arg;

	memset(&arg, 0, sizeof(arg));		//初始化结构体
	sem_init(&arg.sem, 0, 0);			//初始化信号量，信号量为当前进程中的所有线程使用，且信号量的初值为0

	if((err = pthread_create(&cal, NULL, cal_func, (void*)&arg)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	if((err = pthread_create(&get, NULL, get_func, (void*)&arg)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	pthread_join(cal, NULL);
	pthread_join(get, NULL);

	sem_destroy(&arg.sem);				//销毁信号量

	return 0;
}
