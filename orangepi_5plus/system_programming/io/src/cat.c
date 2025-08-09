#include "io.h"

int main(int argc, char **argv)
{
	int fdin = STDIN_FILENO;
	int fdout = STDOUT_FILENO;
	int i;


	for(i=1;i<argc;i++)
	{
		fdin = open(argv[i],O_RDONLY);
		if(fdin < 0)
		{
			perror("open file error");
			exit(EXIT_FAILURE);
		}

		copy_function(fdin,fdout);

		close(fdin);
	}

	if(argc == 1)
	{
		copy_function(fdin,fdout);
	}

	close(fdout);

	return 0;
}
