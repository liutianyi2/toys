#include "unp.h"


int main(int argc, char **argv){
    int sock_fd, n;
    char recv_line[MAX_LINE + 1];
    struct sockaddr_in sai;

    if(argc != 2){
        err_sys("cmd: ./a.out <IP>");
    }

    if((sock_fd = (socket(AF_INET, SOCK_STREAM, 0))) <= 0){
        err_sys("socket failed");
    }
    int tos = 32;
    setsockopt(sock_fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));

    memset(&sai, 0, sizeof(sai));
    sai.sin_family = AF_INET;
    sai.sin_port = htons(9999);
    if((inet_pton(AF_INET, argv[1], &(sai.sin_addr))) < 0){
        err_sys("inet_pton failed");
    }

    if(connect(sock_fd, (SA *)&sai, sizeof(sai)) < 0){
        err_sys("connect failed");
    }

    int read_count = 0;
    //while((n = read(sock_fd, recv_line, MAX_LINE)) > 0){
    //while((n = readn(sock_fd, recv_line, 3)) > 0){
    while((n = readline(sock_fd, recv_line, MAX_LINE)) > 0){
        printf("reading in %d bytes\n", n);
        read_count += n;
        recv_line[n] = 0;
        if(fputs(recv_line, stdout) < 0){
            err_sys("fputs error");
        }
    }
    if(n < 0){
        err_sys("read error");
    }
    printf("read count: %d, begins to sleep\n", read_count);
    fflush(stdout);
    sleep(20000);
    return 0;
}
