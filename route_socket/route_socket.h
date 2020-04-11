#include "unp.h"
#include <net/route.h>
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#define ROUTE_BUF_LEN (sizeof(struct rt_msghdr) + 512)
#define ROUTE_SEQ 9999

//define ROUND_UP_LEN(p, size) (((unsigned long)p & (size - 1)) ? (p - ((unsigned long)p & (size - 1)) + size) : p) 
#define ROUND_UP_LEN(p, size) (((unsigned long)p & (size - 1)) ? (size - ((unsigned long)p & (size - 1))) : 0) 
#define NEXT_SA_ADD(sa) ((SA *)((char *)sa + ROUND_UP_LEN((char *)sa, sizeof(unsigned long))))
#define NEXT_SA(sa) (sa->sa_len ? NEXT_SA_ADD(sa) : (SA *)((char *)sa + sizeof(unsigned long)))


const char *get_mask_presentation(SA *p_sa, socklen_t sa_len){
    static char buf[BUF_LEN];
    char *p_c = &(p_sa->sa_data[2]);
    if(sa_len == 0)
        return "0.0.0.0";
    if(sa_len == 5)
        snprintf(buf, BUF_LEN, "%d.0.0.0", *p_c);
    if(sa_len == 6)
        snprintf(buf, BUF_LEN, "%d.%d.0.0", *p_c, *(p_c + 1));
    if(sa_len == 7)
        snprintf(buf, BUF_LEN, "%d.%d.%d.0", *p_c, *(p_c + 1), *(p_c + 2));
    if(sa_len == 8)
        snprintf(buf, BUF_LEN, "%d.%d.%d.%d", *p_c, *(p_c + 1), *(p_c + 2), *(p_c + 3));

    return buf;
}


int init_sas(struct rt_msghdr *p_rmh, SA **sas){
    int i, count = 0;
    struct sockaddr *p_sa = (SA *)(p_rmh + 1);
    for(i = 0;i < RTAX_MAX;i++){
        if(p_rmh->rtm_addrs & (1 << i)){
            sas[i] = p_sa;
            p_sa = NEXT_SA(p_sa);
            count++;
        }else{
            sas[i] = NULL;
        }
    }

    return count;
}

char *get_route_list(int family, int flags, size_t *p_size){
    char *buf;
    int names[6];

    names[0] = CTL_NET;
    names[1] = AF_ROUTE;
    names[2] = 0;
    names[3] = family;
    names[4] = NET_RT_IFLIST;
    names[5] = flags;

    if(sysctl(names, 6, NULL, p_size, NULL, 0) < 0){
        printf("get_route_list sysctl error: %s\n", strerror(errno));
        return NULL;
    }
    buf = malloc(*p_size);
    if(buf == NULL)
        return NULL;
    if(sysctl(names, 6, buf, p_size, NULL, 0) < 0){
        printf("get_route_list sysctl error: %s\n", strerror(errno));
        free(buf);
        return NULL;
    }

    return buf;
}


