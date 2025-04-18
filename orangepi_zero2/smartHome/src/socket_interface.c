#include "socket_interface.h"

static int sock_fd = -1;

static int tcpsocket_init(void)
{
    sock_fd = socket_init(IPADDR, IPPORT);

    return 0;
}

static void tcpsocket_final(void)
{
    close(sock_fd);
    sock_fd = -1;
}

static void *tcpsocket_get(void *arg)
{
    int client_fd = -1;
    int n_read = -1;
    int ret = -1;
    char buffer[6];
    mqd_t mqd = -1;
    struct sockaddr_in caddr;
    memset(&caddr,'\0',sizeof(caddr));
    int keepalive = 1; // 开启TCP KeepAlive功能,防止出现卡死断联等造成卡死的现象
    int keepidle = 5; // tcp_keepalive_time 3s内没收到数据开始发送心跳包
    int keepcnt = 3; // tcp_keepalive_probes 每次发送心跳包的时间间隔,单位秒
    int keepintvl = 3; // tcp_keepalive_intvl 每3s发送一次心跳包
    int clen = -1;

    if(arg != NULL){
        mqd = ((ctrl_info_t *)arg)->mqd;
    }

    if(mqd == (mqd_t)-1)
        pthread_exit(NULL);

    pthread_detach(pthread_self());

    printf("%s|%s|%d sock_fd = %d\n",__FILE__,__func__,__LINE__,sock_fd);
    if(sock_fd == -1)
    {
        sock_fd = tcpsocket_init();
        if(sock_fd == -1)
        {
            printf("%s|%s|%d: Failed to initialize socket\n",__FILE__,__func__,__LINE__);
            pthread_exit(NULL);
        }
    }
    

    clen = sizeof(struct sockaddr_in);

    printf("%s thread started\n",__func__);
    while(1)
    {
        printf("%s|%s|%d: sock_fd = %d\n",__FILE__,__func__,__LINE__,sock_fd);
        client_fd = accept(sock_fd,(struct sockaddr *)&caddr,&clen);
        if(client_fd == -1)
        {
            perror("accept");
            continue;
        }
     
        ret = setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive,sizeof(keepalive));
        if(ret == -1){
            perror("setsockopt");
            break;
        }

        ret = setsockopt(client_fd, SOL_TCP, TCP_KEEPIDLE, (void *) &keepidle, sizeof(keepidle));
        if(ret == -1){
            perror("setsockopt");
            break;
        }

        ret = setsockopt(client_fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcnt, sizeof(keepcnt));
        if(ret == -1){
            perror("setsockopt");
            break;
        }
        ret = setsockopt(client_fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepintvl, sizeof(keepintvl));
        if(ret == -1){
            perror("setsockopt");
            break;
        }

        printf("IP is %s,Port is %d connected\n",inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port));

        while(1)
        {
            memset(buffer, '\0', sizeof(buffer));
            n_read = read(client_fd, buffer,    sizeof(buffer));
            printf("%s|%s|%d : receive %d bytes from server\n",__FILE__, __func__, __LINE__, n_read);
            printf("%s|%s|%d 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",__FILE__, __func__, 
                __LINE__, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
            
            if(n_read > 0)
            {
                if(buffer[0] == 0xAA && buffer[1] == 0x55 
                && buffer[4] == 0x55 &&buffer[5] == 0xAA){
                printf("%s|%s|%d send message to queue: 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",__FILE__, __func__, 
                    __LINE__, buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
                msg_queue_send(mqd, buffer, n_read);
            }
            }
            else if(n_read == 0 || n_read == -1)
            {
                break;
            }
        }
        close(client_fd);
    }
    pthread_exit(NULL);
}

#if 0
static void *tcpsocket_set(void *arg)
{

}
#endif

struct control tcpsocket_control = {
    .control_name = "tcpsocket",
    .init = tcpsocket_init,
    .final = tcpsocket_final,
    .get = tcpsocket_get,
    .set = NULL,
//   .set = tcpsocket_set,
   .next = NULL
};

struct control *add_SocketInLink(struct control *control_head)
{
    return add_InterfaceInLink(control_head, &tcpsocket_control);
}