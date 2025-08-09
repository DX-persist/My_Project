#include "header.h"

/*
 *创建不完整管道：读一个写端关闭的管道
 */

int main(void)
{
	int pipe_fd[2];
	pid_t pid;

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
	else if(pid > 0)		//父进程等待子进程写入，然后从管道中读取
	{
		sleep(2);			

		close(pipe_fd[1]);			//关闭管道的写端

		char c;
		while(1)
		{
			if(read(pipe_fd[0], &c, 1) != 0)
			{
				printf("%c",c);
			}
			else
			{
				printf("\nread the end of pipe\n");
				break;
			}
		}
		wait(NULL);
	}
	else			//子进程关闭读端，先向管道中写入数据后关闭写端 
	{
		close(pipe_fd[0]);
		
		char *s = "1234";
		if(write(pipe_fd[1], s, strlen(s)) != strlen(s))		//向管道中写入数据
		{
			perror("write error");
			exit(EXIT_FAILURE);
		}
		close(pipe_fd[1]);		//关闭管道的写端
	}

	return 0;

}
