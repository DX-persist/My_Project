#include "header.h"

typedef struct
{
	int data1;
	int data2;
	pthread_t tid;
}ARG;

void* exec_func(void *arg)
{
	ARG *retval = (ARG *)malloc(sizeof(ARG));
	if(retval == NULL)
	{
		perror("malloc error");
		exit(EXIT_FAILURE);
	}

	retval->data1 = 10;
	retval->data2 = 20;
	retval->tid = pthread_self();
	
	printf("the child thread id is %lx\n",retval->tid);
	return (void*)retval;
}

int main(void)
{
	int err = -1;
	pthread_t tid;
	ARG *retval = NULL;

	if((err = pthread_create(&tid, NULL, exec_func, NULL)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	if(pthread_join(tid, (void **)&retval) != 0)
	{
		perror("pthread_join error");
		exit(EXIT_FAILURE);
	}

	if(retval != NULL)
	{
		printf("child thread id is %lx, sum:%d\n",retval->tid,(retval->data1 + retval->data2));
		free(retval);
		retval = NULL;
	}	

	return 0;	
}
