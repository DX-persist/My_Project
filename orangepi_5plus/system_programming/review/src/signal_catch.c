#include "header.h"

void sig_handler(int signum)
{
    switch(signum)
    {
        case 1:
            printf("process %d catches signal is SIGHUP\n",getpid());
            break;
        case 2:
            printf("process %d catches signal is SIGINT\n",getpid());
            break;
        case 9:
            printf("process %d catches signal is SIGKILL\n",getpid());
            break;
      	case 10:
            printf("process %d catches signal is SIGUSR1\n",getpid());
            break;
        case 12:
            printf("process %d catches signal is SIGUSR2\n",getpid());
            break;
        case 19:
            printf("process %d catches signal is SIGSTOP\n",getpid());
            break;
        default:
            printf("there is no such signal\n");
    }
}

int main(void)
{
	printf("process pid is %d\n",getpid());

    //向内核登记信号处理函数以及需要捕获的信号
    if(signal(SIGINT, SIG_DFL) == SIG_ERR)
    {
        perror("signal SIGINT error");
    }
    
    if(signal(SIGHUP, sig_handler) == SIG_ERR)
    {
        perror("signal SIGHUP error");
    }
    
    if(signal(SIGUSR1, sig_handler) == SIG_ERR)
    {
        perror("signal SIGUSR1 error");
    }
    
    if(signal(SIGUSR2, sig_handler) == SIG_ERR)
    {
        perror("signal SIGUSR2 error");
    }
    
    if(signal(SIGKILL, sig_handler) == SIG_ERR)
    {
        perror("signal SIGKILL error");
    }
    
    if(signal(SIGSTOP, sig_handler) == SIG_ERR)
    {
        perror("signal SIGSTOP error");
    }
    
    while(1) sleep(1);
    
    return 0;
}
