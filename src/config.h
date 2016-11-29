#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MAX_CRAFT 10

#include <map>

extern int event_once;

#include "network.h"
#include "subprocess.h"

extern int port;

extern craft_addr crafts[10];
extern int craft_cnt;

extern int is_master;

extern proto_type protos[128];

extern std::map<in_addr_t, craft_addr*> addr_map;

int an_load_config();
#endif

