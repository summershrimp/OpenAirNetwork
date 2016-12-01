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
using namespace std;
int an_make_nonblock(int fd);
int recv_handler(struct epoll_event e);
inline int handle_ack(sockaddr_in addr, uint8_t *buf, int size);
inline int send_ack(sockaddr_in addr, uint8_t *buf, int size);

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

int an_start_server(char *addr, int port){
    event *p;
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
        arq_t wait_d;
        wait_d.size = len;
        memcpy(wait_d.data, send_buf, len);
        crafts[i].wait_queue.push(wait_d);
    }
    return 0;
}


int recv_handler(struct epoll_event e){
    uint8_t buf[1500];
    int len, fd;
    socklen_t addr_len;
    struct sockaddr_in from;
    event *p;
    uavmp_t *up;
    printf("Handling %d... \n", e.data.fd);
    if(!(e.events & EPOLLIN)) {
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
    if(up->type & 0x80) {
        return handle_ack(from, buf, len);
    }
    if(protos[up->type].id == up->type) {
        fd = protos[up->type].fifo_fd;
        write(fd, buf + sizeof(uavmp_t), len - sizeof(uavmp_t));
    }
    send_ack(from, buf, len);
    if(is_master) {
       an_broadcast_msg(up->type, up->code, buf + sizeof(uavmp_t), len - sizeof(uavmp_t)); 
    }
    return 0;
}

inline int handle_ack(sockaddr_in from, uint8_t *buf, int size) {
	uavmp_t *mp = (uavmp_t *) buf;
	if(size < (int) sizeof(uavmp_t)) {
		return -1;
    } 
    if(!addr_map.count(from.sin_addr.s_addr)) {
        printf("No peers found: %s\n", inet_ntoa(from.sin_addr));
        return -1;
    }
    craft_addr &craft = *addr_map[from.sin_addr.s_addr]; 
    if(mp->seq == craft.send_seq + 1) {
        craft.send_seq += 1;
        craft.wait_queue.pop();
    }
    return 0;
}
inline int send_ack(sockaddr_in to, uint8_t *buf, int size){
    uavmp_t *mp = (uavmp_t *)buf;
    if(size < (int) sizeof(uavmp_t)) {
        return -1; 
    }
    if(!addr_map.count(to.sin_addr.s_addr)) {
        printf("No peers found: %s\n", inet_ntoa(to.sin_addr));
        return -1;
    } 
    craft_addr &craft = *addr_map[to.sin_addr.s_addr];
    bool need_send = false;
    if(mp->seq <= craft.recv_seq) {
        need_send = true;
    }
    if(mp->seq == craft.recv_seq + 1) {
        craft.recv_seq += 1;
        need_send = true;
    }
    if(!need_send) {
        return 0;
    }

    mp->type &= 0x80;
    int res = sendto(listenfd, buf, sizeof(uavmp_t), 0, (sockaddr *) &to, sizeof(to));
    if(res == -1) {
        printf("Send ack packet failed: %s", strerror(errno));
        return errno;
    }
    return 0;
}
