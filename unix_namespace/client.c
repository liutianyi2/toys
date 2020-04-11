#include "unp.h"


int main(int argc, char **argv){
    struct sockaddr_un un;
    int connect_fd;
    socklen_t sl;
    
    connect_fd = Socket(AF_LOCAL, SOCK_STREAM, 0);
    bzero(&un, sizeof(un));
    un.sun_family = AF_LOCAL;
    strncpy(un.sun_path, UNIX_SERVER_PATH, sizeof(un.sun_path) - 1);

    Connect(connect_fd, (SA *)&un, sizeof(un));
    str_echo_client_kqueue(stdin, connect_fd);
}

