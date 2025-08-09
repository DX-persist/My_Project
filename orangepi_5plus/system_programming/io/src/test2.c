#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int main()
{
	int fd = open("./test.txt",O_RDONLY);
}
