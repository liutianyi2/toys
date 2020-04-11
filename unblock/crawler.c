#include "unblock.h"


int main(int argc, char **argv){
    if(argc < 3){
        err_sys("cmd: ./crawler hostname file1 ...");
    }
    int n_concurrency = 0, n_files = argc - 2, i, n, sum;
    fd_set rset, wset;
    char buf[MAX_LINE];
    n_files = min(n_files, MAX_CRAWL_TASKS_CN);
    printf("[INFO]ready to crawl %d files;\n", n_files);

    //初始化
    for(i = 0;i < n_files;i++){
        crawl_tasks[i].host_name = argv[1];
        crawl_tasks[i].file_name = argv[i + 2];
        crawl_tasks[i].connect_fd = -1;
        crawl_tasks[i].state = CRAWL_STATE_EMPTY;
    }
    n_left = n_files;
    while(n_done < n_files){
        for(i = 0;i < n_files;i++){
            if(n_concurrency < MAX_CONCURRENCY && n_left > 0){
                if(crawl_tasks[i].state == CRAWL_STATE_EMPTY)
                  unblocked_connect(&(crawl_tasks[i]));
            }else{
                break;
            }
        }
        if(n_done == n_files)
            exit(0);

        rset = g_rset;
        wset = g_wset;
        //printf("begins to select\n");
        //printf("is the only fd set: %d\n", FD_ISSET(crawl_tasks[0].connect_fd, &rset));
        n = Select(max_fd + 1, &rset, &wset, NULL, NULL);
        //printf("select done: %d\n", n);
        for(i = 0;i < n_files;i++){
            if(crawl_tasks[i].state == CRAWL_STATE_CONNECTING && (FD_ISSET(crawl_tasks[i].connect_fd, &rset) || FD_ISSET(crawl_tasks[i].connect_fd, &wset))){
                if(is_connection_ok(&(crawl_tasks[i]))){
                    try_send(&(crawl_tasks[i]));
                }
                continue;
            }
            if(crawl_tasks[i].state == CRAWL_STATE_RECEIVING && FD_ISSET(crawl_tasks[i].connect_fd, &rset)){
                sum = 0;
                while(1){
                    n = read(crawl_tasks[i].connect_fd, buf, MAX_LINE - 1);
                    //printf("read %d done\n", n);
                    if(n <= 0){
                        if(n == 0){
                            printf("[INFO]receive done from %s, after having received %d\n", crawl_tasks[i].file_name, sum);
                            crawl_tasks[i].state = CRAWL_STATE_DONE;
                        }else{
                            if(errno == EWOULDBLOCK){
                                printf("[INFO]receive a part from %s, after having received %d\n", crawl_tasks[i].file_name, sum);
                                break;
                            }
                            printf("[ERROR]receive error from %s, msg: %s, after having received %d\n", crawl_tasks[i].file_name, strerror(errno), sum);
                            crawl_tasks[i].state = CRAWL_STATE_FAILED;
                        }
                        n_concurrency--;
                        n_done++;
                        n_left--;
                        FD_CLR(crawl_tasks[i].connect_fd, &g_rset);
                        Close(crawl_tasks[i].connect_fd);
                        Close(crawl_tasks[i].file_fd);
                        break;
                    }
                    else{
                        buf[n] = 0;
                        //printf("receiving: %s\n", buf);
                        sum += n;
                        write(crawl_tasks[i].file_fd, buf, n);
                    }
                }
            }
        }
    }
}

