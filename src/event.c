#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "event.h"
#include "config.h"

/*
#define HASH_BASE 131

typedef struct _event_hashitem {
    event e;
    _event_hashitem *next;
} event_hashitem;

event_hashitem *event_hashmap[HASH_BASE];

event* event_map_get(int fd) {
    event_hashitem *p = event_hashmap[fd % HASH_BASE];
    while(p) {
        if(p->e.fd == fd) {
            return &(p->e);
        }
        p = p->next;
    }
    return NULL;
}


int event_map_put(event e) {
    event *p;
    event_hashitem *it;
    int hash;
    p = event_map_get(e.fd);
    if(p != NULL) {
        return 1;
    }
    it = (event_hashitem *)malloc(sizeof(event_hashitem));
    if(!it) {
        printf("Allocate memory failed: %s", strerror(errno));
        exit(-1);
    }
    it->e = e;
    hash = e.fd % HASH_BASE;
    it->next = event_hashmap[hash];
    event_hashmap[hash] = it;
    return 0;
}

int event_map_remove(int fd) {
    int hash = fd % HASH_BASE;
    event_hashitem *last, *p = event_hashmap[hash];
    last = NULL;
    if(p == NULL) {
        return 1;
    }
    while(p->next) {
        if(p->e.fd == fd) {
            if(last == NULL) {
                event_hashmap[hash] = p->next;
            } else {
               last->next = p->next;
            }
            free(p);
            return 0;
        }
        last = p;
        p = p->next;
    }
    return 1;
}
*/
int epoll_fd = -1;

int an_init_event_loop(int max_fd) {
    struct rlimit rt;
    rt.rlim_max = rt.rlim_cur = max_fd + 1;
    if(setrlimit(RLIMIT_NOFILE, &rt) == -1) {
        printf("Set rlimit failed: %s\n", strerror(errno));
        exit(errno);
    }
    epoll_fd = epoll_create(max_fd);
    printf("Successfully create epoll fd: %d\n", epoll_fd);
    return 0;
}

int an_start_event_loop() {
    struct epoll_event *events;
    events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * event_once);
    int event_cnt, i;
    event *e;
    for( ; ; ) {
        event_cnt = epoll_wait(epoll_fd, events, event_once, -1);
        if(event_cnt == -1) {
            printf("Wait for epoll events failed: %s\n", strerror(errno));
            exit(errno);
        }
        for(i=0; i<event_cnt; ++i) {
            e = (event *)events[i].data.ptr;
            e->handler(events[i]);
        }
    }
}

int an_add_event(event *e, int events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.ptr = (void *)e;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, e->fd, &ev) == -1) {
        printf("Add fd to epoll failed: %s\n", strerror(errno));
        exit(errno);
    }
    printf("Successfully add event: %d\n", e->fd);
    return 0;
}

int an_remove_event(event *e, int events) {
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = e->fd;
    ev.data.ptr = (void *)e;
    if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, e->fd, &ev) == -1) {
        printf("Delete fd from epoll failed: %s\n", strerror(errno));
        return 1;
    }
    return 0;
}
