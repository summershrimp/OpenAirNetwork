#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <stdint.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include "handler.h"
#include "event.h"
int recv_handler(struct epoll_event e){
    uint8_t buf[1500];
    int len, addr_len;
    struct sockaddr_in from;
    event *p;
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
    sendto(p->fd, buf, len, 0, (struct sockaddr *) &from, sizeof(from));
    return 0;    
}
