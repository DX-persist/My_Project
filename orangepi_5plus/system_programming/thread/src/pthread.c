#include "header.h"

void* exec_func(void *arg)
{
	int data = (int)arg;

	printf("data = %d\n",data);

	return NULL;
}

int main(void)
{
	int err = -1;
	pthread_t tid;

	if((err = pthread_create(&tid, NULL, exec_func, (void*)50)) != 0)
	{
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}
	
	printf("main control thread id is %lx\n",pthread_self());
	pthread_join(tid, NULL);

	return 0;
}
