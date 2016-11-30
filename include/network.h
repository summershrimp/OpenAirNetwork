#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <arpa/inet.h>
#include <stdint.h>
#include <queue>
#include "arq.h"
typedef struct {
    in_addr_t addr;
    uint32_t recv_seq;
    uint32_t send_seq;
    std::queue<arq_t> wait_queue;
} craft_addr;

int an_start_server(char *addr, int port);
int an_broadcast_msg(uint8_t type, uint8_t code, uint8_t *src, int size);


extern int listenfd;
#endif

