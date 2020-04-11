#include "unp.h"
#include <time.h>

void sig_child(int sig){
    pid_t pid;
    int status;

    //pid = wait(&status);
    while((pid = waitpid(-1, &status, WNOHANG)) > 0){
        printf("%d==========sub process done confirmed, pid: %d, status: %d\n", getpid(), pid, status);
    }
}


int main(int argc, char **argv){
    int listen_fd, connect_fd;
    struct sockaddr_in sai, sai_in;
    socklen_t sai_len;
    char buf[MAX_LINE + 1];
    char ip_buf[MAX_LINE + 1];
    time_t tt;

    Signal(SIGCHLD, sig_child);

    listen_fd = Socket(AF_INET, SOCK_STREAM, 0);
    bzero(&sai, sizeof(sai));
    sai.sin_family = AF_INET;
    sai.sin_port = htons(SERVER_PORT);
    sai.sin_addr.s_addr = htonl(INADDR_ANY);
    Bind(listen_fd, (SA *)&sai, sizeof(sai));

    Listen(listen_fd, LISTENQL);
    int i = 0, len;
    pid_t pid = getpid();
    while(1){
        connect_fd = Accept(listen_fd, (SA *)&sai_in, &sai_len);
        printf("%d==========receive query from ip: %s, port: %d\n", pid, inet_ntop(AF_INET, &(sai_in.sin_addr), ip_buf, MAX_LINE), ntohs(sai_in.sin_port));
        if(fork() == 0){
            pid = getpid();
            printf("%d##########forking sub process, pid: %d\n", pid, pid);
            close(listen_fd);
            //echo_str(connect_fd, pid);
            str_echo_server_file(connect_fd);
            close(connect_fd);
            exit(0);
        }
        close(connect_fd);
    }
}
