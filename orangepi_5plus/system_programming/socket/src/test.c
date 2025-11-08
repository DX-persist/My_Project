#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(void)
{
	char buffer[512];
	
	char *test = "Received a signal is SIGCHLD, child process exit and parent process reclaims its' resources\n";
	memset(buffer, '\0', sizeof(buffer));

	if(read(STDIN_FILENO, buffer, sizeof(buffer)) < 0){
		perror("read error");
	}else{
		buffer[sizeof(buffer) - 1] = '\0';
		if(write(STDOUT_FILENO, buffer, sizeof(buffer)) != sizeof(buffer))
		{
			perror("write error");
		}
	}
	printf("sizeof(buffer) = %d\n",sizeof(buffer));
	printf("strlen = %d\n",strlen(test));
	return 0;
}
