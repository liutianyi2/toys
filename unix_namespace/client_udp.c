#include "unp.h"


int main(int argc, char **argv){
    struct sockaddr_un un, un_me;
    int connect_fd;
    socklen_t sl;
    
    connect_fd = Socket(AF_LOCAL, SOCK_DGRAM, 0);
    bzero(&un, sizeof(un));
    un.sun_family = AF_LOCAL;
    strncpy(un.sun_path, UNIX_SERVER_PATH, sizeof(un.sun_path) - 1);

    bzero(&un_me, sizeof(un_me));
    un_me.sun_family = AF_LOCAL;
    strcpy(un_me.sun_path, tmpnam(NULL));
    Bind(connect_fd, (SA *)&un_me, sizeof(un_me));

    echo_client_udp(connect_fd, (SA *)&un, sizeof(un));
    exit(0);
}

