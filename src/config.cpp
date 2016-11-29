#include "config.h"
#include <string.h>
#include <arpa/inet.h>
craft_addr crafts[10];
proto_type protos[128];

std::map<in_addr_t, craft_addr*> addr_map;

int port = 4555;
int is_master = 1;
int craft_cnt = 1;
int event_once = 100;

int an_load_config() {
    crafts[0].addr = inet_addr("192.168.59.3");
    crafts[0].send_seq = 0;
    crafts[0].recv_seq = 0;
    
    protos[0].id = 0;
    strcpy(protos[0].fifo_dir, "/tmp/an_fifo_0");
    protos[0].fifo_fd = -1;
    return 0;
}

int an_init_config() {
    int i;
    for(i=0; i<craft_cnt; ++i) {
        addr_map[crafts[i].addr] = crafts + i;
    }
    return 0;
}
