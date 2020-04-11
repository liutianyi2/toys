#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <stdarg.h>
#include <sys/event.h>
#include <sys/ioctl.h>
#include <sys/un.h>

#define MAX_LINE 1000
#define BUF_LEN 1000
#define MAX_FD 64 
#define IP_LEN 100
#define LISTENQL 5
#define SERVER_PORT 9999
#define READ_BUF_LEN 2000
#define WRITE_BUF_LEN 2000
#define SA struct sockaddr
#define max(a, b) ((a)>(b)?(a):(b))
#define min(a, b) ((a)<(b)?(a):(b))
#define UNIX_SERVER_PATH "/Users/zrb/unp/unix_namespace/unix_test"
typedef void sigfun(int);

int err_sys_syslog = 0;

void err_sys(const char* ps_error){
    if(err_sys_syslog){
        syslog(LOG_ERR, "%s\n", ps_error);
        syslog(LOG_ERR, "%s\n", strerror(errno));
    }else{
        perror(ps_error);
    }
    exit(errno);
}

void err_msg(const char *ps_error){ 
    if(err_sys_syslog){
        syslog(LOG_NOTICE, "%s", ps_error);
    }else{
        printf("%s", ps_error);
    }
}

int fd_readable_timeo(int fd, int nsecs){
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    int res;
    struct timeval tv;
    bzero(&tv, sizeof(tv));
    tv.tv_sec = nsecs;

    res = select(fd + 1, &fds, NULL, NULL, &tv);

    return res;
}


int Socket(int domain, int type, int protocol){
    int fd = socket(domain, type, protocol);
    if(fd < 0){
        err_sys("socket failed");
    }

    return fd;
}

int Inet_pton(int domain, const char* ip_str, int *p_dst){
    int ret;
    if((ret = inet_pton(domain, ip_str, p_dst)) < 0){
        err_sys("inet_pton error");
    }

    return ret;
}

int Connect(int fd, SA *p_sa, int len){
    int res;
    if((res = connect(fd, p_sa, len)) < 0){
        err_sys("connect error");
    }

    return res;
}


int Read(int fd, char *p_buf, int len){
    int res;
    if((res = read(fd, p_buf, len)) < 0){
        if(errno == EINTR)
          return Read(fd, p_buf, len);
        err_sys("read error");
    }

    return res;
}

int readn(int fd, char *p_buf, unsigned int len){
    unsigned int len_read = 0;
    int res;
    while(1){
        if(len_read >= len)
          break;
        res = read(fd, p_buf + len_read, len - len_read);
        if(res == 0)
          return len_read;
        if(res < 0){
            if(errno != EINTR){
                err_sys("readn error");
            }else{
                res = 0;
            }
        }
        len_read += res;
    }

    return len_read;
}

int readline(int fd, char *p_buf, unsigned int len){
    char c;
    unsigned int read_cn = 0;
    int res;
    while(read_cn < len - 1){
        res = read(fd, &c, 1);
        if(res < 0){
            if(errno == EINTR)
              continue;
            err_sys("readline error");
        }
        if(res == 0 || c == '\n'){
            if(c == '\n')
                p_buf[read_cn++] = c;
            break;
        }
        p_buf[read_cn++] = c;
    }

    p_buf[read_cn] = 0;
    return read_cn;
}

int Fputs(const char *p_buf, FILE *stream){
    int res;
    if((res = fputs(p_buf, stream)) < 0)
      err_sys("fputs error");

    return res;
}

int writen(int fd, const char *p_buf, int len){
    int sum = 0, n;
    while(sum < len){
        n = write(fd, p_buf + sum, len - sum);
        if(n < 0){
            if(errno == EINTR || errno == EWOULDBLOCK)
                continue;
            err_sys("writen error");
        }
        sum += n;
    }

    return sum;
}

int Write(int fd, const char *p_buf, int len){
    int res;
    if((res = write(fd, p_buf, len)) < 0){
        //printf("write returns %d\n", res);
        err_sys("write error");
    }

    return res;
}

int Close(int fd){
    int res;
    if((res = close(fd)) < 0)
        err_sys("close error");

    return res;
}

int Listen(int fd, int backlog){
    int res;
    if((res = listen(fd, backlog)) < 0)
        err_sys("listen error");

    return res;
}

int Bind(int fd, SA *p_sa, int len){
    int res;
    if((res = bind(fd, p_sa, len)) < 0)
        err_sys("bind error");

    return res;
}

int Accept(int fd, SA *p_sa, socklen_t *p_len){
    int res;
    if((res = accept(fd, p_sa, p_len)) < 0){
        if(errno == EINTR){
            return Accept(fd, p_sa, p_len);
        }
        err_sys("accept error");
    }

    return res;
}

//int read_credit(int connect_fd, char *pbuf, size_t nbytes, struct cmsgcred *pcmed){
//    struct msghdr msg;
//    struct iovec iov[1];
//    char control[CMSG_SPACE(sizeof(struct cmsgcred))];
//    struct cmsghdr *pch;
//    int n;
//
//    msg.msg_control = control;
//    msg.msg_controllen = sizeof(control);
//    msg.msg_name = NULL;
//    msg.msg_namelen = 0;
//    iov[0].iov_base = pbuf;
//    iov[0].iov_len = nbytes;
//    msg.msg_iov = iov;
//    msg.msg_iovlen = 1;
//    msg.msg_flags = 0;
//
//    n = recvmsg(connect_fd, &msg, 0);
//    pcmed->cmcred_ngroups = 0;
//    if(n < 0)
//        return n;
//    if(msg.msg_controllen != 0){
//        pch = CMSG_FIRSTHDR(msg.msg_control);
//        if(pch != NULL){
//            if(pch->cmsg_len == CMSG_LEN(sizeof(struct cmsgcred)) && pch->cmsg_level == SOL_SOCKET && pch->cmsg_type == SCM_CREDS){
//                strncpy(pcmed, CMSG_DATA(pch), sizeof(struct cmsgcred));
//            }
//        }
//    }
//
//    return n;
//}
//
//void echo_str_with_credit(int fd, pid_t pid){
//    char buf[MAX_LINE];
//    int n, is_first = 1;
//    struct cmsgcred med;
//
//    while(1){
//        n = read_credit(fd, buf, MAX_LINE, &med);
//        if(n < 0){
//            err_sys("echo_str_with_credit, recvmsg error");
//        }
//        if(is_first){
//            is_first = 0;
//            if(med.cmcred_ngroups == 0){
//                printf("%d##########, client not credited\n", pid);
//            }else{
//                printf("%d##########, client credited, pid: %d, uid: %d, euid: %d, gid: %d\n", med.cmcred_pid, med.cmcred_uid, med.cmcred_euid, med.cmcred_gid)
//            }
//        }
//        if(n == 0){
//            printf("%d##########done\n", pid);
//            return;
//        }
//        Write(fd, buf, strlen(buf));
//        printf("%d##########receiving and sending %d bytes\n", pid, n);
//    }
//}

void echo_str(int fd, pid_t pid){
    char buf[MAX_LINE];
    int n;
    long n1, n2;
    while(1){
        n = readline(fd, buf, MAX_LINE);
        if(n == 0){
            printf("%d##########done\n", pid);
            return;
        }
        //if(sscanf(buf, "%ld %ld", &n1, &n2) != 2){
        //    snprintf(buf, MAX_LINE, "wrong type of input\n");
        //}else{
        //    snprintf(buf, MAX_LINE, "sum of the two: %ld\n", n1 + n2);
        //}
        Write(fd, buf, strlen(buf));
        printf("%d##########receiving and sending %d bytes\n", pid, n);
    }
}

int Select(int nfds, fd_set *p_read, fd_set *p_write, fd_set *p_error, struct timeval *p_time){
    int n;
    if((n = select(nfds, p_read, p_write, p_error, p_time)) < 0){
        if(errno == EINTR)
          return n;
        err_sys("Select error");
    }

    return n;
}

int Poll(struct pollfd *fdarray, unsigned long nfds, int timeout){
    int n;
    if((n = poll(fdarray, nfds, timeout)) < 0){
        if(n < 0){
            if(errno == EINTR)
              return n;
            err_sys("Poll error");
        }
    }

    return n;
}

void echo_client_select(int fd){
    fd_set read_set;
    FD_ZERO(&read_set);

    int max_fd = max(fileno(stdin), fd) + 1, n, is_need_in = 1;
    char buf[MAX_LINE];
    while(1){
        FD_SET(fd, &read_set);
        if(is_need_in){
            FD_SET(fileno(stdin), &read_set);
        }else{
            max_fd = fd + 1;
        }

        Select(max_fd, &read_set, NULL, NULL, NULL);

        if(FD_ISSET(fd, &read_set)){
            n = readline(fd, buf, MAX_LINE);
            if(n == 0){
                if(is_need_in)
                    printf("server ended prematurely\n");
                return;
            }
            fputs(buf, stdout);
        }

        if(FD_ISSET(fileno(stdin), &read_set)){
            if((n = Read(fileno(stdin), buf, MAX_LINE)) > 0){
                //printf("writing %d bytes\n", n);
                Write(fd, buf, n);
            }else{
                //return;
                shutdown(fd, SHUT_WR);
                is_need_in = 0;
            }
        }
    }
}

void echo_client(int fd){
    char send_line[MAX_LINE], receive_line[MAX_LINE];
    int receive_cn;
    while(fgets(send_line, MAX_LINE, stdin)){
        Write(fd, send_line, strlen(send_line));
        //sleep(1);
        //Write(fd, send_line, strlen(send_line));
        //printf("writing %s\n", send_line);
        //Write(fd, send_line, strlen(send_line));
        //printf("writing %s\n", send_line);
        receive_cn = readline(fd, receive_line, MAX_LINE);
        if(receive_cn > 0){
            fputs(receive_line, stdout);
        }else{
            printf("server end prematurely\n");
            return;
        }
    }
}


sigfun *Signal(int sig, sigfun *psf){
    struct sigaction ns, os;
    ns.sa_handler = psf;
    sigemptyset(&(ns.sa_mask));
    ns.sa_flags = 0;
    if(sig == SIGALRM){
        ns.sa_flags |= SA_RESTART;
    }

    if(sigaction(sig, &ns, &os) < 0){
        err_sys("Signal error");
    }

    return os.sa_handler;
}

int connect_timeo(int fd, SA *p_sa, int len, int nsecs){
    sigfun *old_fun;
    old_fun = Signal(SIGALRM, SIG_DFL);

    if(alarm(nsecs) != 0){
        err_msg("connect_timeo: set alarm done, covering last set\n");
    }
    int res;
    if((res = connect(fd, p_sa, len)) < 0){
        if(errno == EINTR)
          errno = ETIMEDOUT;
    }
    alarm(0);
    Signal(SIGALRM, old_fun);

    return res;
}

ssize_t Sendto(int sockfd, const void *buff, size_t nbytes, int flags, const struct sockaddr *to, socklen_t addrlen){
    ssize_t res;
    if((res = sendto(sockfd, buff, nbytes, flags, to, addrlen)) < 0){
        err_sys("Sendto error");
    }

    return res;
}

ssize_t Recvfrom(int sockfd, void *buff, size_t nbytes, int flags, struct sockaddr *from, socklen_t *addrlen){
    ssize_t res;
    if((res = recvfrom(sockfd, buff, nbytes, flags, from, addrlen)) < 0){
        err_sys("Recvfrom error");
    }

    return res;
}


void echo_server_udp_universal(int listen_fd){
    char receive_line[MAX_LINE], ip_buf[IP_LEN];
    struct sockaddr_storage sas;
    socklen_t sl;
    ssize_t ss;
    while(1){
        sl = sizeof(sas);
        ss = Recvfrom(listen_fd, receive_line, MAX_LINE, 0, (SA *)&sas, &sl);
        ss = Sendto(listen_fd, receive_line, ss, 0, (SA *)&sas, sl);
    }
}

void echo_server_udp(int listen_fd){
    char receive_line[MAX_LINE], ip_buf[IP_LEN];
    struct sockaddr_in sai;
    socklen_t sl;
    ssize_t ss;
    while(1){
        sl = sizeof(sai);
        ss = Recvfrom(listen_fd, receive_line, MAX_LINE, 0, (SA *)&sai, &sl);
        printf("==========receive query from ip: %s, port: %d, bytes: %ld\n", inet_ntop(AF_INET, &(sai.sin_addr), ip_buf, IP_LEN), ntohs(sai.sin_port), ss);
        ss = Sendto(listen_fd, receive_line, ss, 0, (SA *)&sai, sl);
        printf("==========send back done, writing %ld bytes\n", ss);
    }
}

void flow_client_udp(int sock_fd, const SA *p_sa, socklen_t sl_ori){
    char receive_line[MAX_LINE];
    ssize_t ss = 0;
    for(int i = 0;i < 100000;i++){
        ss += Sendto(sock_fd, receive_line, MAX_LINE, 0, NULL, 0);
    }
    printf("done, sending %ld bytes\n", ss);
}

static ssize_t flow_ss = 0;

void sigint_handler(int sig){
    printf("receiving %ld in total, exit\n", flow_ss);
    exit(0);
}

void flow_server_udp(int listen_fd){
    char receive_line[MAX_LINE];
    int first_flag = 1;
    Signal(SIGINT, sigint_handler);
    while(1){
        flow_ss += Recvfrom(listen_fd, receive_line, MAX_LINE, 0, NULL, 0);
        if(first_flag){
            sleep(3);
            first_flag = 0;
        }
    }
}

void echo_client_udp(int sock_fd, const SA *p_sa, socklen_t sl_ori){
    char receive_line[MAX_LINE], ip_buf[IP_LEN];
    ssize_t ss;
    SA *p_sa_temp = (SA *)malloc(sl_ori);
    socklen_t sl_temp;

    while(fgets(receive_line, MAX_LINE, stdin)){
        ss = Sendto(sock_fd, receive_line, strlen(receive_line), 0, p_sa, sl_ori);
        //ss = Sendto(sock_fd, receive_line, strlen(receive_line), 0, NULL, 0);
        sl_temp = sl_ori;
        ss = Recvfrom(sock_fd, receive_line, MAX_LINE, 0, p_sa_temp, &sl_temp);
        if(p_sa_temp->sa_family == AF_INET){
            printf("==========receive answer from ip: %s, port: %d, bytes: %ld\n", inet_ntop(AF_INET, &(((struct sockaddr_in *)p_sa_temp)->sin_addr), ip_buf, IP_LEN), ntohs(((struct sockaddr_in *)p_sa_temp)->sin_port), ss);
        }
        if(sl_temp != sl_ori || memcmp(p_sa, p_sa_temp, sl_ori) != 0){
            printf("receive answer from wrong server\n");
        }
        receive_line[ss] = 0;
        Fputs(receive_line, stdout);
    }
}

struct addrinfo *host_serv(const char *host_name, const char *service_name, int family, int sock_type){
    struct addrinfo *p_ai, ai;
    bzero(&ai, sizeof(ai));
    ai.ai_family = family;
    ai.ai_flags = AI_CANONNAME;
    ai.ai_socktype = sock_type;

    int res;
    if((res = getaddrinfo(host_name, service_name, &ai, &p_ai)) != 0){
        printf("getaddrinfo error: %s\n", gai_strerror(res));
        err_sys("host_serv wrong");
    }

    return p_ai;
}

int tcp_connect(const char *host_name, const char *service_name){
    struct addrinfo ai, *p_ai, *p_ai_ori;
    int sock_fd;

    bzero(&ai, sizeof(ai));
    ai.ai_family = AF_UNSPEC;
    ai.ai_socktype = SOCK_STREAM;
    ai.ai_flags = AI_CANONNAME;

    int res;
    if((res = getaddrinfo(host_name, service_name, &ai, &p_ai)) != 0){
        printf("getaddrinfo error: %s\n", gai_strerror(res));
        err_sys("host_serv wrong");
    }
    p_ai_ori = p_ai;

    while(p_ai != NULL){
        sock_fd = socket(p_ai->ai_family, p_ai->ai_socktype, p_ai->ai_protocol);
        if(connect(sock_fd, p_ai->ai_addr, p_ai->ai_addrlen) < 0){
            perror("failed to connect one\n");
        }else{
            break;
        }
        p_ai = p_ai->ai_next;
        Close(sock_fd);
    }

    freeaddrinfo(p_ai_ori);
    if(p_ai == NULL)
        err_sys("all failed\n");
    return sock_fd;
}

int tcp_listen(const char *host_name, const char *service_name, socklen_t *p_sl){
    struct addrinfo ai, *p_ai, *p_ai_ori;
    bzero(&ai, sizeof(ai));
    ai.ai_flags = AI_PASSIVE;
    ai.ai_family = AF_INET;
    ai.ai_socktype = SOCK_STREAM;
    
    int listen_fd = 0;
    int res, is_reuse;
    if((res = getaddrinfo(host_name, service_name, &ai, &p_ai)) != 0){
        printf("getaddrinfo error: %s\n", gai_strerror(res));
        err_sys("tcp_listen wrong");
    }

    p_ai_ori = p_ai;
    while(p_ai != NULL){
        if(listen_fd)
            Close(listen_fd);
        listen_fd = Socket(p_ai->ai_family, p_ai->ai_socktype, p_ai->ai_protocol);
        is_reuse = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &is_reuse, sizeof(is_reuse));
        if((res = bind(listen_fd, p_ai->ai_addr, p_ai->ai_addrlen)) < 0){
            perror("failed to bind one\n");
            p_ai = p_ai->ai_next;
            continue;
        }
        if((res = listen(listen_fd, LISTENQL)) < 0){
            perror("failed to listen one\n");
            p_ai = p_ai->ai_next;
            continue;
        }
        break;
    }

    if(p_ai == NULL){
        err_sys("all failed");
    }
    if(p_sl != NULL)
        *p_sl = p_ai->ai_addrlen;
    freeaddrinfo(p_ai_ori);

    return listen_fd;
}

int udp_client(const char *host_name, const char *service_name, SA **p_sa, socklen_t *sl){
    struct addrinfo hints, *p_res, *p_res_ori;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    int res, sock_fd;
    if((res = getaddrinfo(host_name, service_name, &hints, &p_res)) != 0){
        printf("getaddrinfo error: %s\n", gai_strerror(res));
        err_sys("udp_client wrong");
    }
    p_res_ori = p_res;
    while(p_res != NULL){
        if((sock_fd = socket(p_res->ai_family, p_res->ai_socktype, p_res->ai_protocol)) > 0){
            break;
        }else{
            perror("failed to socket one\n");
        }
        p_res = p_res->ai_next;
    }

    if(p_res == NULL){
        err_sys("all failed");
    }
    *p_sa = (SA *)malloc(p_res->ai_addrlen);
    memcpy(*p_sa, p_res->ai_addr, p_res->ai_addrlen);
    *sl = p_res->ai_addrlen;

    freeaddrinfo(p_res_ori);

    return sock_fd;
}

int udp_connect(const char *host_name, const char *service_name){
    struct addrinfo ai, *p_ai, *p_ai_ori;
    int sock_fd;

    bzero(&ai, sizeof(ai));
    ai.ai_family = AF_UNSPEC;
    ai.ai_socktype = SOCK_DGRAM;
    ai.ai_flags = AI_CANONNAME;

    int res;
    if((res = getaddrinfo(host_name, service_name, &ai, &p_ai)) != 0){
        printf("getaddrinfo error: %s\n", gai_strerror(res));
        err_sys("udp_connect wrong");
    }
    p_ai_ori = p_ai;

    while(p_ai != NULL){
        sock_fd = socket(p_ai->ai_family, p_ai->ai_socktype, p_ai->ai_protocol);
        if(connect(sock_fd, p_ai->ai_addr, p_ai->ai_addrlen) < 0){
            perror("failed to connect one\n");
        }else{
            break;
        }
        p_ai = p_ai->ai_next;
        Close(sock_fd);
    }

    freeaddrinfo(p_ai_ori);
    if(p_ai == NULL)
        err_sys("all failed\n");
    return sock_fd;
}

int udp_server(const char *host_name, const char *service_name, socklen_t *p_sl){
    struct addrinfo hint, *p_res, *p_res_ori;
    bzero(&hint, sizeof(hint));
    hint.ai_flags = AI_PASSIVE;
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_DGRAM;
    int res, sock_fd = 0;

    if((res = getaddrinfo(host_name, service_name, &hint, &p_res)) < 0){
        printf("getaddrinfo error: %s\n", gai_strerror(res));
        err_sys("udp_server wrong");
    }
    printf("service_name: %s\n", service_name);
    p_res_ori = p_res;
    while(p_res != NULL){
        sock_fd = socket(p_res->ai_family, p_res->ai_socktype, p_res->ai_protocol);
        if(sock_fd < 0){
            perror("failed to socket one\n");
        }else{
            printf("port: %d\n", ntohs(((struct sockaddr_in *)(p_res->ai_addr))->sin_port));
            if(bind(sock_fd, p_res->ai_addr, p_res->ai_addrlen) < 0){
                perror("failed to connect one\n");
                Close(sock_fd);
            }else{
                break;
            }
        }
        p_res = p_res->ai_next;
    }

    if(p_res == NULL){
        err_sys("all failed\n");
    }
    if(p_sl != NULL)
        *p_sl = p_res->ai_addrlen;

    freeaddrinfo(p_res_ori);

    return sock_fd;
}

// 两次fork的最终目的是使的当前进程是一个会话和进程组中的唯一进程，并且不是会话组和进程组的首进程
void daemon_init(const char *proc_name, int facility){
    //第一次fork，使子进程变成孤儿从而被init收养，也使得子进程不可能是会话组或者进程组的首进程
    pid_t pid;
    pid = fork();
    if(pid > 0){
        exit(0);
    }else if(pid < 0){
        err_sys("In daemon_init, fork failed");
    }

    //setsid，保证第一次子进程是进程组和会话首进程
    if(setsid() < 0){
        err_sys("In daemon_init, setsid failed");
    }

    // 必须先忽略sighup，否则第二次fork会收到sighup子进程失败，第一次不用设置是因为正常情况下的会话组首进程是login
    Signal(SIGHUP, SIG_IGN);

    // 第二次达到deamon的目的
    pid = fork();
    if(pid > 0){
        exit(0);
    }else if(pid < 0){
        err_sys("In daemon_init, second fork failed");
    }

    // 错误日志输出到syslog
    err_sys_syslog = 1;

    // 改变工作目录，目的防止卡文件系统
    chdir("/");

    // 关闭所有文件描述符号，本函数是一个通用的守护进程生成函数，对于各种异常应该足够的健壮
    int i;
    for(i = 0;i <= MAX_FD;i++)
      close(i);

    // 打开标准输入、输出、错误到/dev/null，防止0 1 2描述符被不经意间用到
    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);

    // 最后输出到syslog配置
    openlog(proc_name, LOG_PID, facility);
}

void echo_client_udp_timeo(int sock_fd, const SA *p_sa, socklen_t sl_ori){
    char receive_line[MAX_LINE], ip_buf[IP_LEN];
    ssize_t ss;
    SA *p_sa_temp = (SA *)malloc(sl_ori);
    socklen_t sl_temp;

    while(fgets(receive_line, MAX_LINE, stdin)){
        ss = Sendto(sock_fd, receive_line, strlen(receive_line), 0, p_sa, sl_ori);
        //ss = Sendto(sock_fd, receive_line, strlen(receive_line), 0, NULL, 0);
        sl_temp = sl_ori;
        alarm(5);
        ss = recvfrom(sock_fd, receive_line, MAX_LINE, 0, p_sa_temp, &sl_temp);
        if(ss < 0){
            if(errno == EINTR){
                err_msg("echo_client_udp_timeo: recvfrom time out, continue\n");
                alarm(0);
                continue;
            }else{
                err_sys("echo_client_udp_timeo error");
            }
        }
        alarm(0);
        if(p_sa_temp->sa_family == AF_INET){
            printf("==========receive answer from ip: %s, port: %d, bytes: %ld\n", inet_ntop(AF_INET, &(((struct sockaddr_in *)p_sa_temp)->sin_addr), ip_buf, IP_LEN), ntohs(((struct sockaddr_in *)p_sa_temp)->sin_port), ss);
        }
        if(sl_temp != sl_ori || memcmp(p_sa, p_sa_temp, sl_ori) != 0){
            printf("receive answer from wrong server\n");
        }
        receive_line[ss] = 0;
        Fputs(receive_line, stdout);
    }
}

void echo_client_udp_timeo_select(int sock_fd, const SA *p_sa, socklen_t sl_ori){
    char receive_line[MAX_LINE], ip_buf[IP_LEN];
    ssize_t ss;
    SA *p_sa_temp = (SA *)malloc(sl_ori);
    socklen_t sl_temp;
    int res;

    while(fgets(receive_line, MAX_LINE, stdin)){
        ss = Sendto(sock_fd, receive_line, strlen(receive_line), 0, p_sa, sl_ori);
        //ss = Sendto(sock_fd, receive_line, strlen(receive_line), 0, NULL, 0);
        sl_temp = sl_ori;
        res = fd_readable_timeo(sock_fd, 5);
        if(res == 0){
            err_msg("echo_client_udp_timeo_select: read timeout\n");
            continue;
        }else if(res < 0){
            err_sys("echo_client_udp_timeo_select error");
        }
        ss = recvfrom(sock_fd, receive_line, MAX_LINE, 0, p_sa_temp, &sl_temp);
        if(p_sa_temp->sa_family == AF_INET){
            printf("==========receive answer from ip: %s, port: %d, bytes: %ld\n", inet_ntop(AF_INET, &(((struct sockaddr_in *)p_sa_temp)->sin_addr), ip_buf, IP_LEN), ntohs(((struct sockaddr_in *)p_sa_temp)->sin_port), ss);
        }
        if(sl_temp != sl_ori || memcmp(p_sa, p_sa_temp, sl_ori) != 0){
            printf("receive answer from wrong server\n");
        }
        receive_line[ss] = 0;
        Fputs(receive_line, stdout);
    }
}

void echo_client_udp_timeo_setsocketopt(int sock_fd, const SA *p_sa, socklen_t sl_ori){
    char receive_line[MAX_LINE], ip_buf[IP_LEN];
    ssize_t ss;
    SA *p_sa_temp = (SA *)malloc(sl_ori);
    socklen_t sl_temp;
    int res;
    struct timeval tv;
    bzero(&tv, sizeof(tv));
    tv.tv_sec = 5;
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while(fgets(receive_line, MAX_LINE, stdin)){
        ss = Sendto(sock_fd, receive_line, strlen(receive_line), 0, p_sa, sl_ori);
        //ss = Sendto(sock_fd, receive_line, strlen(receive_line), 0, NULL, 0);
        sl_temp = sl_ori;
        ss = recvfrom(sock_fd, receive_line, MAX_LINE, 0, p_sa_temp, &sl_temp);
        if(ss < 0){
            if(errno == EWOULDBLOCK){
                err_msg("echo_client_udp_timeo_setsocketopt: recvfrom time out");
                continue;
            }else{
                err_sys("echo_client_udp_timeo_setsocketopt error");
            }
        }
        if(p_sa_temp->sa_family == AF_INET){
            printf("==========receive answer from ip: %s, port: %d, bytes: %ld\n", inet_ntop(AF_INET, &(((struct sockaddr_in *)p_sa_temp)->sin_addr), ip_buf, IP_LEN), ntohs(((struct sockaddr_in *)p_sa_temp)->sin_port), ss);
        }
        if(sl_temp != sl_ori || memcmp(p_sa, p_sa_temp, sl_ori) != 0){
            printf("receive answer from wrong server\n");
        }
        receive_line[ss] = 0;
        Fputs(receive_line, stdout);
    }
}

FILE *Fdopen(int fd, const char * mode){
    FILE *pf;
    if((pf = fdopen(fd, mode)) == NULL){
        err_sys("Fdopen error");
    }

    return pf;
}

void str_echo_server_file(int sockfd){
    FILE *fin, *fout;
    char buf[MAX_LINE];

    fin = Fdopen(sockfd, "r");
    fout = Fdopen(sockfd, "w");
    printf("ininininininin\n");

    while(fgets(buf, MAX_LINE, fin) != NULL){
        fputs(buf, fout);
        fflush(fout);
    }
}

int Kqueue(){
    int kid;
    if((kid = kqueue()) < 0){
        err_sys("kqueue error");
    }

    return kid;
}

int Kevent(int kq, const struct kevent *changelist, int nchanges, struct kevent *eventlist, int nevents, const struct timespec *timeout){
    int res;
    if((res = kevent(kq, changelist, nchanges, eventlist, nevents, timeout)) < 0)
        err_sys("Kevent error");
    return res;
}

# define KQUEUE_SIZE 2

void str_echo_client_kqueue(FILE *fin, int connect_fd){
    int kfd, is_file = 0, res, i, n, is_done = 0, peek_n, ioctl_n;
    struct kevent kes[KQUEUE_SIZE];
    char buf[MAX_LINE];
    struct timespec ts;
    struct stat st;

    fstat(fileno(fin), &st);
    if((st.st_mode & S_IFMT) == S_IFREG){
        is_file = 1;
    }

    EV_SET(&(kes[0]), fileno(fin), EVFILT_READ, EV_ADD, 0, 0, NULL);
    EV_SET(&(kes[1]), connect_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);

    kfd = Kqueue();
    ts.tv_sec = ts.tv_nsec = 0;
    Kevent(kfd, kes, KQUEUE_SIZE, NULL, 0, &ts);

    while(1){
        res = Kevent(kfd, NULL, 0, kes, KQUEUE_SIZE, NULL);
        for(i = 0;i < res;i++){
            if(kes[i].ident == fileno(fin)){
                n = Read(fileno(fin), buf, MAX_LINE);
                if(n > 0){
                    Write(connect_fd, buf, n);
                }else if(n == 0 || (is_file && n == kes[i].data)){
                    shutdown(connect_fd, SHUT_WR);
                    kes[i].flags = EV_DELETE;
                    kevent(kfd, &(kes[i]), 1, NULL, 0, &ts);
                    is_done = 1;
                }
            }
            if(kes[i].ident == connect_fd){
                peek_n = recv(connect_fd, buf, MAX_LINE, MSG_PEEK);
                ioctl(connect_fd, FIONREAD, &ioctl_n);
                printf("peek len: %d, ioctl len: %d\n", peek_n, ioctl_n);
                
                n = Read(connect_fd, buf, MAX_LINE);
                if(n == 0){
                    if(is_done)
                        err_msg("finished\n");
                    else
                        err_msg("server died prematurely\n");
                    return;
                }
                Write(fileno(stdout), buf, n);
            }
        }
    }
}

int Socketpair(int family, int type, int protocal, int sockfd[2]){
    int res;
    if((res = socketpair(family, type, protocal, sockfd)) < 0)
        err_sys("Socketpair error");

    return res;
}

int write_open_fd(int unfd, int filefd, char *buf, size_t buf_len){
    struct msghdr msg;
    struct iovec iov[1];
    union{
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    }control_un;
    control_un.cm.cmsg_len = CMSG_SPACE(sizeof(int));
    control_un.cm.cmsg_level = SOL_SOCKET;;
    control_un.cm.cmsg_type = SCM_RIGHTS;;
    *((int *)CMSG_DATA(&(control_un.cm))) = filefd;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);

    iov[0].iov_base = buf;
    iov[0].iov_len = buf_len;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    return sendmsg(unfd, &msg, 0);
}

int read_open_fd(int un_fd, char *c, int nbytes, int *fd){
    struct msghdr msg;
    struct iovec iov[1];
    int n;

#ifndef HAVE_MSGHDR_MSG_CONTROL
    union{
        struct cmsghdr cm;
        char control[CMSG_SPACE(sizeof(int))];
    }control_un;
    struct cmsghdr *cmptr;
    msg.msg_control = control_un.control;
    msg.msg_controllen = sizeof(control_un.control);
#else
    int newfd;
    msg.msg_accrights = (caddr_t)&newfd;
    msg.msg_accrightslen = sizeof(int);
#endif

    msg.msg_name = NULL;
    msg.msg_namelen = 0;
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

    iov[0].iov_base = c;
    iov[0].iov_len = nbytes;

    if((n = recvmsg(un_fd, &msg, 0)) < 0)
        return n;
#ifndef HAVE_MSGHDR_MSG_CONTROL
    if((cmptr = CMSG_FIRSTHDR(&msg)) != NULL && cmptr->cmsg_len == CMSG_LEN(sizeof(int))){
        if(cmptr->cmsg_level != SOL_SOCKET){
            err_sys("read_open_fd, cmsg_level != SOL_SOCKET");
        }
        if(cmptr->cmsg_type != SCM_RIGHTS){
            err_sys("read_open_fd, cmsg_type != SCM_RIGHTS");
        }
        *fd = *((int *)(CMSG_DATA(cmptr)));
    }else{
        *fd = -1;
    }
#else
    if(msg.msg_accrightslen == sizeof(int))
        *fd = newfd;
    else
      *fd = -1;
#endif
    return n;
}

int myopen(char *file_path, int mode){
    int fds[2], res, status, code, fd;
    Socketpair(AF_LOCAL, SOCK_STREAM, 0, fds);
    char fd1_buf[20], mode_buf[20], c;

    res = fork();
    if(res < 0){
        err_sys("myopen fork error");
    }
    if(res == 0){
        //子进程
        snprintf(fd1_buf, sizeof(fd1_buf), "%d", fds[1]);
        snprintf(mode_buf, sizeof(mode_buf), "%d", mode);
        close(fds[0]);
        execl("./openfile", "openfile", file_path, mode_buf, fd1_buf, (char *)NULL);
        exit(-9999);
    }

    // 父进程
    close(fds[1]);
    waitpid(res, &status, 0);
    if(!WIFEXITED(status)){
        err_sys("child exit prematurely");
    }
    code = WEXITSTATUS(status);
    if(code != 0){
        errno = code;
        return -1;
    }

    // 读取描述符逻辑
    read_open_fd(fds[0], &c, 1, &fd);
    close(fds[0]);
    return fd;
}

static int is_sigchld_caught = 0;
void sigusr2_handler(int signo){
    printf("in sigchld_handler\n");
    is_sigchld_caught = 1;
}

void echo_client_process(int connect_fd){
    int in_fd = fileno(stdin), out_fd = fileno(stdout), child_pid, n;
    char buf[MAX_LINE];
    child_pid = fork();
    if(child_pid < 0)
        err_sys("echo_client_process: fork error");
    if(child_pid == 0){
        while((n = Read(connect_fd, buf, MAX_LINE)) > 0)
            Write(out_fd, buf, n);
        close(connect_fd);
        printf("son exited\n");
        kill(getppid(), SIGCHLD);
        exit(0);
    }

    Signal(SIGCHLD, sigusr2_handler);
    while(1){
        n = read(in_fd, buf, MAX_LINE);
        if(n == 0){
            break;
        }else if(n < 0){
            if(errno == EINTR){
                if(is_sigchld_caught)
                    err_sys("server died prematurely");
                continue;
            }
            err_sys("echo_client_process, read error");
        }
        Write(connect_fd, buf, n);
    }
    shutdown(connect_fd, SHUT_WR);
    pause();
    printf("father exited\n");
}

void echo_client_unblocked(int connect_fd){
    fd_set read_set, write_set;
    int in_fd = fileno(stdin), out_fd = fileno(stdout), fd_max, is_in_end = 0, select_n, fcntl_res, n;
    // 以网路为中心命名的read\write，从网络读入read，预备写入网络write
    char read_buf[READ_BUF_LEN], write_buf[WRITE_BUF_LEN];
    char *pr_begin, *pr_end, *pw_begin, *pw_end;
    pr_begin = pr_end = read_buf;
    pw_begin = pw_end = write_buf;

    fd_max = max(max(connect_fd, in_fd), out_fd) + 1;
    fcntl_res = fcntl(connect_fd, F_GETFL, 0);
    fcntl(connect_fd, F_SETFL, fcntl_res | O_NONBLOCK);
    fcntl_res = fcntl(in_fd, F_GETFL, 0);
    fcntl(in_fd, F_SETFL, fcntl_res | O_NONBLOCK);
    fcntl_res = fcntl(out_fd, F_GETFL, 0);
    fcntl(out_fd, F_SETFL, fcntl_res | O_NONBLOCK);

    while(1){
        FD_ZERO(&read_set);
        FD_ZERO(&write_set);

        if(pr_end < (read_buf + READ_BUF_LEN - 1))
            FD_SET(connect_fd, &read_set);
        if(pw_begin < pw_end)
            FD_SET(connect_fd, &write_set);
        if(pw_end < (write_buf + WRITE_BUF_LEN - 1) && is_in_end == 0)
            FD_SET(in_fd, &read_set);
        if(pr_begin < pr_end)
            FD_SET(out_fd, &write_set);

        select_n = select(fd_max, &read_set, &write_set, NULL, NULL);
        if(select_n < 0){
            if(errno == EINTR)
                continue;
            err_sys("echo_client_unblocked, select error");
        }
        if(FD_ISSET(in_fd, &read_set)){
            n = read(in_fd, pw_end, (write_buf + WRITE_BUF_LEN - 1 - pw_end));
            if(n < 0){
                if(errno != EINTR && errno != EWOULDBLOCK)
                    err_sys("read error");
            }else{
                printf("receive from in_fd, %d\n", n);
                if(n == 0)
                    is_in_end = 1;
                pw_end += n;
                if(pw_end >= (write_buf + WRITE_BUF_LEN - 1))
                    is_in_end = 1;
                FD_SET(connect_fd, &write_set);
            }
        }
        if(FD_ISSET(connect_fd, &read_set)){
            n = read(connect_fd, pr_end, (read_buf + READ_BUF_LEN - 1 - pr_end));
            printf("receive from server, %d\n", n);
            if(n < 0){
                if(errno != EINTR && errno != EWOULDBLOCK)
                    err_sys("read error");
            }else if(n == 0){
                if(is_in_end == 0)
                    err_sys("echo_client_unblocked: server terminated prematurely");
                Close(out_fd);
                return;
            }else{
                pr_end += n;
                FD_SET(out_fd, &write_set);
            }
        }
        if(FD_ISSET(connect_fd, &write_set)){
            if(pw_begin < pw_end){
                n = write(connect_fd, pw_begin, pw_end - pw_begin);
                if(n < 0){
                    if(errno != EINTR && errno != EWOULDBLOCK){
                        err_sys("write error");
                    }
                }else if(n > 0){
                    pw_begin += n;
                    if((is_in_end && pw_begin == pw_end) || (pw_begin >= (write_buf + WRITE_BUF_LEN - 1))){
                        shutdown(connect_fd, SHUT_WR);
                    }
                }
            }
            if(pw_begin == pw_end && is_in_end)
                shutdown(connect_fd, SHUT_WR);
        }
        if(FD_ISSET(out_fd, &write_set)){
            n = write(out_fd, pr_begin, pr_end - pr_begin);
            if(n < 0){
                if(errno != EINTR && errno != EWOULDBLOCK)
                    err_sys("write error");
            }else if(n > 0){
                pr_begin += n;
            }
        }
    }
}

int connect_nonb(int fd, const SA *p_sa, socklen_t len, int nsec){
    int old_flag, res, error;
    fd_set wset, rset;
    struct timeval tv, *p_tv;
    old_flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, old_flag | O_NONBLOCK);

    res = connect(fd, p_sa, len);
    if(res < 0){
        if(errno != EWOULDBLOCK)
            return res;
    }

    if(res == 0)
        return res;

    //yes, according to the authur, we can do something we want here, but its not simple to be realized by code, maybe a pointer of a function
    FD_ZERO(&wset);
    FD_SET(fd, &wset);
    rset = wset;
    if(nsec <= 0){
        p_tv = NULL;
    }else{
        tv.tv_sec = nsec;
        tv.tv_usec = 0;
        p_tv = &tv;
    }
    res = Select(fd + 1, &rset, &wset, NULL, p_tv);
    if(res == 0){
        Close(fd);
        errno = ETIMEDOUT;
        return -1;
    }
    if(FD_ISSET(fd, &wset) || FD_ISSET(fd, &rset)){
        len = sizeof(error);
        res = getsockopt(fd, SOL_SOCKET, SO_ERROR, &error, &len);
        if(res < 0)
            return res;
    }else{
        err_sys("connect_nonb, suprising");
    }

    if(error){
        errno = error;
        Close(fd);
        return -1;
    }

    return 0;
}
