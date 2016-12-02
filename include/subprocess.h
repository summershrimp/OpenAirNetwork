#ifndef __SUBPROCESS_H__
#define __SUBPROCESS_H__
#include <stdint.h>

typedef struct {
    uint8_t id;
    char fifo_in_dir[128];
    char fifo_out_dir[128];
    int fifo_in_fd;
    int fifo_out_fd;
} proto_type;

int an_make_fifos();
#endif
