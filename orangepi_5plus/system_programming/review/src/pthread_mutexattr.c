#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage:%s [normal | error | recursive]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int err = -1;
	pthread_mutexattr_t mutexattr;
	pthread_mutex_t mutex;

	pthread_mutexattr_init(&mutexattr);		//初始化互斥锁属性
	
	if(!strcmp(argv[1], "normal"))
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_NORMAL);		//设置互斥锁属性为标准互斥锁
	else if(!strcmp(argv[1], "error"))
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_ERRORCHECK);		//设置互斥锁属性为检错互斥锁
	else if(!strcmp(argv[1], "recursive"))	
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);			//设置互斥锁属性为递归互斥锁	
	else
		fprintf(stdout, "unknown type\n");

	pthread_mutex_init(&mutex, &mutexattr);		//按上边的互斥锁属性创建互斥锁

	//创建不同的互斥锁后，第二次上锁有不同的结果
	puts("first locked...");	
	if((err = pthread_mutex_lock(&mutex)) != 0)
		fprintf(stderr,"failed to lock shared recource\n");
	else
		fprintf(stdout,"successfully locked\n");
	
	puts("second locked....");
	if((err = pthread_mutex_lock(&mutex)) != 0)
		fprintf(stderr,"failed to lock shared recource\n");
	else
		fprintf(stdout,"successfully locked\n");
	
	pthread_mutex_unlock(&mutex);		//释放互斥锁	
	pthread_mutex_unlock(&mutex);		//释放互斥锁	
	

	pthread_mutexattr_destroy(&mutexattr);		//销毁互斥锁属性
	pthread_mutex_destroy(&mutex);		//销毁互斥锁


	return 0;
}
