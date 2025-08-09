#include "io.h"

//cat < test.txt > zz.txt

int main(int argc, char **argv)
{
	int fdin,fdout;
	int i;

	//由于命令行参数argv[0]是命令，所以从argv[1]开始判断
	for(i=1;i<argc;i++)
	{
		//判断argv[1]是否是"+"，如果是就将标准输入重定向到文件
		if(!(strcmp("+",argv[i])))
		{
			fdin = open(argv[++i], O_RDONLY);
			if(fdin < 0)
			{
				perror("open file error");
				exit(EXIT_FAILURE);
			}

			if(dup2(fdin,STDIN_FILENO) != STDIN_FILENO)
			{
				perror("dup22 error");
				exit(EXIT_FAILURE);
			}

			close(fdin);
		}
		//判断argv[3]是否是"-"，如果是就将标准输出重定向到文件
		if(!(strcmp("-",argv[++i])))
		{
			fdout = open(argv[++i], O_WRONLY | O_CREAT | O_TRUNC, 0777);
			if(fdout < 0)
			{
				perror("open file error");
				exit(EXIT_FAILURE);
			}

			if(dup2(fdout,STDOUT_FILENO) != STDOUT_FILENO)
			{
				perror("dup22 error");
				exit(EXIT_FAILURE);
			}
			close(fdout);
		}
		//重定向后标准输入和标准输出都指向了文件，调用copy函数，从标准文件中读取输出到标准文件中去
		copy_function(STDIN_FILENO, STDOUT_FILENO);
	}


	return 0;
}
