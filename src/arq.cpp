#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "arq.h"
#include "network.h"
#include "config.h"

int an_arq_retry(){
    int i;
    for(i=0; i<craft_cnt; ++i) {
        sockaddr_in addr;
        craft_addr *a = crafts + i;
        arq_t at = (a->wait_queue).front();
        addr.sin_port = port;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = a->addr;
        int result = sendto(listenfd, at.data, at.size, 0, (struct sockaddr *) &addr, sizeof(addr));
        if(result == -1) {
            printf("ARQ re-send to %s failed: %s\n", inet_ntoa(addr.sin_addr), strerror(errno));
        }
    }
    return 0;
}

