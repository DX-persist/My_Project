#include "header.h"
#include "tell.h"

static int pipe_fd[2];

//创建管道用于父子进程间的同步，由于共享内存没有进程间同步的方式
//所以利用管道的阻塞机制来控制它们之间的同步
void init_pipe()
{
    if(pipe(pipe_fd) < 0)
    {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }
}

//当管道中没有数据的时候，从管道中读取会阻塞直到有另外一个
//进程向管道中写入数据
void wait_pipe()
{
    char c;

    if(read(pipe_fd[0], &c, sizeof(char)) < 0)
    {
        perror("read error");
        exit(EXIT_FAILURE);
    }
}

//向管道中写入数据，使得被阻塞的进程继续执行
void notify_pipe()
{
    char c = 'x';

    if(write(pipe_fd[1], &c, sizeof(char)) != sizeof(char))
    {
        perror("write error");
        exit(EXIT_FAILURE);
    }
}

//销毁管道
void destroy_pipe()
{
    close(pipe_fd[0]);
    close(pipe_fd[1]);
}
