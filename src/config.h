#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MAX_CRAFT 10

extern int event_once;

#include "network.h"

extern int port;

extern craft_addr crafts[10];
extern int craft_cnt;

int an_load_config();
#endif

