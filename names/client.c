#include "unp.h"


int main(int argc, char **argv){
    if(argc != 3){
        err_sys("cmd: ./client host_name service_name");
    }
    char buf[MAX_LINE], ip_buf[IP_LEN];
    int nbytes;
    struct sockaddr_storage sas;
    socklen_t sl = sizeof(sas);
    int sock_fd = tcp_connect(argv[1], argv[2]);

    getpeername(sock_fd, (SA *)&sas, &sl);
    if(sas.ss_family == AF_INET){
        printf("connected to ip %s, port: %d", inet_ntop(AF_INET, (SA *)(&(((struct sockaddr_in *)(&(sas)))->sin_addr)), ip_buf, IP_LEN), ((struct sockaddr_in *)(&(sas)))->sin_port);
    }

    nbytes = Read(sock_fd, buf, MAX_LINE - 1);
    buf[nbytes] = 0;
    fputs(buf, stdout);
}
