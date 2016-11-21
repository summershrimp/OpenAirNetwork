#ifndef __EVENTS_H__
#define __EVENTS_H__

#include <sys/epoll.h>
#include "network.h"

typedef int (*event_handler) (struct epoll_event ev);

typedef struct {
    int fd;
    event_handler handler;
    craft_addr craft;
} event;

int an_start_event_loop();
int an_init_event_loop(int max_fd);
int an_remove_event(event *e, int events);
int an_add_event(event *e, int events);
#endif
