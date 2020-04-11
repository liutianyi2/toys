#include "unp.h"
#include <net/if.h>
#include <net/if_arp.h>
#include <sys/ioctl.h>

int main(int argc, char **argv){
    if(argc != 2)
        err_sys("cmd: ./arp_test ipv4_addr");
    struct sockaddr_in sai;
    bzero(&sai, sizeof(sai));
    sai.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &(sai.sin_addr));

    struct arpreq ar;
    bzero(&ar, sizeof(ar));
    memcpy(&(ar.arp_pa), &sai, sizeof(sai));
    int fd;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    ioctl(fd, SIOCGARP, &ar);
    //ioctl(fd, SIOCGIFFLAGS, &ar);

    char *pc = (char *)(&(ar.arp_ha));
    printf("%x:%x:%x:%x:%x:%x\n", *pc, *(pc + 1), *(pc + 2), *(pc + 3), *(pc + 4), *(pc + 5));
}

