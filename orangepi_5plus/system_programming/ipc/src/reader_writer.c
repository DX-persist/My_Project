#include "header.h"

typedef struct 
{
    int val;
    int semid;
}Storage;

void init_sem(Storage *s)
{
    //创建信号量集，信号量的个数为2
    s->semid = semget(IPC_PRIVATE, 2, IPC_CREAT | IPC_EXCL | 0774);
    if(s->semid < 0)
    {
        perror("semget error");
        exit(EXIT_FAILURE);
    }

    //对信号量集中的信号进行赋值
    union semun
    {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    };

    union semun un;
    //将信号量集中的信号量的初值都设置为0
    unsigned short array[2] = {0, 0};   
    un.array = array;
    //第二个参数为0表示设置所有信号量的值，第三个参数指定SETALL标志位
    if(semctl(s->semid, 0, SETALL, un) < 0)
    {
        perror("semctl error");
        exit(EXIT_FAILURE);
    }
}

void write_func(Storage *s, int value)
{
    s->val = value;
    printf("write process:%d write:%d\n",getpid(),s->val);

    struct sembuf sops_v[1] = {{0, 1, SEM_UNDO}};
    struct sembuf sops_p[1] = {{1, -1, SEM_UNDO}};

    //对标号为0的信号量作加1操作，使得读者操作继续
    if(semop(s->semid, sops_v, 1) < 0)
    {
        perror("semop1 error");
        exit(EXIT_FAILURE);
    }

    //对标号为1的信号量作减1操作，等待读者进程作加1操作
    if(semop(s->semid, sops_p, 1) < 0)
    {
        perror("semop2 error");
        exit(EXIT_FAILURE);
    }
}

void read_func(Storage *s)
{
    struct sembuf sops_p[1] = {{0, -1, SEM_UNDO}};
    struct sembuf sops_v[2] = {{1, 1, SEM_UNDO}};

    if(semop(s->semid, sops_p, 1) < 0)
    {
        perror("semop3 error");
        exit(EXIT_FAILURE);
    }

    printf("read process:%d read:%d\n",getpid(),s->val);

    if(semop(s->semid, sops_v, 1) < 0)
    {
        perror("semop4 error");
        exit(EXIT_FAILURE);
    }
}

void destroy_sem(Storage *s)
{
    if(semctl(s->semid, 0, IPC_RMID, NULL) < 0)
    {
        perror("semctl error");
        exit(EXIT_FAILURE);
    }   
}

int main(void)
{
    //创建共享内存，将信号量创建在共享内存上
    int shmid = shmget(IPC_PRIVATE, sizeof(Storage), IPC_CREAT | IPC_EXCL | 0774);
    if(shmid < 0)
    {
        perror("shmget error");
        exit(EXIT_FAILURE);
    }

    //将共享内存映射到当前进程的虚拟空间中
    Storage *s = (Storage*)shmat(shmid, 0, 0);
    if(s == (Storage*)-1)
    {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }    

    init_sem(s);
    pid_t pid;

    if((pid = fork()) < 0)
    {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    else if(pid > 0)           //parent process
    {
        int i = 1;
        for(; i <= 100; i++)
        {
            write_func(s, i);
        }

        wait(NULL);
        
        shmdt(s);
        shmctl(shmid, IPC_RMID, NULL);
        
    }
    else                        //child process
    {
        int i = 1;
        for(; i <= 100; i++)
        {
            read_func(s);
        }
        destroy_sem(s);
        shmdt(s);
    }

    return 0;
}