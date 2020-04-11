#include "unp.h"


int main(int argc, char **argv){
    if(fork() > 0){
        exit(0);
    }
    printf("in sub\n");
    if(setsid() < 0){
        err_sys("setsid error");
    }
    printf("succeed\n");
    sleep(100);
}

