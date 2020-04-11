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
    int i = 0, len, max_fd = listen_fd, max_i = -1, connect_fds[FD_SETSIZE], n;
    pid_t pid = getpid();
    fd_set rset, all_set;
    FD_ZERO(&all_set);
    FD_SET(listen_fd, &all_set);
    for(;i < FD_SETSIZE;i++){
        connect_fds[i] = -1;
    }

    while(1){
        rset = all_set;
        //printf("%d max_fd", max_fd);
        Select(max_fd + 1, &rset, NULL, NULL, NULL);
        //printf("select done");

        if(FD_ISSET(listen_fd, &rset)){
            connect_fd = Accept(listen_fd, (SA *)&sai_in, &sai_len);
            printf("%d==========receive query from ip: %s, port: %d\n", pid, inet_ntop(AF_INET, &(sai_in.sin_addr), ip_buf, MAX_LINE), ntohs(sai_in.sin_port));
            for(i = 0;i <= (max_i + 1);i++){
                if(connect_fds[i] == -1){
                    connect_fds[i] = connect_fd;
                    break;
                }
            }
            if(i > max_i)
                max_i = i;
            FD_SET(connect_fd, &all_set);
            //printf("%d %d %d\n", connect_fd, max_i, max_fd);
        }

        max_fd = listen_fd;
        for(i = 0;i <= max_i;i++){
            if(connect_fds[i] != -1){
                if(FD_ISSET(connect_fds[i], &rset)){
                    n = Read(connect_fds[i], buf, sizeof(buf));
                    if(n == 0){
                        printf("%d##########receiving and sending %d bytes, done\n", connect_fds[i], n);
                        FD_CLR(connect_fds[i], &all_set);
                        close(connect_fds[i]);
                        connect_fds[i] = -1;
                        if(max_i == i){
                            while(max_i >= 0 && connect_fds[max_i] == -1)
                              max_i--;
                        }
                        continue;
                    }else{
                        Write(connect_fds[i], buf, n);
                        printf("%d##########receiving and sending %d bytes, continue\n", connect_fds[i], n);
                    }
                }
                if(connect_fds[i] > max_fd)
                    max_fd = connect_fds[i];
            }
        }
    }
}
