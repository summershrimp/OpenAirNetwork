#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "subprocess.h"
#include "config.h"
#include "util.h"
#include "event.h"
#include "network.h"
#include "uavmp/uavmp.h"

event events[128];

int fifo_handler(struct epoll_event e);

int an_create_and_open_fifo(char *dir, int mode) {
    struct stat file_stat;
    int fd;
    if(stat(dir, &file_stat) == -1) {
        if(errno == ENOENT) {
            mkfifo(dir, 0660);
            goto LAB_OPEN;
        } else {
            printf("Stat file %s error: %s\n", dir, strerror(errno));
            return -1;
        }
    }
    if(!(file_stat.st_mode & S_IFIFO)) {
        printf("File %s already exists and not a fifo file.\n", dir);
        return -1;
    }
LAB_OPEN:
    fd = open(dir, mode);
    if(fd == -1) {
        printf("Open fifo file %s error: %s\n", dir, strerror(errno));
        return -1;
    }
    an_make_nonblock(fd);
    return fd;
}

int an_make_fifos(){
    int i, fd;
    event *e;
    for(i=0; i<128; ++i) {
        if(protos[i].id != i) {
            continue;
        }
        fd = an_create_and_open_fifo(protos[i].fifo_in_dir, O_RDWR|O_NONBLOCK);
        if(fd == -1) {
            puts("IN_DIR");
            continue;
        }
        protos[i].fifo_in_fd = fd;
        fd = an_create_and_open_fifo(protos[i].fifo_out_dir, O_RDWR|O_NONBLOCK);
        if(fd == -1) {
            puts("OUT_DIR");
            continue;
        }
        protos[i].fifo_out_fd = fd;
        e = events + i;
        e->fd = fd;
        e->ptr = (void*)(protos + i);
        e->handler = fifo_handler;
        an_add_event(e, EPOLLIN|EPOLLET);
    }
    return 0;
}

int fifo_handler(struct epoll_event e) {
    uint8_t buffer[1400];
    int size;
    event *ep;
    proto_type *ptp;
    ep = (event *)e.data.ptr;
    ptp = (proto_type *)ep->ptr;
    size = read(ep->fd, buffer, sizeof(buffer));
    if(size == -1) {
        printf("Read from %s error: %s\n", ptp->fifo_out_dir, strerror(errno));
        return 0;
    }
    an_broadcast_msg(ptp->id, 0, buffer, size);
    return 0;
}
