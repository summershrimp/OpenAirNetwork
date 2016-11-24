#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <arpa/inet.h>
#include <stdint.h>

typedef struct {
    in_addr_t addr;
    int recv_seq;
    int send_seq;
    int fd;
} craft_addr;

int an_start_server(char *addr, int port);
int an_broadcast_msg(uint8_t type, uint8_t code, uint8_t *src, int size);

#endif

