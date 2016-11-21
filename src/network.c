#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>

#include "network.h"
#include "event.h"
#include "config.h"
#include "handler.h"

int an_make_nonblock(int fd);

int listenfd;

int an_make_listen_fd(char *addr, int port) {
    struct sockaddr_in sin;
    int sockfd, res;
    sockfd = socket(AF_INET, SOCK_DGRAM|SOCK_NONBLOCK|SOCK_CLOEXEC, IPPROTO_UDP);
    if(sockfd == -1) {
        printf("Create udp socket error: %s\n", strerror(errno));
        exit(errno);
    }
    an_make_nonblock(sockfd);
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = inet_addr(addr);
    
    res = bind(sockfd, (struct sockaddr *) &sin, sizeof(sin));
    if(res == -1) {
        printf("Bind address %s:%d error: %s\n", addr, port, strerror(errno));
        exit(errno);
    }
    return sockfd;
}

void an_make_connection() {
    int i = craft_cnt, sockfd, res;
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    event *p = NULL;
    while(i) {
        sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if(sockfd == -1) {
            printf("Create socket error: %s\n", strerror(errno));
            exit(errno);
        }
        sin.sin_addr.s_addr = crafts[i].addr;
        res = connect(sockfd, (struct sockaddr *) &sin, sizeof(sin));
        if(res == -1){
            printf("Create connection to %s:%d error: %s\n", inet_ntoa(sin.sin_addr), port, strerror(errno));
            crafts[i].fd = -1;
        } else {
            crafts[i].fd = sockfd;
            an_make_nonblock(sockfd);
            p = (event *)malloc(sizeof(event));
            p->fd = sockfd;
            p->craft = crafts[i];
            p->handler = recv_handler;
            an_add_event(p, EPOLLIN | EPOLLET);
        }
        --i;
    }
}

int an_make_nonblock(int fd) {
    if (fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0)|O_NONBLOCK) == -1) {
        return errno;
    }
    return 0;
}

int an_start_server(char *addr, int port){
    event *p;
    an_init_event_loop(1000);
    an_make_connection();
    listenfd = an_make_listen_fd(addr, port);
    p = (event *)malloc(sizeof(event));
    p->fd = listenfd;
    p->handler = recv_handler;
    an_add_event(p, EPOLLIN|EPOLLET);
    an_start_event_loop();
    return 0;
}
