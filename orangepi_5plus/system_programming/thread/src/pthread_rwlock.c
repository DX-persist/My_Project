#include "header.h"

typedef struct
{
	int data;
	char buffer[32];
	pthread_rwlock_t rwlock;
}OperArg;

void* read_data(void *arg)
{
	OperArg *r = (OperArg*)arg;
	int i;
	pthread_rwlock_rdlock(&r->rwlock);
	
	for(i=0;i<3;i++)
	{
		printf("[thread id:%lx data = %d]\n",pthread_self(),r->data);
		printf("thread is sleeping....\n");
		sleep(1);
	}
	pthread_rwlock_unlock(&r->rwlock);

	pthread_exit(NULL);
}

void* write_data(void *arg)
{
	OperArg *r = (OperArg*)arg;
	int i = 3;
	pthread_rwlock_wrlock(&r->rwlock);
	strcpy(r->buffer,"hello world");
	printf("write thread id:%lx context:%s\n",pthread_self(),r->buffer);
	pthread_rwlock_unlock(&r->rwlock);
	
	pthread_exit(NULL);
}

int main(void)
{
	int err = -1;
	pthread_t w1,w2,w3;
	OperArg arg;

	memset(&arg,'\0',sizeof(arg));
	arg.data = 10;

	pthread_rwlock_init(&arg.rwlock, NULL);

	//创建读锁线程用来读取结构体中的数据
	if((err = pthread_create(&w1, NULL, read_data, (void*)&arg)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}
	if((err = pthread_create(&w3, NULL, read_data, (void*)&arg)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	sleep(1);
	//创建写锁线程向结构体内部写入数据
	if((err = pthread_create(&w2, NULL, write_data, (void*)&arg)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}
	else
	{
		printf("main control create thread successfully\n");
	}
	
	pthread_join(w1, NULL);
	pthread_join(w2, NULL);
	pthread_join(w3, NULL);

	pthread_rwlock_destroy(&arg.rwlock);

	return 0;
}
