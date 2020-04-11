#include "unp.h"


int main(int argc, char **argv){
    int listen_fd;
    socklen_t sl, sl_ori;
    struct sockaddr *p_sa;
    char buf[MAX_LINE], ip_buf[IP_LEN];
    ssize_t nbytes;
    struct sockaddr_in sai;
    socklen_t slen = sizeof(sai);
    if(argc != 2){
        err_sys("cmd: ./upd_server service_name");
    }

    listen_fd = udp_server(NULL, argv[1], &sl);
    getsockname(listen_fd, (SA *)&sai, &slen);
    printf("listening on ip: %s, port: %d\n", inet_ntop(AF_INET, &(sai.sin_addr), ip_buf, IP_LEN), sai.sin_port);
    p_sa = malloc(sl); 
    sl_ori = sl;
    while(1){
        sl = sl_ori;
        nbytes = Recvfrom(listen_fd, buf, MAX_LINE - 1, 0, p_sa, &sl);
        if(p_sa->sa_family == AF_INET){
            printf("client ip: %s, port: %d\n", inet_ntop(AF_INET, &(((struct sockaddr_in *)(p_sa))->sin_addr), ip_buf, IP_LEN), ntohs(((struct sockaddr_in *)(p_sa))->sin_port));
        }
        Sendto(listen_fd, buf, nbytes, 0, p_sa, sl);
    }
}

