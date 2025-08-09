#include "header.h"

int main(void)
{
    char *cmd1 = "/bin/ls";
    char *cmd2 = "ls";
    char *argv1 = "/home/dx";
    char *argv2 = "/";

    pid_t pid;

    /**************execl函数******************/
    if((pid = fork()) < 0)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        printf("this is the child process and the pid is %d,parent pid is %d\n",getpid(),getppid());
        if(execl(cmd1, cmd1, argv1, argv2, NULL) < 0)
        {
            perror("execl error");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("this is parent process and the pid is %d,the child process is %d\n",getpid(),pid);
        wait(NULL); //等待子进程退出，防止子进程如果execl失败变成一个僵尸进程
    }

    /**************execlp函数******************/
    if((pid = fork()) < 0)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        printf("this is the child process and the pid is %d,parent pid is %d\n",getpid(),getppid());
        if(execlp(cmd2, cmd2, argv1, argv2, NULL) < 0)
        {
            perror("execlp error");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("this is parent process and the pid is %d,the child process is %d\n",getpid(),pid);
        wait(NULL); //等待子进程退出，防止子进程如果execl失败变成一个僵尸进程
    }

    /**************execv函数******************/
    char *argvs[] = {cmd1,argv1,argv2,NULL};
    if((pid = fork()) < 0)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        printf("this is the child process and the pid is %d,parent pid is %d\n",getpid(),getppid());
        if(execv(cmd1, argvs) < 0)
        {
            perror("execv error");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("this is parent process and the pid is %d,the child process is %d\n",getpid(),pid);
        wait(NULL); //等待子进程退出，防止子进程如果execl失败变成一个僵尸进程
    }

    /**************execvp函数******************/
    char *argvp[] = {cmd2,argv1,argv2,NULL};
    if((pid = fork()) < 0)
    {
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0)
    {
        printf("this is the child process and the pid is %d,parent pid is %d\n",getpid(),getppid());
        if(execvp(cmd2, argvp) < 0)
        {
            perror("execvp error");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("this is parent process and the pid is %d,the child process is %d\n",getpid(),pid);
        wait(NULL); //等待子进程退出，防止子进程如果execl失败变成一个僵尸进程
    }

    return 0;
}
