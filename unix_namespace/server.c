#include "unp.h"


void sig_child(int sig){
    pid_t pid;
    int status;

    while((pid = waitpid(-1, &status, WNOHANG)) > 0){
        printf("%d==========sub process done confirmed, pid: %d, status: %d\n", getpid(), pid, status);
    }
}


int main(int argc, char **argv){
    struct sockaddr_un un, un2;
    socklen_t sl;
    char buf[MAX_LINE];
    int un_fd, connect_fd, res;
    pid_t pid = getpid();

    unlink(UNIX_SERVER_PATH);

    un_fd = Socket(AF_LOCAL, SOCK_STREAM, 0);
    bzero(&un, sizeof(un));
    un.sun_family = AF_LOCAL;
    strncpy(un.sun_path, UNIX_SERVER_PATH, sizeof(un.sun_path) - 1);
    Bind(un_fd, (SA *)&un, sizeof(un));
    Listen(un_fd, LISTENQL);

    Signal(SIGCHLD, sig_child);

    while(1){
        bzero(&un2, sizeof(un2));
        sl = sizeof(un2);
        if((connect_fd = accept(un_fd, (SA *)(&un2), &sl)) < 0){
            if(errno == EINTR)
                continue;
            err_sys("accept error");
        }
        printf("%d==========request received, from %s\n", pid, un2.sun_path);
        res = fork();
        if(res == 0){
            pid = getpid();
            close(un_fd);
            //echo_str_with_credit(connect_fd, pid);
            echo_str(connect_fd, pid);
            exit(0);
        }
        close(connect_fd);
    }
}
