#include "header.h"
#include "io.h"

int main(int argc, char **argv)
{
	int fdin = STDIN_FILENO;
	int fdout = STDIN_FILENO;
	int i;
	int fd_tmp;
	char flag = 0;

	if(argc == 1)
	{
		copy(fdin, fdout);
	}

	for(i=1;i<argc;i++)
	{
		//+号重定义位输入重定向
		if(!strcmp(argv[i], "+"))
		{
			fd_tmp = open(argv[++i], O_RDONLY);
			if(fd_tmp < 0)
			{
				fprintf(stderr,"open file error: %s\n",strerror(errno));
				exit(EXIT_FAILURE);
			}		

			if(dup2(fd_tmp, STDIN_FILENO) != STDIN_FILENO)
			{
				perror("dup2 error");
				exit(EXIT_FAILURE);
			}
			close(fd_tmp);
		}
		else if(!strcmp(argv[i], "-"))
		{
			//将-号重定向为输出重定向，给文件赋予的权限是创建者拥有可读可写可执行的权限，同组人和其他人只有可读的权限

			fd_tmp = open(argv[++i], O_WRONLY | O_CREAT | O_TRUNC, 
										S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH);
			if(fd_tmp < 0)
			{
				fprintf(stderr,"open file error: %s\n",strerror(errno));
				exit(EXIT_FAILURE);
			}

			if(dup2(fd_tmp, STDOUT_FILENO) != STDOUT_FILENO)
			{
				perror("dup2 error");
				exit(EXIT_FAILURE);
			}
			close(fd_tmp);
		}
		else
		{
			flag = 1;

			fd_tmp = open(argv[i], O_RDONLY);
			if(fd_tmp < 0)
			{
				fprintf(stderr,"open file error: %s\n",strerror(errno));
				exit(EXIT_FAILURE);
			}

			if(dup2(fd_tmp, STDIN_FILENO) != STDIN_FILENO)
			{
				perror("dup2 error");
				exit(EXIT_FAILURE);
			}
			close(fd_tmp);

			copy(STDIN_FILENO, STDOUT_FILENO);
		}
	}

	if(!flag)
	{
		copy(STDIN_FILENO, STDOUT_FILENO);
	}

	return 0;
}
