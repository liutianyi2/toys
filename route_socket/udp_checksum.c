#include "unp.h"
#include <netinet/ip_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>
#include <sys/param.h>
#include <sys/sysctl.h>

int main(){
    int names[4];
    int res;
    size_t len;
    len = sizeof(res);

    names[0] = CTL_NET;
    names[1] = AF_INET;
    names[2] = IPPROTO_UDP;
    names[3] = UDPCTL_CHECKSUM;

    sysctl(names, 4, &res, &len, NULL, 0);
    printf("%d\n", res);
}
