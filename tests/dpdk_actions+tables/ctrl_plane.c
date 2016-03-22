#include "dpdk_lib.h"
#include "action_info.h"

extern void table_setdefault_promote  (int tableid, uint8_t* value);
extern void exact_add_promote  (int tableid, uint8_t* key, uint8_t* value);
extern void lpm_add_promote    (int tableid, uint8_t* key, uint8_t depth, uint8_t* value);
extern void ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value);

///////////////////////////////////////////////////////////////////////////////
// TABLE MANIPUATION

void
table1_add(uint8_t field1[4], uint8_t field1_prefix_length, struct table1_action action)
{
    uint8_t prefix_length = field1_prefix_length;
    lpm_add_promote(TABLE1, (uint8_t*)field1, prefix_length, (uint8_t*)&action);
}

///////////////////////////////////////////////////////////////////////////////
// CALLBACKS

void table1_add_table_entry(struct p4_ctrl_msg* ctrl_m)
{
    uint8_t* field1 = (uint8_t*)(((struct p4_field_match_lpm*)ctrl_m->field_matches[0])->bitmap);
    uint16_t field1_prefix_length = ((struct p4_field_match_lpm*)ctrl_m->field_matches[0])->prefix_length;
    if (strcmp("korte", ctrl_m->action_name)==0)
    {
        uint8_t* ap1 = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[0])->bitmap;
        uint8_t* ap2 = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[1])->bitmap;
        uint8_t* ap3 = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[2])->bitmap;
        struct table1_action action;
        action.action_id = action_korte;
        action.korte_params = (struct action_korte_params){*ap1, *ap2, *ap3};
        table1_add(field1, (uint8_t)field1_prefix_length, action);
    }
}

void recv_from_controller(struct p4_ctrl_msg* ctrl_m)
{
    if (ctrl_m->type == P4T_ADD_TABLE_ENTRY) {
        if (strcmp("table1", ctrl_m->table_name) == 0)
			table1_add_table_entry(ctrl_m);
    }
}

backend bg;

void init_control_plane()
{
	printf("Creating control plane connection...\n");
	bg = create_backend(3, 1000, "localhost", 11111, recv_from_controller);
	launch_backend(bg);
}
