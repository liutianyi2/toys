#include "unp.h"


int main(int argc, const char **argv){
    int listen_fd;
    struct sockaddr_in sai;

    listen_fd = Socket(AF_INET, SOCK_DGRAM, 0);
    int n = 10000;
    setsockopt(listen_fd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(int));
    bzero(&sai, sizeof(sai));
    sai.sin_family = AF_INET;
    sai.sin_port = htons(SERVER_PORT);
    sai.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(listen_fd, (SA *)&sai, sizeof(sai));

    echo_server_udp(listen_fd);
    //flow_server_udp(listen_fd);
}

