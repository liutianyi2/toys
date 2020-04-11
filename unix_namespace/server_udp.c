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

    un_fd = Socket(AF_LOCAL, SOCK_DGRAM, 0);
    bzero(&un, sizeof(un));
    un.sun_family = AF_LOCAL;
    strncpy(un.sun_path, UNIX_SERVER_PATH, sizeof(un.sun_path) - 1);
    Bind(un_fd, (SA *)&un, sizeof(un));

    echo_server_udp_universal(un_fd);
}
