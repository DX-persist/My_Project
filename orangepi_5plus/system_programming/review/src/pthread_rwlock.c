#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"usage:%s [r|w][r|W]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	pthread_rwlock_t rwlock;		//定义读写锁类型
	int err = -1;

	pthread_rwlock_init(&rwlock, NULL);		//对读写锁进行初始化
	
	
	//使用外部传参创建读写锁
	if(!strcmp(argv[1], "r"))
	{
		if((err = pthread_rwlock_rdlock(&rwlock)) != 0)
			fprintf(stderr, "first failed to create rdlcok:%s\n",strerror(err));
		else
			fprintf(stdout, "first successfully create rdlock\n");
	}
	else if(!strcmp(argv[1], "w"))
	{
		if((err = pthread_rwlock_wrlock(&rwlock)) != 0)
			fprintf(stderr, "first failed to create wrlcok:%s\n",strerror(err));
		else
			fprintf(stdout, "first successfully create wrlock\n");
	}

	//第二次创建读写锁
	if(!strcmp(argv[2], "r"))
	{
		if((err = pthread_rwlock_rdlock(&rwlock)) != 0)
			fprintf(stderr, "second failed to create rdlcok:%s\n",strerror(err));
		else
			fprintf(stdout, "second successfully create rdlock\n");
	}
	else if(!strcmp(argv[2], "w"))
	{
		if((err = pthread_rwlock_wrlock(&rwlock)) != 0)
			fprintf(stderr, "second failed to create wrlcok:%s\n",strerror(err));
		else
			fprintf(stdout, "second successfully create wrlock\n");
	}


	pthread_rwlock_unlock(&rwlock);		//释放读写锁
	pthread_rwlock_unlock(&rwlock);		//释放读写锁

	pthread_rwlock_destroy(&rwlock);		//对读写锁进行销毁

	return 0;
}
