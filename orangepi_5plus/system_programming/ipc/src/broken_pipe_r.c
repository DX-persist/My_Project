#include "header.h"

/*
 *	创建不完整管道：写一个读端关闭的管道
 */

void sig_handler(int signum)
{
	if(signum == 13)
	{
		printf("receive a signal is SIGPIPE\n");
	}
}

int main(void)
{
	int pipe_fd[2];
	pid_t pid;

	//向内核注册信号和信号处理函数，如果发生信号就去执行相应的处理函数
	if(signal(SIGPIPE, sig_handler) == SIG_ERR)
	{
		perror("signal error");
	}

	//父进程创建管道
	if(pipe(pipe_fd) < 0)
	{
		perror("pipe error");
		exit(EXIT_FAILURE);
	}

	if((pid = fork()) < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if(pid > 0)		//父进程等待子进程关闭读端然后再向管道中写入
	{
		sleep(2);			//睡眠2秒，保证子进程已经将读端关闭
		
		close(pipe_fd[0]);		//父进程关闭读端
		
		char *s = "1234";
		if(write(pipe_fd[1], s, strlen(s)) != strlen(s))
		{
			fprintf(stderr,"%s:%s\n",strerror(errno),(errno == EPIPE)?"EPIPE":"unknown");		
			//当向已经关闭读端的管道中写入的时候会产生SIGPIPE信号，同时errno被设置为EPIPE
		}
		close(pipe_fd[1]);		//写完后关闭写端
		wait(NULL);						
	}
	else					//子进程将读端和写端的管道都关闭
	{
		close(pipe_fd[0]);
		close(pipe_fd[1]);
	}

	return 0;
}
