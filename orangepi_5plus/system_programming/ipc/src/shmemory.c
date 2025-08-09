#include "header.h"
#include "tell.h"

#define size 1024

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        fprintf(stderr,"%s|%s|%d error! Usage:%s key_value\n",
                    __FILE__,__func__,__LINE__,argv[0]);
        EXIT_FAILURE;
    }

    key_t key;
    int shmid;
    pid_t pid;
    
    //从外部获取键值用于配合shmget函数形成唯一的ID
    key = atoi(argv[1]);

    //创建共享内存，大小为1024个字节，指定flag IPC_CREAT | IPC_EXCL表明
    //没有此共享内存就创建，如果存在就报错并设置errno为EEXIST   
    if((shmid = shmget(key, size, IPC_CREAT | IPC_EXCL | 0774)) < 0)
    {
        perror("shmget error");
        exit(EXIT_FAILURE);
    }

    //初始化管道用于父子间通信
    init_pipe();

    if((pid = fork()) < 0)
    {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    else if(pid > 0)            //parent process
    {
        //将共享内存映射到当前进程的虚拟空间中
        int *shmaddr = (int *)shmat(shmid, 0, 0);
        if(shmaddr == (int *)-1)
        {
            perror("shmat error");
            exit(EXIT_FAILURE);
        }

        //将数据存放到映射后的地址中（此时的地址是当前进程虚拟空间的地址）
        //最终会通过MMU映射到物理内存上去
        *shmaddr = 100; *(shmaddr + 1) = 200;
        printf("parent process:%d write %d %d to the shared memory\n",
                                        getpid(),*shmaddr,*(shmaddr+1));
        //解除共享内存的映射
        shmdt(shmaddr);

        //通知子进程父进程已经完成了对共享内存的操作
        notify_pipe();

        //关闭管道
        destroy_pipe();

        //等待子进程退出并回收其资源
        wait(NULL);     
    }
    else                        //child process
    {
        //子进程阻塞等待父进程将数据存放到共享内存中再执行
        wait_pipe();

        //子进程将共享内存映射到当前进程的虚拟空间中
        int *shmaddr = (int *)shmat(shmid, 0, 0);
        if(shmaddr == (int *)-1)
        {
            perror("shmat error");
            exit(EXIT_FAILURE);
        }

        //子进程从共享内存中获取数据
        printf("child process:%d read %d %d from the shared memory\n",
                                        getpid(),*shmaddr,*(shmaddr+1));

        //解除共享内存的映射
        shmdt(shmaddr);

        //关闭管道
        destroy_pipe();

        //将共享内存从内核中移除
        shmctl(shmid, IPC_RMID, NULL);
    }
}