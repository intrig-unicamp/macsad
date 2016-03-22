#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include "messages.h"

#define P4_MAX_NUMBER_OF_ACTION_PARAMETERS 10
#define P4_MAX_NUMBER_OF_FIELD_MATCHES 10

struct p4_ctrl_msg {
	uint8_t type;
	uint32_t xid;
	char* table_name;
	uint8_t action_type;
	char* action_name;
	int num_action_params;
	struct p4_action_parameter* action_params[P4_MAX_NUMBER_OF_ACTION_PARAMETERS];
	int num_field_matches;
	struct p4_field_match_header* field_matches[P4_MAX_NUMBER_OF_FIELD_MATCHES];
};

typedef void (*p4_msg_callback)(struct p4_ctrl_msg*);

int handle_p4_msg(char* buffer, int length, p4_msg_callback cb);
int handle_p4_set_default_action(struct p4_set_default_action* m, struct p4_ctrl_msg* ctrl_m);
int handle_p4_add_table_entry(struct p4_add_table_entry* m, struct p4_ctrl_msg* ctrl_m);


#endif
