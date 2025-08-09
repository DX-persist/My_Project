#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        printf("parameters are not enough, please enter IP and port\n");
        exit(EXIT_FAILURE);
    }

    int sock_fd;
    int ret;
    int new_sockfd;
    int len;
    int n_read;

    char readbuf[128] = {'\0'};
    char dest[128] = {'\0'};
    char *writebuf = "I got your message";

    struct sockaddr_in saddr;
    struct sockaddr_in caddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in));
    memset(&caddr, 0, sizeof(struct sockaddr_in));

    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(atoi(argv[2])); // Convert string to integer and then to network byte order
    if (inet_pton(AF_INET, argv[1], &(saddr.sin_addr)) == 0)
    {
        printf("Invalid address format\n");
        exit(EXIT_FAILURE);
    }

    if ((ret = bind(sock_fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in))) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(sock_fd, 10);

    len = sizeof(struct sockaddr_in);
    if ((new_sockfd = accept(sock_fd, (struct sockaddr *)&caddr, &len)) == -1)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    inet_ntop(AF_INET, &(caddr.sin_addr), dest, sizeof(dest));
    printf("Received a connection from IP: %s, port: %d\n", dest, ntohs(caddr.sin_port));

    n_read = read(new_sockfd, readbuf, sizeof(readbuf));
    if (n_read > 0)
    {
        printf("Read %d bytes and the content is: %s\n", n_read, readbuf);
    }

    write(new_sockfd, writebuf, strlen(writebuf) + 1);

    return 0;
}

