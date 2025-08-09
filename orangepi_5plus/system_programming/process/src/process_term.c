#include "header.h"

void exit_handler1(void)
{
	printf("first term func1\n");
}

void exit_handler2(void)
{
	printf("second term func2\n");
}

void exit_handler3(void)
{
	printf("third term func3\n");
}

int main(int argc, char **argv)
{
	if(argc < 3)
	{
		fprintf(stderr,"usage: %s [filepath] [exit_type:exit|_exit|return]\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	//向内核登记终止函数
	if(atexit(exit_handler1) != 0)
	{
		perror("atexit");
		exit(EXIT_FAILURE);
	}

	if(atexit(exit_handler2) != 0)
	{
		perror("atexit");
		exit(EXIT_FAILURE);
	}

	if(atexit(exit_handler3) != 0)
	{
		perror("atexit");
		exit(EXIT_FAILURE);
	}
	
	FILE *fp = fopen(argv[1],"w");
	if(fp == NULL)
	{
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	fprintf(fp, "hello world");
	
	if(!strcmp(argv[2],"return"))
	{
		return 0;
	}
	else if(!strcmp(argv[2],"_exit"))
	{
		_exit(EXIT_SUCCESS);
	}
	else if(!strcmp(argv[2],"exit"))
	{
		exit(EXIT_FAILURE);
	}
	else
	{
		fprintf(stderr,"usage: %s [filepath] [exit_type:exit|_exit|return]\n",argv[0]);
		exit(EXIT_FAILURE);
	}
}
