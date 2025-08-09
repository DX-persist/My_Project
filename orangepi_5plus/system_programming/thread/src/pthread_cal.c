#include "header.h"

typedef struct
{
    int retult;
    int is_wait;
    pthread_mutex_t mutex;      //定义互斥锁类型
    pthread_cond_t cond;         //定义条件变量类型
}Result;

void* cal_func(void *arg)
{
    Result *r = (Result*)arg;
    int i, sum = 0;
    
    //计算1-100的和然后存放到结构体中
    for(i = 1; i <= 100; i++)
        sum += i;       
    r->retult = sum;

    printf("[cal thread id]: %lx\n",pthread_self());

    pthread_mutex_lock(&r->mutex);
    while(!r->is_wait)
    {
        //如果进入到这个循环中就说明等待线程还没有准备好，这时候就要释放互斥锁给等待的
        //线程有机会拿到互斥锁然后对共享资源进行修改，下次判断等待线程准备好就可以给等待
        //线程发送信号唤醒，让等待线程拿到计算结果并打印出来
        pthread_mutex_unlock(&r->mutex);    
        usleep(100);
        pthread_mutex_lock(&r->mutex);
    }
    pthread_mutex_unlock(&r->mutex);        //释放锁，上锁和释放锁是一一对应的
    pthread_cond_broadcast(&r->cond);       //唤醒所有在等待队列中的线程

    pthread_exit(NULL);
}

void* get_func(void *arg)
{
    Result *r = (Result*)arg;

    //加锁，对共享资源进行保护
    pthread_mutex_lock(&r->mutex);
    r->is_wait = 1;
    pthread_cond_wait(&r->cond, &r->mutex);     
    //这里传互斥锁进入是为了保证等待队列这个共享资源的安全，实际上在内部做了多次加锁释放锁的操作
    pthread_mutex_unlock(&r->mutex);

    //通过阻塞等待计算线程将结果存放到结构体中，然后被另外一个线程使用broadcast唤醒
    printf("[get thread id:%lx] sum = %d\n",pthread_self(),r->retult);

    pthread_exit(NULL);
}

int main(void)
{   
    int err = -1;
    pthread_t get,cal;

    Result r;
    memset(&r,'\0',sizeof(r));
    r.is_wait = 0;
    pthread_mutex_init(&r.mutex, NULL);      //以默认属性创建互斥锁
    pthread_cond_init(&r.cond, NULL);        //以默认属性创建条件变量

    //创建获取结果线程，计算结果从结构体中获取并打印出来
    if((err = pthread_create(&get, NULL, get_func, (void*)&r)) != 0)
    {
        perror("pthread_create error");
        exit(EXIT_FAILURE);
    }

    //创建计算结果线程，然后将结果存放到结构体中
    if((err = pthread_create(&cal, NULL, cal_func, (void*)&r)) != 0)
    {
        perror("pthread_create error");
        exit(EXIT_FAILURE);
    }

    //等待子线程退出
    pthread_join(get, NULL);
    pthread_join(cal, NULL);

    pthread_mutex_destroy(&r.mutex);      //销毁互斥锁
    pthread_cond_destroy(&r.cond);         //销毁条件变量

    return 0;
}