#include "unp.h"


int main(int argc, char **argv){
    daemon_init("flash_back", LOG_LOCAL0);

    socklen_t sl;
    int listen_fd = tcp_listen(NULL, "distinct", &sl), connect_fd;
    struct sockaddr_in sai;
    char message[] = "hello and goodbye\n", ip_buf[IP_LEN], err_buf[MAX_LINE];

    while(1){
        sl = sizeof(sai);
        connect_fd = Accept(listen_fd, (SA *)&sai, &sl);
        snprintf(err_buf, MAX_LINE - 1, "receive request from %s, port: %d\n", inet_ntop(AF_INET, &(sai.sin_addr), ip_buf, IP_LEN), sai.sin_port);
        err_msg(err_buf);
        Write(connect_fd, message, sizeof(message));
        Close(connect_fd);
    }
}

