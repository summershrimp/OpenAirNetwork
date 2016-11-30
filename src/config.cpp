#include "config.h"
#include <string.h>
#include <arpa/inet.h>
#include <yaml.h>

craft_addr crafts[10];
proto_type protos[128];

std::map<in_addr_t, craft_addr*> addr_map;

int port = 4555;
int is_master = 1;
int craft_cnt = 1;
int event_once = 100;

yaml_parser_t parser;
yaml_event_t event;
yaml_document_t document;
int an_load_config() {
    
    yaml_parser_initialize(&parser);
    FILE *fp = fopen("config.yml", "r");
    yaml_parser_set_input_file(&parser, fp);
    int done = 0;
    yaml_event_type_t last_event = YAML_NO_EVENT;
    while(!done) {
        if(!yaml_parser_parse(&parser, &event)) {
            printf("Parse yaml config file failed.");
            exit(0);
        }
        switch(event.type) {
        	case YAML_NO_EVENT: puts("No event!"); break;
	       	case YAML_STREAM_START_EVENT: puts("STREAM START"); break;
    		case YAML_STREAM_END_EVENT:   puts("STREAM END");   break;
    		case YAML_DOCUMENT_START_EVENT: puts("<b>Start Document</b>"); break;
    		case YAML_DOCUMENT_END_EVENT:   puts("<b>End Document</b>");   break;
    		case YAML_SEQUENCE_START_EVENT: puts("<b>Start Sequence</b>"); break;
    		case YAML_SEQUENCE_END_EVENT:   puts("<b>End Sequence</b>");   break;
    		case YAML_MAPPING_START_EVENT:  puts("<b>Start Mapping</b>");  break;
    		case YAML_MAPPING_END_EVENT:    puts("<b>End Mapping</b>");    break;
    		case YAML_ALIAS_EVENT:  printf("Got alias (anchor %s)\n", event.data.alias.anchor); break;
    		case YAML_SCALAR_EVENT: printf("Got scalar (value %s)\n", event.data.scalar.value); break;
 
        }
        last_event = event.type;
        done = (event.type == YAML_STREAM_END_EVENT);
        yaml_event_delete(&event);
        
    }
    yaml_parser_delete(&parser);
    fclose(fp);


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
