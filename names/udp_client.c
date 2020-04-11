#include "unp.h"


int main(int argc, char **argv){
    struct sockaddr *p_to;
    socklen_t sl;
    int sock_fd, nbytes;
    char buf[MAX_LINE];
    if(argc != 3){
        err_sys("cmd: ./udp_client host_name service_name");
    }

    sock_fd = udp_client(argv[1], argv[2], &p_to, &sl);
    Sendto(sock_fd, " ", 1, 0, p_to, sl);
    nbytes = Recvfrom(sock_fd, buf, MAX_LINE - 1, 0, NULL, NULL);
    buf[nbytes] = 0;

    fputs(buf, stdout);
}

