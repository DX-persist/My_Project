#include "header.h"
#include "account.h"
#include "pv.h"

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        fprintf(stderr,"Usage:%s key_value\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    key_t key;
    int shmid;
    pid_t pid;

    //从外部获取键值
    key = atoi(argv[1]);

    //创建共享内存用于父子进程间的通信
    if((shmid = shmget(key, sizeof(Account), IPC_CREAT | IPC_EXCL | 0774)) < 0)
    {
        perror("shmget error");
        exit(EXIT_FAILURE);
    }

    //将共享内存映射到当前进程的虚拟内存空间中去
    Account *a = (Account*)shmat(shmid, 0, 0);
    if(a == (Account*)-1)
    {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }
    a->acc_num = 123456789;
    a->balance = 10000;
    //初始化信号量集，信号量的个数是1，初始值是1
    a->semid = I(1,1);

    printf("account number:%d balance:%f\n",a->acc_num,a->balance);
    //模拟两个用户在操作同一个银行账户
    if((pid = fork()) < 0)
    {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    else if(pid > 0)            //parent process
    {
        //获取账户余额
        double amount = withdrawal(a, 10000);

        printf("parent process:%d get the money:%f\n",getpid(),amount);
        
        //销毁信号量集，信号量集的销毁要在解除映射之前，若在信号量销毁之前解除映射
        //那么信号量集就不能再释放了，因为信号量集是在共享内存上定义的
        D(a->semid);

        //解除共享内存的映射
        shmdt(a);
        //等待子进程退出并回收其资源
        wait(NULL);
        //将共享内存从内核中移除
        shmctl(shmid, IPC_RMID, NULL);
    }
    else                        //child process
    {
        //获取账户余额
        double amount = withdrawal(a, 10000);

        printf("child process:%d get the money:%f\n",getpid(),amount);
        printf("the balance of acccount %f\n",a->balance);
        //解除共享内存的映射
        shmdt(a);
    }

    return 0;
}
