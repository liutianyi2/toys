#include "unp.h"

void hup_handler(int signo){
    int fd = open("/Users/zrb/unp/daemon/last_sentences", O_WRONLY|O_CREAT);
    write(fd, "hup received", sizeof("hup received"));
    exit(0);
}

int main(int argc, char **argv){
    Signal(SIGHUP, hup_handler);
    while(1){
        sleep(10);
    }
}

