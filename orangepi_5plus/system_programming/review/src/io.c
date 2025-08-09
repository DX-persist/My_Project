#include "io.h"
#include "header.h"

#define BUFFER_SIZE 4096

void copy(int fdin, int fdout)
{
	int nbytes;
	char buffer[BUFFER_SIZE];

	memset(buffer,'\0',sizeof(buffer));

	while((nbytes = read(fdin, buffer, BUFFER_SIZE)) > 0)
	{
		if(write(fdout, buffer, nbytes) != nbytes)
		{
			fprintf(stderr,"write error: %s\n",strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	if(nbytes < 0)
	{
		fprintf(stderr,"read error: %s\n",strerror(errno));
		exit(EXIT_FAILURE);
	}
}

void set_fl(int fd, int flag)
{
	int val = fcntl(fd, F_GETFL);

	val |= flag;

	if(fcntl(fd, F_SETFL, val) < 0)
	{
		perror("fcntl error");
	}
}

void clr_flg(int fd, int flag)
{
	int val = fcntl(fd, F_GETFL);

	val &= ~flag;

	if(fcntl(fd, F_SETFL, val) < 0)
	{
		perror("fcntl error");
	}
}
