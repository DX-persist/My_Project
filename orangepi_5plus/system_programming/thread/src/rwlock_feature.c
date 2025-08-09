#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr, "usage: %s [r|w] [r|w]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	pthread_rwlock_t rwlock;		//定义读写锁类型
	int err = -1;

	pthread_rwlock_init(&rwlock, NULL);		//初始化读写锁

	if(!strcmp(argv[1], "r"))
	{
		if((err = pthread_rwlock_rdlock(&rwlock)) != 0)	
			fprintf(stderr,"first failed to create rdlock:%s\n",strerror(err));
		else
			fprintf(stdout,"first successfully create rwlock...\n");	
	}
	else if(!strcmp(argv[1], "w"))
	{
		if((err = pthread_rwlock_wrlock(&rwlock)) != 0)
			fprintf(stderr,"first failed to create wrlock:%s\n",strerror(err));
		else
			fprintf(stdout,"first successfully create wrlock...\n");
	}

	if(!strcmp(argv[2], "r"))
	{
		if((err = pthread_rwlock_rdlock(&rwlock)) != 0)	
			fprintf(stderr,"second failed to create rdlock:%s\n",strerror(err));
		else
			fprintf(stdout,"second successfully create rwlock...\n");	
	}
	else if(!strcmp(argv[2], "w"))
	{
		if((err = pthread_rwlock_wrlock(&rwlock)) != 0)
			fprintf(stderr,"second failed to create wrlock:%s\n",strerror(err));
		else
			fprintf(stdout,"second successfully create wrlock...\n");
	}

	pthread_rwlock_unlock(&rwlock);		//释放读写锁
	pthread_rwlock_unlock(&rwlock);		//释放读写锁

	pthread_rwlock_destroy(&rwlock);		//销毁读写锁

	return 0;
}
