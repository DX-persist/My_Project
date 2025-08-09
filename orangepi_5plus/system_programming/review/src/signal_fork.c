#include "header.h"

void sig_handler(int signum)
{
    printf("pid:%d signum:%d\n",getpid(),signum);
}

int main()
{
    pid_t pid;
    
    if((pid = fork()) < 0)
    {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
	else if(pid == 0)	//子进程向内核注册信号处理函数
	{
		printf("this is child process,pid:%d ppid:%d\n",getpid(),getppid());

		if(signal(SIGHUP, sig_handler) == SIG_ERR)
		{
			perror("signal error");
		}
		
		int i = 0;
		while(i < 15)
		{
			printf("pid:%d i = %d\n",getpid(),i);
			i++;
			sleep(1);
		}
	}
	else		//父进程向子进程发送信号
	{
		sleep(1);
		printf("parent process,pid:%d child's pid:%d\n",getpid(),pid);
		if(kill(pid, SIGHUP) != 0)
		{
			perror("kill error");
			exit(EXIT_FAILURE);
		}
		wait(NULL);
	}
}
