#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage: %s [normal | error | recursive]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
	
	int err = -1;
	pthread_mutex_t mutex;
	pthread_mutexattr_t mutexattr;

	pthread_mutexattr_init(&mutexattr);

	if(!strcmp(argv[1],"normal"))
	{
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_NORMAL);
	}
	else if(!strcmp(argv[1], "error"))
	{
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);
	}
	else if(!strcmp(argv[1],"recursive"))
	{
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
	}

	pthread_mutex_init(&mutex, &mutexattr);

	if((err = pthread_mutex_lock(&mutex)) != 0)
	{
		fprintf(stderr,"failed to lock %s\n",strerror(err));
	}
	else
	{
		fprintf(stdout,"successfully locked\n");
	}

	if((err = pthread_mutex_lock(&mutex)) != 0)
	{
		fprintf(stderr,"failed to lock %s\n",strerror(err));
	}
	else
	{
		fprintf(stdout,"successfully locked\n");
	}
	
	pthread_mutex_unlock(&mutex);
	pthread_mutex_unlock(&mutex);

	pthread_mutexattr_destroy(&mutexattr);
	pthread_mutex_destroy(&mutex);

	return 0;
}
