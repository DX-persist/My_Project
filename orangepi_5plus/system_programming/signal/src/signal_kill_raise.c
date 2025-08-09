#include "header.h"

void sig_handler(int signum)
{
    printf("process %d receive a signal is %d\n",getpid(),signum);

    switch (signum)
    {
    case 1:
        printf("receive signal is SIGHUP\n");
        break;
    case 2:
        printf("receive signal is SIGINT\n");
        break;
    case 10:
        printf("receive signal is SIGUSR1\n");
        break;
    case 12:
        printf("receive signal is SIGUSR2\n");
        break;
    default:
        printf("no sign to kernel\n");
        break;
    }
}

int main(int argc, char **argv)
{
    if(argc < 2)
    {
        fprintf(stderr,"usage: %s [signum]\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    int signum = atoi(argv[1]);

    if(signal(SIGUSR1, sig_handler) == SIG_ERR)
    {
        perror("signal SIGUSR1 error");
    }

    if(signal(SIGUSR2, sig_handler) == SIG_ERR)
    {
        perror("signal SIGUSR2 error");
    }

    if(signal(SIGHUP, sig_handler) == SIG_ERR)
    {
        perror("signal SIGHUP error");
    }

    if(signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("signal SIGINT error");
    }

    //将当前进程的PID号传进去给当前进程发信号
    if(kill(getpid(), signum) != 0)
    {
        perror("kill perror");
        exit(EXIT_FAILURE);
    }

    if(raise(signum) != 0)
    {
        perror("raise perror");
        exit(EXIT_FAILURE);
    }
    
    return 0;
}