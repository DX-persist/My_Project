#include "header.h"

void get_stat(pthread_attr_t *attr)
{
	int detachstate;

	if(pthread_attr_getdetachstate(attr, &detachstate) != 0)
	{
		perror("pthread_attr_getdetachstate error");
	}
	else
	{
		if(detachstate == PTHREAD_CREATE_JOINABLE)
		{
			printf("joinable thread\n");
		}
		else if(detachstate == PTHREAD_CREATE_DETACHED)
		{
			printf("detached thread\n");
		}
		else
		{
			printf("error type\n");
		}
	}
}

void set_stat(pthread_attr_t *attr, int detachstate)
{
	if(pthread_attr_setdetachstate(attr, detachstate) != 0)
	{
		perror("pthread_attr_setdetachstate error");
	}
}
