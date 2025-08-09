#include "header.h"

int main(void)
{
	int pipe_fd[2];
	pid_t pid;

	//父进程通过pipe函数创建管道用于父子进程间通信
	if(pipe(pipe_fd) != 0)
	{
		perror("pipe error");
		exit(EXIT_FAILURE);
	}

	if((pid = fork()) < 0)
	{
		perror("fork error");
		exit(EXIT_FAILURE);
	}
	//父进程向管道中写入，关闭读端
	else if(pid > 0)
	{
		int start = 1, end = 100;

		close(pipe_fd[0]);			//父进程关闭读端

		if(write(pipe_fd[1], &start, sizeof(int)) != sizeof(int))
		{
			perror("write error");
			exit(EXIT_FAILURE);
		}

		if(write(pipe_fd[1], &end, sizeof(int)) != sizeof(int))
		{
			perror("write error");
			exit(EXIT_FAILURE);
		}

		close(pipe_fd[1]);		//写入完成后关闭写端

		wait(NULL);			//等待子进程退出并回收它的资源
	}
	//子进程从管道中读取，关闭写端
	//父进程从尾部写入，子进程从头部读取
	else
	{
		int start, end;

		close(pipe_fd[1]);

		if(read(pipe_fd[0], &start, sizeof(int)) < 0)
		{
			perror("read error");
			exit(EXIT_FAILURE);
		}

		if(read(pipe_fd[0], &end, sizeof(int)) < 0)
		{
			perror("read error");
			exit(EXIT_FAILURE);
		}
		close(pipe_fd[0]);			//读取完成后关闭读端
		
		printf("child process read data start:%d end:%d\n",start,end);
	}

	return 0;
}
