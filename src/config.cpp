#include "config.h"
#include <string.h>
#include <arpa/inet.h>
#include <yaml.h>
#include <stack>
#include <string.h>

craft_addr crafts[10];
proto_type protos[128];

std::map<in_addr_t, craft_addr*> addr_map;

int port = 4556;
int is_master = 0;
int craft_cnt = 0;
int event_once = 100;

yaml_parser_t parser;
yaml_token_t token;
yaml_document_t document;
enum parse_status{ NORMAL, IN_CRAFTS, IN_PROTOS, PROTOS_SUB};

int an_load_config() {
    
    protos[0].id = 1;

    yaml_parser_initialize(&parser);
    FILE *fp = fopen("config.yml", "r");
    yaml_parser_set_input_file(&parser, fp);
    std::stack<yaml_token_type_t> st;
    std::stack<parse_status> st_status;
    char *key = NULL;
    int tmp_id = 0;
    char *tmp_in_fifo = NULL, *tmp_out_fifo = NULL;
	do {
        yaml_parser_scan(&parser, &token);
        switch(token.type){
    	/* Stream start/end */
    	case YAML_STREAM_START_TOKEN: 
            puts("STREAM START");
            st.push(token.type); 
            break;
    	case YAML_STREAM_END_TOKEN:
            puts("STREAM END");
            st.pop();
            break;
    	/* Token types (read before actual token) */
    	case YAML_KEY_TOKEN: 
            st.push(token.type);
            break;
    	case YAML_VALUE_TOKEN:
            st.push(token.type);
            break;
    	/* Block delimeters */
    	case YAML_BLOCK_SEQUENCE_START_TOKEN:
            puts("<b>Start Block (Sequence)</b>");
            if(!strcmp(key, "crafts")) {
                st_status.push(IN_CRAFTS);
            } else if(!strcmp(key, "protos")){
                st_status.push(IN_PROTOS);
            } else {
                printf("Unknown config: %s\n", key);
            }
            st.push(token.type);
            break;
        case YAML_BLOCK_ENTRY_TOKEN:
            puts("<b>Start Block (Entry)</b>");
    	    st.push(token.type);
            if(!st_status.empty() && st_status.top() == IN_PROTOS) {
                st_status.push(PROTOS_SUB);
            }
            break;
        case YAML_BLOCK_END_TOKEN:
            puts("<b>End block</b>");
            st.pop();
            if(!st_status.empty() && st_status.top() == PROTOS_SUB) {
                printf("Proto: id=%d, fifo=%s,%s\n", tmp_id, tmp_in_fifo, tmp_out_fifo);
                st_status.pop();
                if(tmp_id > 127 || tmp_id < 0 || tmp_in_fifo == NULL || tmp_out_fifo == NULL) {
                    continue;
                } 
                protos[tmp_id].id = tmp_id;
                strcpy(protos[tmp_id].fifo_in_dir, tmp_in_fifo);
                strcpy(protos[tmp_id].fifo_out_dir, tmp_out_fifo);
                protos[tmp_id].fifo_in_fd = -1;
                protos[tmp_id].fifo_out_fd = -1;
                free(tmp_in_fifo);
                free(tmp_out_fifo);
                tmp_in_fifo = NULL;
                tmp_out_fifo = NULL;
            }
            break;
    	/* Data */
    	case YAML_BLOCK_MAPPING_START_TOKEN:  puts("[Block mapping]");            break;
    	case YAML_SCALAR_TOKEN:  
            switch(st.top()){
            case YAML_KEY_TOKEN: 
                if(key != NULL){
                    free(key);
                }
                key = strdup((char *)token.data.scalar.value);
                break;
            case YAML_VALUE_TOKEN:
                char *tmp_value;
                tmp_value = (char *)token.data.scalar.value;
                if(!st_status.empty() && st_status.top() == PROTOS_SUB) {
                    if(!strcmp(key, "id")) {
                        tmp_id = atoi(tmp_value);
                        if(tmp_id == 0){
                            printf("Wrong protocal type id: %s\n", tmp_value);
                            exit(-1);
                        }
                    } else if(!strcmp(key, "fifo_in")) {
                        tmp_in_fifo = strdup(tmp_value);
                    } else if(!strcmp(key, "fifo_out")) {
                        tmp_out_fifo = strdup(tmp_value);
                    }
                } else {
                    if(!strcmp(key, "port")) {
                        port = atoi(tmp_value);
                        if(port <= 0) {
                            printf("Wrong port: %s\n", tmp_value);
                            exit(-1);
                        }
                    } else if (!strcmp(key, "master")) {
                        if(!strcmp(tmp_value, "true")) {
                            is_master = true;
                        } else if(!strcmp(tmp_value, "false")) {
                            is_master = false;
                        } else {
                            printf("Wrong master type: %s, should be true|false.\n", tmp_value);
                            //exit(0);
                        }
                    }
                }
                printf("%s: %s\n", key, token.data.scalar.value);
                break;
            case YAML_BLOCK_ENTRY_TOKEN:
                if(!st_status.empty()){
                    if(st_status.top() == IN_CRAFTS) {
                        crafts[craft_cnt].addr = inet_addr((char *)token.data.scalar.value);
                        crafts[craft_cnt].send_seq = 0;
                        crafts[craft_cnt].recv_seq = 0;
                        ++craft_cnt;
                    }
                }
                printf("%s\n", token.data.scalar.value);
                break;
            default:
                break;
            }
            break;
    	/* Others */
    	default:
    		printf("Got token of type %d\n", token.type);
    }
    if(token.type != YAML_STREAM_END_TOKEN)
        yaml_token_delete(&token);
    } while(token.type != YAML_STREAM_END_TOKEN);

    yaml_parser_delete(&parser);
    fclose(fp);

    printf("UDP Port: %d\n", port);
    printf("Is master: %d\n", is_master);

    for(int i = 0; i < craft_cnt; ++i) {
        in_addr in;
        in.s_addr = crafts[i].addr;
        printf("Craft %d addr: %s\n", i, inet_ntoa(in));
    }

    for(int i=0; i < 128; ++i) {
        if(protos[i].id == i) {
            printf("Proto type id=%d, fifo=%s,%s\n", protos[i].id, protos[i].fifo_in_dir, protos[i].fifo_out_dir);
        }
    }

    return 0;
}

int an_init_config() {
    int i;
    for(i=0; i<craft_cnt; ++i) {
        addr_map[crafts[i].addr] = crafts + i;
    }
    return 0;
}
