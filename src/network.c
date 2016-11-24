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
#include "util.h"
#include "subprocess.h"
#include "uavmp/uavmp.h"

int an_make_nonblock(int fd);
int recv_handler(struct epoll_event e);

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

//Deprecated
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
            p->ptr = (void*)&crafts[i];
            p->handler = recv_handler;
            an_add_event(p, EPOLLIN | EPOLLET);
        }
        --i;
    }
}

int an_start_server(char *addr, int port){
    event *p;
    //an_make_connection();
    listenfd = an_make_listen_fd(addr, port);
    p = (event *)malloc(sizeof(event));
    p->fd = listenfd;
    p->handler = recv_handler;
    an_add_event(p, EPOLLIN|EPOLLET);
    an_start_event_loop();
    return 0;
}

int an_make_frame(uint8_t type, int code, craft_addr c, uint8_t *src, unsigned int src_size, 
        uint8_t *dst, unsigned int dst_size) {
    int i = 0;
    uint8_t *bp;
    if(sizeof(uavmp_t) + src_size > (unsigned int)dst_size) {
        return -1;
    }
    uavmp_t *up = (uavmp_t *)dst;
	up->version = 0x1;
	up->rsv = 0x0;
	up->type = type;
	up->code = code;
	up->crc8 = 0x0;
	up->idt = c.addr;
    up->seq = htonl(c.send_seq);
    i += sizeof(uavmp_t);
    bp = dst + i;
    memcpy(bp, src, src_size);
    return src_size + sizeof(uavmp_t);
}

int an_broadcast_msg(uint8_t type, uint8_t code, uint8_t *src, int size) {
    uint8_t send_buf[1450];
    struct sockaddr_in peer;
    int i, len;
    peer.sin_port = htons(port);
    peer.sin_family = AF_INET;
    for(i=0; i<craft_cnt; ++i) {
        peer.sin_addr.s_addr = crafts[i].addr;
        len = an_make_frame(type, code, crafts[i], src, size, send_buf, sizeof(send_buf));
        if(len == -1) {
            printf("Package too large\n");
            return -1;
        }
        printf("Send packet to %s:%d length: %d\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port), len);
        sendto(listenfd, send_buf, len, 0, (struct sockaddr *)&peer, sizeof(peer));
    }
    return 0;
}


int recv_handler(struct epoll_event e){
    uint8_t buf[1500];
    int len, addr_len, i, fd;
    struct sockaddr_in from;
    event *p;
    uavmp_t *up;
    printf("Handling %d... \n", e.data.fd);
    if(! e.events & EPOLLIN) {
        return -1;
    }
    p = (event*)e.data.ptr;
    printf("event fd: %d\n", p->fd);
    len = recvfrom(p->fd, buf, sizeof(buf), 0, (struct sockaddr *) &from, &addr_len);
    printf("Receive from %s:%d len: %d\n", inet_ntoa(from.sin_addr) ,ntohs(from.sin_port), len);
    if(len == 0) {
        printf("No data comes.\n");
        return -1;
    }

    if(len == -1) {
        printf("Read error: %s\n", strerror(errno));
        return errno;
    }
    up = (uavmp_t *)buf;
    fd = protos[up->type].fifo_fd;
    write(fd, buf + sizeof(uavmp_t), len - sizeof(uavmp_t));
    //TODO: add ack packet.  sendto(p->fd, buf, len, 0, (struct sockaddr *) &from, sizeof(from));
    return 0;
}
