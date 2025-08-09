#include "header.h"

typedef struct
{
	int value;
	pthread_mutex_t mutex;
}ResourceA;

typedef struct
{
	int value;
	pthread_mutex_t mutex;
}ResourceB;

typedef struct
{
	ResourceA *a;
	ResourceB *b;
}Resource;

void* exec_func1(void *arg)
{
	Resource *s = (Resource*)arg;

	//对共享资源A进行上锁
	pthread_mutex_lock(&s->a->mutex);
	sleep(1);
	printf("func1 thread id:%lx waiting for resourceB....\n",pthread_self());
	//对共享资源B进行上锁
	pthread_mutex_lock(&s->b->mutex);	
	printf("ResourceA value = %d\n",s->a->value);
	printf("ResourceB value = %d\n",s->b->value);
	pthread_mutex_unlock(&s->b->mutex);
	pthread_mutex_unlock(&s->a->mutex);

	pthread_exit(NULL);
}

void* exec_func2(void *arg)
{
	Resource *s = (Resource*)arg;

	pthread_mutex_lock(&s->a->mutex);
	sleep(1);
	printf("func2 thread id:%lx waiting for ResourceB\n",pthread_self());
	pthread_mutex_lock(&s->b->mutex);

	printf("ResourceA value = %d\n",s->a->value);
	printf("ResourceB value = %d\n",s->b->value);

	pthread_mutex_unlock(&s->b->mutex);
	pthread_mutex_unlock(&s->a->mutex);

/*
	//对共享资源B进行上锁
	pthread_mutex_lock(&s->b->mutex);
	sleep(1);
	printf("func2 thread id:%lx waiting for resourceA....\n",pthread_self());
	//对共享资源A进行上锁
	pthread_mutex_lock(&s->a->mutex);

	printf("ResourceA value = %d\n",s->a->value);
	printf("ResourceB value = %d\n",s->b->value);

	pthread_mutex_unlock(&s->a->mutex);
	pthread_mutex_unlock(&s->b->mutex);
*/
	pthread_exit(NULL);
}

int main(void)
{
	int err = -1;
	pthread_t func1, func2;
	ResourceA a;
	ResourceB b;
	Resource arg = {&a, &b};
	
	a.value = 100;
	b.value = 200;
	pthread_mutex_init(&a.mutex, NULL);
	pthread_mutex_init(&b.mutex, NULL);

	if((err = pthread_create(&func1, NULL, exec_func1, (void*)&arg)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	if((err = pthread_create(&func2, NULL, exec_func2, (void*)&arg)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	pthread_join(func1, NULL);
	pthread_join(func2, NULL);

	pthread_mutex_destroy(&a.mutex);
	pthread_mutex_destroy(&b.mutex);

	return 0;
}
