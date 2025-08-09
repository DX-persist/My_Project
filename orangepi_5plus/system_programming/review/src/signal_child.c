#include "header.h"

void sig_handler(int signum)
{
    printf("process %d receive a signal and the number is %d\n",getpid(),signum);
    wait(NULL);
}

void count_func(int num)
{
    int i;
    
    for(i=0;i<num;i++)
    {
        printf("process: %d i:%d\n",getpid(),i);
        sleep(1);
    }
}

int main(void)
{
    //向内核登记捕获SIGCHLD信号，一旦产生这个信号就去执行相应的信号处理函数
    if(signal(SIGCHLD, sig_handler) == SIG_ERR)
    {
        perror("signal SIGCHLD error");
    }
    
    pid_t pid;
    if((pid = fork()) < 0)
    {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        printf("this is child process and the pid is %d\n",getpid());
        count_func(10);
    }
    else
    {
        printf("this is parent process and the pid is %d\n",getpid());
        count_func(40);
    }
    
    return 0;
}
