#include "unp.h"


int main(int argc, char **argv){
    int un_fd;
    socklen_t sl;
    struct sockaddr_un un1, un2;

    if(argc != 2){
        err_sys("cmd: ./test unix_namespace_path");
    }

    un_fd = Socket(AF_LOCAL, SOCK_STREAM, 0);
    unlink(argv[1]);

    bzero(&un1, sizeof(un1));
    un1.sun_family = AF_LOCAL;
    strncpy(un1.sun_path, argv[1], sizeof(un1.sun_path) - 1);
    Bind(un_fd, (SA *)(&un1), SUN_LEN(&un1));
    
    sl = sizeof(un2);
    getsockname(un_fd, (SA *)&un2, &sl);
    printf("result path: %s, result length: %d\n", un2.sun_path, sl);

    close(un_fd);
}
