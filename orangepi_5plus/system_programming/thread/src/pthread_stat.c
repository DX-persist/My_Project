#include "header.h"

void get_stat(pthread_attr_t *attr)
{
	int state;

	if(pthread_attr_getdetachstate(attr, &state) != 0)		//获取线程的分离属性，然后将属性存储在state所指向的空间里
	{
		perror("pthread_attr_getdetachstate error");
	}
	else
	{
		if(state == PTHREAD_CREATE_JOINABLE)
			printf("joinable thread\n");
		else if(state == PTHREAD_CREATE_DETACHED)
			printf("detached thread\n");
		else
			printf("error type\n");
	}
}

void set_stat(pthread_attr_t *attr, int detachstate)
{
	if(pthread_attr_setdetachstate(attr, detachstate) != 0)
	{
		perror("pthread_attr_setdetachstate error");
	}
}
