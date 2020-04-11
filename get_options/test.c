#include "unp.h"
#include <time.h>
#define OPEN_MAX 1024

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
    int i, nfds, n, j, m;
    pid_t pid = getpid();
    struct pollfd connect_fds[OPEN_MAX];
    short events = POLLRDNORM;

    connect_fds[0].fd = listen_fd;
    connect_fds[0].events = events;
    for(i = 1;i < OPEN_MAX;i++)
      connect_fds[i].fd = -1;

    fd_set rset, all_set;
    FD_ZERO(&all_set);
    FD_SET(listen_fd, &all_set);
    for(;i < FD_SETSIZE;i++){
        connect_fds[i].fd = -1;
    }
    nfds = 1;

    while(1){
        n = Poll(connect_fds, nfds, -1);
        //printf("aaaaaaa: %d %d\n", nfds, n);
        for(i = nfds - 1;i >= 0;i--){
            if(n <= 0)
                break;
            if(connect_fds[i].fd == -1 || connect_fds[i].revents == 0)
                continue;
            n--;
            if(i == 0){
                connect_fd = Accept(listen_fd, (SA *)&sai_in, &sai_len);
                printf("%d==========receive query from ip: %s, port: %d\n", pid, inet_ntop(AF_INET, &(sai_in.sin_addr), ip_buf, MAX_LINE), ntohs(sai_in.sin_port));
                for(j = 1;j < OPEN_MAX;j++){
                    if(connect_fds[j].fd == -1)
                        break;
                }
                if(j >= OPEN_MAX){
                    err_sys("too many connects, self finished");
                }
                connect_fds[j].fd = connect_fd;
                connect_fds[j].events = events;
                if(j == nfds)
                    nfds++;
            }else{
                //printf("checkout\n");
                if(connect_fds[i].revents & (POLLRDNORM | POLLERR)){
                    m = Read(connect_fds[i].fd, buf, sizeof(buf));
                    if(m == 0){
                        printf("%d##########receiving and sending %d bytes, done\n", connect_fds[i].fd, m);
                        close(connect_fds[i].fd);
                        connect_fds[i].fd = -1;
                        if(i == (nfds - 1)){
                            while(nfds > 1 && connect_fds[nfds - 1].fd == -1)
                                nfds--;
                        }
                    }else{
                        printf("%d##########receiving and sending %d bytes, continue\n", connect_fds[i].fd, n);
                        Write(connect_fds[i].fd, buf, m);
                    }
                }
            }
        }
    }
}
