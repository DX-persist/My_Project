#include "header.h"

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		fprintf(stderr,"usage:%s [filepath]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	int fd = open(argv[1],O_WRONLY);
	if(fd < 0)
	{
		perror("open");
		exit(EXIT_FAILURE);
	}
	
	pid_t pid;

	pid = fork();
	if(pid < 0)
	{
		perror("fork");
		exit(EXIT_FAILURE);
	}
	else if(pid > 0)
	{
		if(lseek(fd,0,SEEK_END) < 0)		//父进程通过lseek函数将文件位置偏移到文件的末尾，然后子进程去操作文件
		{
			perror("lseek");
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		char *str = "hello MAKABAKA";
		ssize_t size = strlen(str);

		sleep(3);
		if(write(fd,str,size) != size)		//虽然父子进程操作的都是fd，但是子进程的fd是父进程将文件描述符表复制了一份给子进程
											//所以它们操作的不是同一个fd，但是操作的是同一个文件。
		{
			perror("write error");
			exit(EXIT_FAILURE);
		}
	}

	printf("pid:%d finished\n",getpid());

	close(fd);		//这里的close函数会被父子进程执行两次，每执行一次就会将引用计数器的数字减1，直到将引用计数器减为0文件才会关闭

	return 0;
}
