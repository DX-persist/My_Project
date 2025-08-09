#include "header.h"

typedef struct
{
	int value;
	int r_wait;
	int w_wait;
	pthread_mutex_t rm;
	pthread_mutex_t wm;
	pthread_cond_t rc;
	pthread_cond_t wc;
}Storage;

void* read_func(void *arg)
{
	Storage *s = (Storage*)arg;
	int i = 1;

	for(; i<= 100; i++)
	{
		//加读取锁来保护读取条件
		pthread_mutex_lock(&s->rm);
		s->r_wait = 1;					//表示读取线程已经准备好了
		pthread_cond_wait(&s->rc, &s->rm);		//读取线程阻塞等待写入线程唤醒
		
		printf("[read thread id:%lx] read [%d] from the structure\n",pthread_self(),s->value);
		
		pthread_mutex_unlock(&s->rm);		//释放读取锁										
		
		//判断写入线程是否准备好，如果准备好就给它发送信号
		pthread_mutex_lock(&s->wm);
		while(!s->w_wait)	
		{
			pthread_mutex_unlock(&s->wm);
			sleep(1);
			pthread_mutex_lock(&s->wm);
		}
		s->w_wait = 0;			//将w_wait清零，等待下一次条件满足
		pthread_cond_broadcast(&s->wc);
		pthread_mutex_unlock(&s->wm);
	}


	pthread_exit(NULL);
}

void* write_func(void *arg)
{
	Storage *s = (Storage*)arg;
	int i = 1;

	for(; i <= 100; i++)
	{
		//检测读取线程是否准备好
		pthread_mutex_lock(&s->rm);
		
		s->value = i + 100;
		printf("[write thread id:%lx] write [%d] to structure\n",pthread_self(),s->value);

		while(!s->r_wait)
		{
			pthread_mutex_unlock(&s->rm);	//如果没有准备好就释放互斥锁给读取线程修改条件的机会
			sleep(1);
			pthread_mutex_lock(&s->rm);		//如果读取线程还没有准备好就仍然上锁		
		}

		s->r_wait = 0;						//将判断读取线程的条件重新清空
		pthread_cond_broadcast(&s->rc);		//唤醒读取线程
		pthread_mutex_unlock(&s->rm);		//释放锁
	

		pthread_mutex_lock(&s->wm);		//改变w_wait条件表示写者线程已经准备好了
		s->w_wait = 1;
		pthread_cond_wait(&s->wc, &s->wm);	//将当前线程加入到等待队列中，同时等待读者线程发送信号

		pthread_mutex_unlock(&s->wm);
	}

	pthread_exit(NULL);
}

int main(void)
{
	int err = -1;
	pthread_t read,write;
	Storage s;

	memset(&s, '\0', sizeof(s));
	
	//初始化两组互斥锁和条件变量
	pthread_mutex_init(&s.rm, NULL);
	pthread_mutex_init(&s.wm, NULL);
	pthread_cond_init(&s.rc, NULL);
	pthread_cond_init(&s.wc, NULL);

	//创建读取线程用于从结构体中获取数据
	if((err = pthread_create(&read, NULL, read_func, (void*)&s)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	//创建写入线程用于向结构体中写入数据
	if((err = pthread_create(&write, NULL, write_func, (void*)&s)) != 0)
	{
		perror("pthread_create error");
		exit(EXIT_FAILURE);
	}

	//等待子线程退出
	pthread_join(read, NULL);
	pthread_join(write, NULL);


	//销毁两组互斥锁和条件变量
	pthread_mutex_destroy(&s.rm);
	pthread_mutex_destroy(&s.wm);
	pthread_cond_destroy(&s.rc);
	pthread_cond_destroy(&s.wc);


	return 0;
}
