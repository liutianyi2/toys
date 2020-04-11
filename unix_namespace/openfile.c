#include "unp.h"


int main(int argc, char **argv){
    if(argc != 4){
        exit(255);
    }
    int fd, mode, filefd;
    char *path = argv[1];
    mode = atoi(argv[2]);
    fd = atoi(argv[3]);

    if((filefd = open(path, mode)) < 0){
        exit(errno);
    }

    if(write_open_fd(fd, filefd, "", 1) < 0){
        exit(errno);
    }

    exit(0);
}
