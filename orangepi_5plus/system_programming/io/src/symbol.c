#include "io.h"

int main(int argc, char **argv)
{
	int fdin,fdout;
	int i;
	int flag = 0;

	for(i=1;i<argc;i++)
	{
		//i=1，检测外部传参2的符号，如果输入的是+号，
		//那么就将标准输入重定向到文件，去捕获文件的输入然后输出到标准输出去
		if(!(strcmp("+",argv[i])))		
		{
			//将+定义为输入重定向
			fdin = open(argv[++i],O_RDONLY);		
			if(fdin < 0)
			{
				perror("open file error");
				exit(EXIT_FAILURE);
			}	

			//通过dup2函数复制文件描述符后，标准输入这个文件描述符指向了文件表项
			if(dup2(fdin,STDIN_FILENO) != STDIN_FILENO)
			{
				perror("dup2 error");
				exit(EXIT_FAILURE);
			}

			close(fdin);		//将旧的文件描述符关闭
		}

		//如果检测到外部传参2是-号，那么就将标准输出重定向到文件
		//将-重定向为输出重定向
		else if(!(strcmp("-",argv[i])))
		{
			fdout = open(argv[++i],O_WRONLY | O_CREAT | O_TRUNC, 0777);	//由于文件可能事先不存在，所以要创建文件
			if(fdout < 0)
			{
				perror("open file error");
				exit(EXIT_FAILURE);
			}
			//将标准输出重定向到文件，经过这个操作以后标准输出指向刚刚打开的文件
			//dup2函数如果成功执行，那么返回新的文件描述符即参数2
			if(dup2(fdout,STDOUT_FILENO) != STDOUT_FILENO)
			{
				perror("dup2 error");
				exit(EXIT_FAILURE);
			}

			close(fdout);
		}

		//如果参数2不是+/-那么就说明是从某一个文件中读取然后输出到屏幕中去，所以要将标准输入重定向到文件
		//以此来获取文件的内容，然后使用copy函数，将通过重定向后的标准输入和标准输出传到这个函数里去执行
		else
		{
			flag = 1;
			
			fdin = open(argv[i],O_RDONLY);
			if(fdin < 0)
			{
				perror("open file error");
				exit(EXIT_FAILURE);
			}

			if(dup2(fdin,STDIN_FILENO) != STDIN_FILENO)
			{
				perror("dup2 error");
				exit(EXIT_FAILURE);
			}

			copy_function(STDIN_FILENO, STDOUT_FILENO);
		}
	}

	if(!flag)
	{
		copy_function(STDIN_FILENO, STDOUT_FILENO);
	}

	return 0;
}
