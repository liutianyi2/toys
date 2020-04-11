#include "unp.h"


int main(int argc, char **argv){
    if(argc != 2){
        err_sys("cmd: ./a.out file_path");
    }

    int fd, n;
    char buf[MAX_LINE];
    if((fd = myopen(argv[1], O_RDONLY)) < 0){
        err_sys("myopen error");
    }

    while((n = Read(fd, buf, MAX_LINE)) > 0)
        Write(fileno(stdout), buf, n);

    exit(0);
}

