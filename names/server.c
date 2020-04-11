#include "unp.h"


int main(int argc, char **argv){
    if(argc != 2){
        err_sys("cmd: ./server server_name");
    }

    socklen_t sl;
    int listen_fd = tcp_listen(NULL, argv[1], &sl), connect_fd;
    struct sockaddr_storage sas;
    char ip_buf[IP_LEN], response[] = "hello, and byebye";

    while(1){
        sl = sizeof(sas);
        connect_fd = Accept(listen_fd, (SA *)&sas, &sl);
        printf("new client\n");
        if(sas.ss_family == AF_INET){
            printf("client ip: %s, port: %d\n", inet_ntop(AF_INET, &(((struct sockaddr_in *)(&sas))->sin_addr), ip_buf, IP_LEN), ntohs(((struct sockaddr_in *)(&sas))->sin_port));
        }
        write(connect_fd, response, sizeof(response));
        close(connect_fd);
    }
}

