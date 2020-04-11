#include "unp.h"


union val{
    unsigned char u_char_val;
    int int_val;
    struct in_addr inaddr_val;
    struct linger linger_val;
    struct timeval time_val;
} v;


static char *print_flag_val(union val *, socklen_t *);
static char *print_int_val(union val *, socklen_t *);
static char *print_linger_val(union val *, socklen_t *);
static char *print_time_val(union val *, socklen_t *);


struct sock_option{
    int level;
    int optname;
    const char *p_description;
    char *(*print_fun)(union val *, socklen_t *);
} sock_options[] = {
    {SOL_SOCKET, SO_BROADCAST, "SO_BROADCAST", print_flag_val},
    {SOL_SOCKET, SO_DEBUG, "SO_DEBUG", print_flag_val},
    {SOL_SOCKET, SO_DONTROUTE, "SO_DONTROUTE", print_flag_val},
    {SOL_SOCKET, SO_KEEPALIVE, "SO_KEEPALIVE", print_flag_val},
    {SOL_SOCKET, SO_LINGER, "SO_LINGER", print_linger_val},
    {SOL_SOCKET, SO_OOBINLINE, "SO_OOBINLINE", print_flag_val},
    {SOL_SOCKET, SO_RCVBUF, "SO_RCVBUF", print_int_val},
    {SOL_SOCKET, SO_SNDBUF, "SO_SNDBUF", print_int_val},
    {SOL_SOCKET, SO_RCVLOWAT, "SO_RCVLOWAT", print_int_val},
    {SOL_SOCKET, SO_SNDLOWAT, "SO_SNDLOWAT", print_int_val},
    {SOL_SOCKET, SO_RCVTIMEO, "SO_RCVTIMEO", print_time_val},
    {SOL_SOCKET, SO_SNDTIMEO, "SO_SNDTIMEO", print_time_val},
    {SOL_SOCKET, SO_REUSEADDR, "SO_REUSEADDR", print_flag_val},
    {SOL_SOCKET, SO_REUSEPORT, "SO_REUSEPORT", print_flag_val},
    {SOL_SOCKET, SO_TYPE, "SO_TYPE", print_int_val},
    {SOL_SOCKET, SO_USELOOPBACK, "SO_USELOOPBACK", print_flag_val},
    {IPPROTO_IP, IP_TOS, "IP_TOS", print_int_val},
    {IPPROTO_IP, IP_TTL, "IP_TTL", print_int_val},
    {IPPROTO_TCP, TCP_MAXSEG, "TCP_MAXSEG", print_int_val},
    {IPPROTO_TCP, TCP_NODELAY, "TCP_NODELAY", print_flag_val},
};


int main(int argc, char **argv){
    int n = sizeof(sock_options) / sizeof(struct sock_option), i, socket_fd, res;
    socklen_t sl;
    for(i = 0;i < n;i++){
        switch(sock_options[i].level){
            case SOL_SOCKET:
            case IPPROTO_IP:
            case IPPROTO_TCP:
                socket_fd = socket(AF_INET, SOCK_STREAM, 0);
                break;
            default:
                printf("%s: level not supported\n", sock_options[i].p_description);
                continue;
        }

        sl = sizeof(union val);
        res = getsockopt(socket_fd, sock_options[i].level, sock_options[i].optname, &v, &sl);
        //printf("%d %d\n", res, sl);
        if(res < 0)
            printf("error found %d\n", res);
        printf("%s: %s\n", sock_options[i].p_description, sock_options[i].print_fun(&v, &sl));
    }
}

static char *print_flag_val(union val *p_uv, socklen_t *p_sl){
    static char output[100];
    if(*p_sl != sizeof(int)){
        snprintf(output, sizeof(output) / sizeof(char), "wrong value len, %d", *p_sl);
    }else{
        snprintf(output, sizeof(output) / sizeof(char), "%s", (p_uv->int_val == 0) ? "no": "yes");
    }
    return output;
}

static char *print_int_val(union val *p_uv, socklen_t *p_sl){
    static char output[100];
    if(*p_sl != sizeof(int)){
        snprintf(output, sizeof(output) / sizeof(char), "wrong value len, %d", *p_sl);
    }else{
        snprintf(output, sizeof(output) / sizeof(char), "%d", p_uv->int_val);
    }
    return output;
}

static char *print_linger_val(union val *p_uv, socklen_t *p_sl){
    static char output[100];
    if(*p_sl != sizeof(struct linger)){
        snprintf(output, sizeof(output) / sizeof(char), "wrong value len, %d", *p_sl);
    }else{
        struct linger *p_lg = &(p_uv->linger_val);
        snprintf(output, sizeof(output) / sizeof(char), "%s, %d", (p_lg->l_onoff) == 0 ? "off" : "on", p_lg->l_linger);
    }
    return output;
}

static char *print_time_val(union val *p_uv, socklen_t *p_sl){
    static char output[100];
    if(*p_sl != sizeof(struct timeval)){
        snprintf(output, sizeof(output) / sizeof(char), "wrong value len, %d", *p_sl);
    }else{
        struct timeval *p_tv = &(p_uv->time_val);
        snprintf(output, sizeof(output) / sizeof(char), "%ld seconds, %d useconds", p_tv->tv_sec, p_tv->tv_usec);
    }
    return output;
}

