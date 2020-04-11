#include "unp.h"

void sigpipe_handler(int sig){
    printf("sig pipe received\n");
    exit(0);
}


int main(int argc, char **argv){
    int sock_fd, n;
    struct sockaddr_in sai, sai2;

    if(argc != 2){
        err_sys("cmd: ./a.out <IP>");
    }

    int i, sock_fds[5];
    signal(SIGPIPE, sigpipe_handler);

    int opt;
    socklen_t sl = sizeof(int);

    for(i = 0;i < 1; i++){
        if((sock_fd = (socket(AF_INET, SOCK_STREAM, 0))) <= 0){
            err_sys("socket failed");
        }

        // temp code
        getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &opt, &sl);
        //printf("before rcvbuf: %d\n", opt);
        sl = sizeof(int);
        getsockopt(sock_fd, IPPROTO_TCP, TCP_MAXSEG, &opt, &sl);
        opt = 1;
        setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        //printf("before rcvbuf: %d\n", opt);

        int tos = 32;
        setsockopt(sock_fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos));

        memset(&sai, 0, sizeof(sai));
        sai.sin_family = AF_INET;
        sai.sin_port = htons(SERVER_PORT);
        if((inet_pton(AF_INET, argv[1], &(sai.sin_addr))) < 0){
            err_sys("inet_pton failed");
        }

        // temp code
        //memset(&sai2, 0, sizeof(sai2));
        //sai2.sin_family = AF_INET;
        //sai2.sin_port = htons(6666);
        //if((inet_pton(AF_INET, "127.0.0.1", &(sai2.sin_addr))) < 0){
        //    err_sys("inet_pton failed");
        //}
        //Bind(sock_fd, (SA *)&sai2, sizeof(sai2));

        if(connect(sock_fd, (SA *)&sai, sizeof(sai)) < 0){
            err_sys("connect failed");
        }
        //sleep(100);

        // temp code
        sl = sizeof(int);
        getsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &opt, &sl);
        //printf("after rcvbuf: %d\n", opt);
        sl = sizeof(int);
        getsockopt(sock_fd, IPPROTO_TCP, TCP_MAXSEG, &opt, &sl);
        //printf("after rcvbuf: %d\n", opt);

        sock_fds[i] = sock_fd;
    }

    //echo_client_select(sock_fds[0]);
    //str_echo_client_kqueue(stdin, sock_fds[0]);
    echo_client_unblocked(sock_fds[0]);

    exit(100);
}
