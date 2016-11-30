#ifndef __ARQ_H__
#define __ARQ_H__

#include "stdint.h"

typedef struct _arq_t {
    int size;
    uint8_t data[1450];
    struct _arq_t *next;
} arq_t;


int an_arq_retry();

#endif
