#include "unp.h"

#define MAX_CONCURRENCY 5
#define MAX_CRAWL_TASKS_CN 20
#define CRAWL_STATE_EMPTY 0
#define CRAWL_STATE_CONNECTING 1
#define CRAWL_STATE_CONNECTED 2
#define CRAWL_STATE_RECEIVING 10
#define CRAWL_STATE_DONE 30
#define CRAWL_STATE_FAILED 40

typedef struct crawl_task_struct{
    char *host_name;
    char *file_name;
    int connect_fd;
    int file_fd;
    int state;
} crawl_task;
crawl_task crawl_tasks[MAX_CRAWL_TASKS_CN];

fd_set g_wset, g_rset;
int max_fd = 0, is_inited = 0, n_concurrency = 0, n_done = 0, n_left;
const char *get_str = "GET %s HTTP/1.1\nHost:%s\nContent-Type: text/html\nConnection: close\r\n\r\n";

void try_send(crawl_task *p_ct){
    //if(p_ct != CRAWL_STATE_CONNECTED)
    //    return;
    int n;
    char buf[MAX_LINE];
    snprintf(buf, MAX_LINE, get_str, p_ct->file_name, p_ct->host_name);

    n = writen(p_ct->connect_fd, buf, strlen(buf));
    FD_CLR(p_ct->connect_fd, &g_wset);
    FD_SET(p_ct->connect_fd, &g_rset);
    p_ct->state = CRAWL_STATE_RECEIVING;
}

int is_connection_ok(crawl_task *p_ct){
    int error, res, old_errno, ret;
    socklen_t len;
    len = sizeof(res);
    res = getsockopt(p_ct->connect_fd, SOL_SOCKET, SO_ERROR, &error, &len);
    if(res < 0 || error != 0){
        if(res >= 0){
            old_errno = errno;
            errno = error;
        }
        printf("[ERROR]getting %s from %s connect error, %s\n", p_ct->file_name, p_ct->host_name, strerror(errno));
        p_ct->state = CRAWL_STATE_FAILED;
        n_done++;
        n_concurrency--;
        if(res >= 0)
            errno = old_errno;
        Close(p_ct->connect_fd);
        ret = 0;
    }
    ret = 1;
    FD_CLR(p_ct->connect_fd, &g_rset);
    FD_CLR(p_ct->connect_fd, &g_wset);

    return ret;
}

void unblocked_connect(crawl_task *p_ct){
    if(is_inited == 0){
        is_inited = 1;
        FD_ZERO(&g_wset);
        FD_ZERO(&g_rset);
    }
    struct addrinfo *p_ai;
    int connect_fd, res, old_flag;
    p_ai = host_serv(p_ct->host_name, "http", AF_INET, SOCK_STREAM);
    char file_name_buf[MAX_LINE];

    connect_fd = Socket(p_ai->ai_family, p_ai->ai_socktype, p_ai->ai_protocol);
    old_flag = fcntl(connect_fd, F_GETFL, 0);
    fcntl(connect_fd, F_SETFL, old_flag | O_NONBLOCK);
    p_ct->connect_fd = connect_fd;
    n_left--;

    //struct sockaddr_in *p_sa;
    //char ip_buf[IP_LEN];
    //p_sa = (struct sockaddr_in *)p_ai->ai_addr;
    //printf("ip: %s\n", inet_ntop(p_sa->sin_family, &(p_sa->sin_addr), ip_buf, IP_LEN));
    //printf("port: %d\n", ntohs(p_sa->sin_port));

    res = connect(connect_fd, p_ai->ai_addr, p_ai->ai_addrlen);
    if(res < 0){
        if(errno != EINPROGRESS){
            printf("[ERROR]getting %s from %s connect error, %s\n", p_ct->file_name, p_ct->host_name, strerror(errno));
            p_ct->state = CRAWL_STATE_FAILED;
            n_done++;
            return;
        }
        p_ct->state = CRAWL_STATE_CONNECTING;
        FD_SET(connect_fd, &g_wset);
        FD_SET(connect_fd, &g_rset);
        printf("[INFO]getting %s from %s connect connecting\n", p_ct->file_name, p_ct->host_name);
        n_concurrency++;
    }else{
        n_concurrency++;
        p_ct->state = CRAWL_STATE_CONNECTED;
        try_send(p_ct);
    }
    // update max fd
    strcpy(file_name_buf, p_ct->file_name + 1);
    p_ct->file_fd = open(file_name_buf, O_WRONLY | O_CREAT, S_IRWXU | S_IRWXG);
    if(connect_fd > max_fd)
        max_fd = connect_fd;
}

