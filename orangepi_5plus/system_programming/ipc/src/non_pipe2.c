#include "header.h"

int main(void)
{
	int pipe_fd[2];
	int i = 0;
	pid_t pid;
	char *cmd1[] = {"cat", "/etc/passwd", NULL};
	char *cmd2[] = {"grep", "root", NULL};

	//父进程创建管道用于在兄弟进程中通信
	if(pipe(pipe_fd) < 0)
	{
		perror("pipe error");	
		exit(EXIT_FAILURE);
	}	

	//父进程创建子进程
	for(; i < 2; i++)
	{
		if((pid = fork()) < 0)
		{
			perror("fork error");
			exit(EXIT_FAILURE);
		}
		else if(pid == 0)
		{
			//子进程1用于获取cat指令执行的结果并写到管道中去
			if(i == 0)
			{
				close(pipe_fd[0]);			//子进程1用于向管道中写入，关闭读端
				//将标准输出重定向到管道的写入端，将cat的内容写入到管道中去
				//重定向后标准输出就指向了管道的写入端
				if(dup2(pipe_fd[1], STDOUT_FILENO) != STDOUT_FILENO)
				{
					perror("dup2 error");
					exit(EXIT_FAILURE);
				}
				close(pipe_fd[1]);			//此时标准输出的指向和之歌相同，所以关闭原来的文件描述符

				if(execvp(cmd1[0], cmd1) == -1)
				{
					perror("execvp error");
					exit(EXIT_FAILURE);
				}				
			}
			
			//子进程2用于从管道中获取并过滤相应的关键字	
			if(i == 1)
			{
				close(pipe_fd[1]);			//子进程2用于从管道中读取，关闭写端
				//grep指令默认会从标准输入中读取，所以要将标准输入重定向到管道的读端
				//然后利用grep过滤出来
				if(dup2(pipe_fd[0], STDIN_FILENO) != STDIN_FILENO)
				{
					perror("dup2 error");
					exit(EXIT_FAILURE);
				}
				close(pipe_fd[0]);			//经过重定向后标准输入和管道的读端文件描述符指向相同，所以关闭原来的文件描述符

				if(execvp(cmd2[0], cmd2) == -1)
				{
					perror("execvp error");
					exit(EXIT_FAILURE);
				}
			}
			break;
		}
		else 
		{
			if(i == 1)
			{
				//父进程只做创建管道和创建子进程的事情，不对管道进行操作
				//所以这里关闭父进程中的文件描述符
				close(pipe_fd[0]);
				close(pipe_fd[1]);

				//等待两个子进程退出并回收它的资源
				wait(NULL);
				wait(NULL);
			}
		}
	}	

	return 0;
}
