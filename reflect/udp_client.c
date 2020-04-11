#include "unp.h"


int main(int argc, const char **argv){
    if(argc != 2){
        err_sys("cmd: ./client server_ip");
    }
    int sock_fd;
    struct sockaddr_in sai, sai_me;
    socklen_t sl = sizeof(sai_me);
    char IP_BUF[IP_LEN];

    sock_fd = Socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&sai, sizeof(sai));
    sai.sin_family = AF_INET;
    sai.sin_port = htons(SERVER_PORT);
    if((inet_pton(AF_INET, argv[1], &(sai.sin_addr))) < 0){
        err_sys("inet_pton failed");
    }

    Connect(sock_fd, (SA *)&sai, sizeof(sai));
    getsockname(sock_fd, (SA *)&sai_me, &sl);
    printf("after bind, local ip: %s, port: %d", inet_ntop(AF_INET, &(sai_me.sin_addr), IP_BUF, IP_LEN), ntohs(sai_me.sin_port));

    echo_client_udp(sock_fd, (SA *)&sai, sizeof(sai));
    //flow_client_udp(sock_fd, (SA *)&sai, sizeof(sai));
}

