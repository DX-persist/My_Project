#include "header.h"
#include "pv.h"

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;   
};

//创建信号量集，并对信号量集中的信号量进行赋值
int I(int nsems, int value)
{
    //创建信号量集，信号量集中的信号量的数量为nsems
    int semid = semget(IPC_PRIVATE, nsems, IPC_CREAT | IPC_EXCL | 0774);
    if(semid < 0)
    {
        perror("semget error");
        return -1;
    }

    //对信号量集中的信号量进行赋值
    union semun un;

    unsigned short *array = (unsigned short *)malloc(sizeof(unsigned short) * nsems);
    if(array == NULL)
    {
        perror("malloc error");
        return -1;
    }
    int i;
    for(i = 0; i < nsems; i++)
    {
        array[i] = value;
    }
    un.array = array;

    //对信号量集进行赋值，0标识操作所有的信号量，SETALL标识设置所有信号量的值
    if(semctl(semid, 0, SETALL, un) < 0)
    {
        perror("semctl error");
        return -1;
    }

    free(array);

    return semid;
}

//对信号量集编号为semid，信号量编号为semnum的信号量作P操作
void P(int semid, int semnum, int value)
{
    assert(value >= 0);

    //定义一个结构体数组包含要操作的信号量的信息
    //semnum标识要操作的信号量的标识，-value标识要作P操作，SEM_UNDO
    //标识若进程意外退出，信号量恢复到上一个操作
    struct sembuf semops[] = {{semnum, -value, SEM_UNDO}};

    //semop函数对信号量操作，参数是信号量集的id，要操作信号量集中
    //信号量的标识，以及信号量的个数
    if(semop(semid, semops, sizeof(semops)/sizeof(semops[0])) < 0)
    {
        perror("semop error");
        exit(EXIT_FAILURE);
    }
}

//对信号量集编号为semid，信号量编号为semnum的信号量作V操作
void V(int semid, int semnum, int value)
{
    assert(value >= 0);

    //定义一个结构体数组包含要操作的信号量的信息
    //semnum标识要操作的信号量的标识，-value标识要作P操作，SEM_UNDO
    //标识若进程意外退出，信号量恢复到上一个操作
    struct sembuf semops[] = {{semnum, value, SEM_UNDO}};

    //semop函数对信号量操作，参数是信号量集的id，要操作信号量集中
    //信号量的标识，以及信号量的个数
    if(semop(semid, semops, sizeof(semops)/sizeof(semops[0])) < 0)
    {
        perror("semop error");
        exit(EXIT_FAILURE);
    }
}

//销毁编号为semid的信号量集
void D(int semid)
{
    //semctl函数中的参数0标识操作所有的信号量，将信号量从内核中移除
    if(semctl(semid, 0, IPC_RMID, NULL) < 0)
    {
        perror("semctl error");
        exit(EXIT_FAILURE);
    }
}
