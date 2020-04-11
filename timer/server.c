#include "unp.h"
#include <time.h>


int main(int argc, char **argv){
    int listen_fd, connect_fd;
    struct sockaddr_in sai, sai_in;
    socklen_t sai_len;
    char buf[MAX_LINE + 1];
    char ip_buf[MAX_LINE + 1];
    time_t tt;

    listen_fd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&sai, sizeof(sai));
    sai.sin_family = AF_INET;
    sai.sin_port = htons(9999);
    sai.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(listen_fd, (SA *)&sai, sizeof(sai));

    Listen(listen_fd, LISTENQL);
    int i = 0, len;
    while(1){
        connect_fd = Accept(listen_fd, (SA *)&sai_in, &sai_len);
        printf("receive query from ip: %s, port: %d\n", inet_ntop(AF_INET, &(sai_in.sin_addr), ip_buf, MAX_LINE), ntohs(sai_in.sin_port));
        tt = time(NULL);
        snprintf(buf, MAX_LINE, "%.24s\n", ctime(&tt));

        len = strlen(buf);
        while(i < len){
            Write(connect_fd, buf + i, 1);
            i++;
            //sleep(1);
        }

        //Write(connect_fd, buf, strlen(buf));
        //sleep(2);
        // Write(connect_fd, buf, strlen(buf));
        //Write(connect_fd, buf, strlen(buf));
        //sleep(1000);
        close(connect_fd);
        printf("connection closed\n");
    }
}
