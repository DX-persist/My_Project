#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>

typedef struct reader_writer_file
{
    int semid;
    int fd;
    char pathname[32];
}Storage;

//初始化信号量集和信号量集中的信号量的初值
int init_sem(Storage *s)
{
    assert(s != NULL);

    //创建信号量集，信号量的个数为2，信号量的权限为IPC_CREAT | IPC_EXCL | 0774
    s->semid = semget(IPC_PRIVATE, 2, IPC_CREAT | IPC_EXCL | 0774);
    if(s->semid < 0)
    {
        return -1;
        perror("semget error");
        exit(EXIT_FAILURE);
    }

    union semun
    {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    };
    union semun un;
    unsigned short array[2] = {0, 0};
    un.array = array;
    //对信号量集中的信号量值进行初始化，0表示要操作所有的信号量，SETALL表示要设置所有
    //的信号量初值，要设置的初值存放到un联合体中
    if(semctl(s->semid, 0, SETALL, un) < 0)
    {
        perror("semctl error");
        exit(EXIT_FAILURE);
    }

    return s->semid;
}

void write_func(Storage *s, int value)
{
    assert(s != NULL);

    //以只写的权限打开文件，若文件存在则报错并设置errno为EEXIST
    s->fd = open(s->pathname, O_CREAT | O_EXCL | O_WRONLY | O_TRUNC, 0774);
    if(s->fd < 0)
    {
        perror("open file error");
        exit(EXIT_FAILURE);
    }

    //构造写入文件中的数据
    time_t Time = time(NULL);
    char *str1 = ctime(&Time);
    char *str2 = "hello world";
    char buffer[128];

    sprintf(buffer,"time:%stimes:[%d] context:[%s]",str1,value,str2);
    printf("write process:%d write:%s\n",getpid(),buffer);

    //将数据写入到文件中去
    if(write(s->fd, buffer, sizeof(buffer)) != sizeof(buffer))
    {
        perror("write error");
        exit(EXIT_FAILURE);
    }
    close(s->fd);       //关闭文件

    //设置0号信号量用于V(1)操作，SEM_UNDO表示若进程异常退出，会取消当前对信号量的操作
    //恢复到信号量的上一次状态，防止信号量造成资源的死锁
    struct sembuf semops_v[1] = {{0, 1, SEM_UNDO}};
    //设置1号信号量用于P(1)操作
    struct sembuf semops_p[1] = {{1, -1, SEM_UNDO}};

    //写入进程写完后作V(1)操作，通知读取进程开始读取
    if(semop(s->semid, semops_v, sizeof(semops_v) / sizeof(semops_v[0])) < 0)
    {
        perror("semop error");
        exit(EXIT_FAILURE);
    }

    //写入进程作P(1)操作，等待读取进程读取完成，表示可以开始下一轮的写入
    if(semop(s->semid, semops_p, sizeof(semops_p) / sizeof(semops_p[0])) < 0)
    {
        perror("semop error");
        exit(EXIT_FAILURE);
    }
}

void read_func(Storage *s)
{
    assert(s != NULL);

    //设置0号信号量作P(1)操作
    struct sembuf semops_p[1] = {{0, -1, SEM_UNDO}};
    //设置1号信号量作V(1)操作
    struct sembuf semops_v[1] = {{1, 1, SEM_UNDO}};
    
    //读取进程作P(1)操作，等待写入进程写入后通知当前进程
    if(semop(s->semid, semops_p, sizeof(semops_p) / sizeof(semops_p[0])) < 0)
    {
        perror("semop error");
        exit(EXIT_FAILURE);
    }

    //读取进程以只读的方式打开文件进行读取
    s->fd = open(s->pathname, O_RDONLY);
    if(s->fd < 0)
    {
        perror("open file error");
        exit(EXIT_FAILURE);
    }

    char buffer[128];
    if(read(s->fd, buffer, sizeof(buffer)) < 0)
    {
        perror("read error");
        exit(EXIT_FAILURE);
    }
    close(s->fd);
    printf("read process:%d read:%s\n",getpid(),buffer);

    //读取完成后将文件移除，防止下一次写入的时候报错文件已存在
    remove(s->pathname);        

    //读取进程作V(1)操作，通知写入进程开始下一轮写入
    if(semop(s->semid, semops_v, sizeof(semops_v) / sizeof(semops_v[0])) < 0)
    {
        perror("semop error");
        exit(EXIT_FAILURE);
    }
}

//销毁信号量集
void destroy_sem(Storage *s)
{
    assert(s != NULL);

    if(semctl(s->semid, 0, IPC_RMID, NULL) < 0)
    {
        perror("semctl error");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv)
{
    //从外部传参获取要操作的文件
    if(argc < 2)
    {
        fprintf(stderr, "%s | %s |%d error,Usage:%s pathname\n",
                            __FILE__,__func__,__LINE__,argv[0]);
        exit(EXIT_FAILURE);
    }

    //在堆空间上动态开辟空间操作
    Storage *s = (Storage*)malloc(sizeof(Storage));
    if(s == NULL)
    {
        perror("malloc error");
        exit(EXIT_FAILURE);
    }

    //初始化结构体
    memset(s, 0, sizeof(s));
    strcpy(s->pathname, argv[1]);
    //创建信号量集并初始化信号量
    s->semid = init_sem(s);
    
    //判断文件是否存在，若存在先将文件删除
    if(!access(s->pathname, F_OK))
        remove(s->pathname);

    pid_t pid;
    if((pid = fork()) < 0)
    {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    else if(pid > 0)
    {
        int i = 1;
        for(; i <= 100; i++)
        {
            write_func(s, i);
        }

        wait(NULL);         //等待子进程退出并回收其资源
        destroy_sem(s);     //将信号量集从内核中移除
    }
    else
    {
        int i = 1;
        for(; i <= 100; i++)
        {
            read_func(s);
        }
    }

    free(s);        //释放堆空间

    return 0;
}