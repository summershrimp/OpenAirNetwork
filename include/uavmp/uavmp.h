#ifndef __UAVMP_H__
#define __UAVMP_H__

#include <stdint.h>

typedef struct {
    uint8_t version:4;
    uint8_t rsv:4;
    uint8_t type:8;
    uint8_t code:8;
    uint8_t crc8:8;
    uint32_t idt:32;
    uint32_t seq:32;
} uavmp_t;

#endif

