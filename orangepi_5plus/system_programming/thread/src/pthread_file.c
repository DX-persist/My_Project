#include "header.h"

typedef struct
{
	int fd;
	int is_wait;
	char str1[64];
	char str2[64];
	pthread_mutex_t mutex;
	pthread_cond_t cond;
}FileOper;

void* exec_func1(void *arg)
{
	FileOper *r = (FileOper*)arg;
	char buffer[128] = {'\0'};

	sprintf(buffer,"thread id:%lx context:%s\n",pthread_self(),r->str1);
	int len = strlen(buffer);

	pthread_mutex_lock(&r->mutex);
	r->is_wait = 1;
	pthread_cond_wait(&r->cond, &r->mutex);
	
	if(write(r->fd, buffer, len) != len)
	{
		perror("write error");
		exit(EXIT_FAILURE);
	}

	pthread_mutex_unlock(&r->mutex);

	close(r->fd);

	pthread_exit(NULL);
}

void* exec_func2(void *arg)
{
	FileOper *r = (FileOper*)arg;
	char buffer[128] = {'\0'};

	sprintf(buffer,"thread id:%lx context:%s\n",pthread_self(),r->str2);
	int len = strlen(buffer);

	
	pthread_mutex_lock(&r->mutex);
	if(write(r->fd, buffer, len) != len)
	{
		perror("write error");
		exit(EXIT_FAILURE);
	} 
	
	while(!r->is_wait)
	{
		pthread_mutex_unlock(&r->mutex);
		usleep(100);
		pthread_mutex_lock(&r->mutex);
	}

	pthread_mutex_unlock(&r->mutex);
	pthread_cond_broadcast(&r->cond);

	close(r->fd);

	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	
	if(argc < 4)
	{
		fprintf(stderr, "usage:%s [newfile_path] [string1] [string2]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int fd;
	int err = -1;
	pthread_t f1,f2;
	
	//如果文件不存在就创建文件，权限为文件的拥有者和同组人可读可写可执行
	fd = open(argv[1], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXG | S_IRWXU);		
	if(fd < 0)
	{
		fprintf(stderr, "open file error:%s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}

	FileOper arg;

	memset(&arg, '\0', sizeof(arg));
	
	arg.fd = fd;
	arg.is_wait = 0;
	strcpy(arg.str1,argv[2]);
	strcpy(arg.str2,argv[3]);
	pthread_mutex_init(&arg.mutex, NULL);
	pthread_cond_init(&arg.cond, NULL);

	if((err = pthread_create(&f1, NULL, exec_func1, (void*)&arg)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_SUCCESS);
	}

	if((err = pthread_create(&f2, NULL, exec_func2, (void*)&arg)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	printf("main control id:%lx, fd = %d\n",pthread_self(),fd);

	pthread_join(f1, NULL);
	pthread_join(f2, NULL);

	pthread_mutex_destroy(&arg.mutex);
	pthread_cond_destroy(&arg.cond);

	close(fd);

	return 0;
}
