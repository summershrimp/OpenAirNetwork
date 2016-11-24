#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "main.h"
#include "config.h"
#include "network.h"
#include "event.h"
#include "subprocess.h"


int main(void) {
    an_load_config();
    //an_make_daemon();
    printf("Finish loading config.\n");
    an_init_event_loop(2000);
    an_make_fifos();
    an_start_server("0.0.0.0", port);
    return 0;
}

int an_make_daemon() {
    pid_t ppid,psid;
    ppid = fork();
    if(ppid < 0) {
        printf("Fork failed: %s", strerror(errno));
        exit(errno);
    } else if(ppid == 0) {
        psid = setsid();
        if(psid == -1) {
            printf("Cannot set sid: %s", strerror(errno));
            exit(errno);
        }
        return 0;
    }
    exit(EXIT_SUCCESS);
    return -1;
}
