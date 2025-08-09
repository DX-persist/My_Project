#include "header.h"

int g_v = 30;

int main(void)
{
	int a_v = 30;
	static int s_v = 30;

	pid_t pid;

	//fopen是标准C库函数，所以在操作文件的时候会有缓存，所以当向文件写入或者从文件中读取的时候都会经过缓存，缓存的类型为全缓存
	//open函数是系统提供的系统调用函数，所以不带缓存功能，它在向文件写入或者读取的时候能够直接拿到

	FILE *fp = fopen("fp.txt","w");		
	int fd = open("fd.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG);	//如果文件不存在就创建文件，创建文件的权限为文件的拥有者和同组拥有rwx的权限
	
	char *str = "hello world";
	char buffer[48] = {'\0'};

	fprintf(fp, "pid:%d, content:%s",getpid(),str);	//fprinf函数向文件写入的时候会写入到缓存里，必须刷新缓存或者关闭文件才能够将内容写入到文件里去
	sprintf(buffer,"pid: %d str:%s\n",getpid(),str);
	write(fd,buffer,sizeof(buffer));		//write函数会直接写入到文件里而不经过缓存

	printf("parent pid is %d\n",getpid());
	pid = fork();
	if(pid > 0)
	{
		g_v = 40; a_v = 40; s_v = 40;
		printf("this is parent process, getpid: %d getppid: %d pid: %d\n",getpid(),getppid(),pid);
		printf("ag_v:%p aa_v:%p as_v:%p\n",&g_v,&a_v,&s_v);
	}
	else if(pid == 0)
	{
		g_v = 50; a_v = 50; s_v = 50;
		printf("this is child process,getpid: %d getppid: %d pid: %d\n",getpid(),getppid(),pid);
		printf("ag_v:%p aa_v:%p as_v:%p\n",&g_v,&a_v,&s_v);
	}
	else
	{
		perror("fork error");
	}

		sprintf(buffer,"pid: %d str:%s\n",getpid(),str);
		write(fd,buffer,sizeof(buffer));

		//printf("pid is %d g_v:%d a_v:%d s_v:%d\n",getpid(),g_v,a_v,s_v);
		fprintf(fp,"pid is %d g_v:%d a_v:%d s_v:%d\n",getpid(),g_v,a_v,s_v);

	return 0;
}
