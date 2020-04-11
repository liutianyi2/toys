#include "unp.h"


int main(int argc, char **argv){
    int i;
    struct hostent *p_he;
    char **p_arg;
    char **p_temp;
    char IP_BUF[IP_LEN];
    struct sockaddr_in sai;
    if(argc < 2){
        err_sys("cmdline: ./a.out hostname1 hostname2...");
    }

    p_arg = argv + 1;
    for(i = 1;i < argc;i++){
        inet_pton(AF_INET, *p_arg, (int *)(&(sai.sin_addr)));
        if((p_he = gethostbyaddr((char *)(&(sai.sin_addr)), sizeof(sai.sin_addr), AF_INET)) != NULL){
            printf("gethostbyname for %s succeed:\ncanonical name: %s\n", *p_arg, p_he->h_name);
            p_temp = p_he->h_aliases;
            while(*p_temp){
                printf("alias: %s\n", *p_temp);
                p_temp++;
            }
            switch(p_he->h_addrtype){
                case AF_INET:
                    printf("protocal type: IPV4\n");
                    p_temp = p_he->h_addr_list;
                    while(*p_temp){
                        printf("ptr name: %s\n", inet_ntop(AF_INET, *p_temp, IP_BUF, IP_LEN));
                        p_temp++;
                    }
                    break;
                default:
                    printf("unknown type of protocal: %d\n", p_he->h_addrtype);
                printf("\n");
            }
        }else{
            printf("gethostbyname for %s failed: %s\n\n", *p_arg, hstrerror(h_errno));
        }
        p_arg++;
    }
}

