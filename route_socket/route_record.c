#include "route_socket.h"


int main(int argc, char **argv){
    if(argc != 2)
        err_sys("cmd: ./route_record ip");

    struct rt_msghdr *p_rmh;
    struct sockaddr_in *p_sa;
    struct sockaddr *ps, *pss[RTAX_MAX];
    char buf[ROUTE_BUF_LEN];
    bzero(buf, ROUTE_BUF_LEN);

    pid_t pid = getpid();
    p_rmh = (struct rt_msghdr *)buf;
    p_rmh->rtm_msglen = sizeof(struct rt_msghdr) + sizeof(struct sockaddr_in);
    p_rmh->rtm_type = RTM_GET;
    p_rmh->rtm_seq = ROUTE_SEQ;
    p_rmh->rtm_pid = pid;
    p_rmh->rtm_version = RTM_VERSION;

    p_sa = (struct sockaddr_in *)(buf + sizeof(struct rt_msghdr));
    p_sa->sin_family = AF_INET;
    p_sa->sin_len = sizeof(struct sockaddr_in);
    inet_pton(AF_INET, argv[1], &(p_sa->sin_addr));

    int sock_fd = Socket(AF_ROUTE, SOCK_RAW, 0);
    int n;
    write(sock_fd, p_rmh, p_rmh->rtm_msglen);
    printf("%d\n", p_rmh->rtm_msglen);
    while(1){
        n = read(sock_fd, p_rmh, ROUTE_BUF_LEN);
        printf("%d received\n", n);
        if(p_rmh->rtm_type == RTM_GET && p_rmh->rtm_seq == ROUTE_SEQ && p_rmh->rtm_pid == pid)
            break;
    }

    n = init_sas(p_rmh, pss);
    //printf("%d\n", n);
    int i;
    char ip_buf[IP_LEN];
    for(i = 0;i < RTAX_MAX;i++){
        if(pss[i] != NULL){
            if(i == RTAX_DST){
                printf("dst: %s\n", inet_ntop(pss[i]->sa_family, &((struct sockaddr_in *)pss[i])->sin_addr, ip_buf, IP_LEN));
            }else if(i == RTAX_GATEWAY){
                printf("gate way: %s\n", inet_ntop(pss[i]->sa_family, &((struct sockaddr_in *)pss[i])->sin_addr, ip_buf, IP_LEN));
            }else if(i == RTAX_NETMASK){
                printf("netmask: %s\n", get_mask_presentation(pss[i], pss[i]->sa_len));
            }else if(i == RTAX_GENMASK){
                printf("genmask: %s\n", get_mask_presentation(pss[i], pss[i]->sa_len));
            }
        }
    }
}

