
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <p4_hw_tables.h>

#include "table_config.h"
#include "control_plane_actions.h"

#include <stdio.h>
#include <stddef.h>

extern struct table_t table_dmac;
extern struct table_t table_smac;

extern void set_table_entry_data(struct table_t*const table, int entry_index, const char*const data);
extern bool table_contains_entry(struct table_t*const table, const char*const entry_data);

static struct control_plane_action_set_table_entry* msg1;
static struct control_plane_action_set_table_entry* msg2;

extern const int table_dmac_action_dmac_miss;
extern const int table_dmac_action_dmac_hit;
extern const int table_smac_action_smac_miss;
extern const int table_smac_action_smac_hit;

extern void dbg_printf(int verbosity_level, const char *fmt, ...);
extern void dbg_print_hex(int verbosity_level, int length, const char*const data);

extern long mac_to_long(const unsigned char*const mac);


void handle_set_table_action(struct control_plane_action_set_table_entry* msg)
{
    if (table_contains_entry(msg->table, msg->data)) {
        dbg_printf(1, "table %p already contains data, no new table entry is necessary\n", msg->table);
        return;
    }

    set_table_entry_data(msg->table, msg->entry_index, msg->data);
    ++msg->table->entry_count;

    dbg_printf(0, "control sent new entry for table %p: ", msg->table);
    dbg_print_hex(0, msg->table->entry_size, msg->data);
}

// The first message that the control plane generates in response to the digest.
void control_plane_generate_dmac_msg(unsigned char* mac, int ingress_port)
{
    msg1 = malloc(sizeof(struct control_plane_action_set_table_entry*));
    msg1->table = &table_dmac;
    msg1->entry_index = table_dmac.entry_count;
    msg1->data = malloc(sizeof(struct table_entry_dmac_t));
    memcpy(msg1->data + offsetof(struct table_entry_dmac_t, mac), mac, 6);
    *(int*)(msg1->data + offsetof(struct table_entry_dmac_t, action_code)) = table_dmac_action_dmac_hit;
    *(int*)(msg1->data + offsetof(struct table_entry_dmac_t, port)) = ingress_port;
}

// The second message that the control plane generates in response to the digest.
void control_plane_generate_smac_msg(unsigned char* mac, int ingress_port)
{
    msg2 = malloc(sizeof(struct control_plane_action_set_table_entry*));
    msg2->table = &table_smac;
    msg2->entry_index = table_smac.entry_count;
    msg2->data = malloc(sizeof(struct table_entry_smac_t));
    memcpy(msg2->data + offsetof(struct table_entry_smac_t, mac), mac, 6);
    *(int*)(msg2->data + offsetof(struct table_entry_smac_t, action_code)) = table_smac_action_smac_hit;
    *(int*)(msg2->data + offsetof(struct table_entry_smac_t, port)) = ingress_port;
}

// Placeholder: the data plane punts up data to the control plane.
// Here we assume that we get two messages from the control plane as answers.
void send_msg_to_control_plane(unsigned char* mac, int ingress_port)
{
    control_plane_generate_dmac_msg(mac, ingress_port);
    control_plane_generate_smac_msg(mac, ingress_port);
}

// Currently only this control plane action type is supported.
static bool is_received_action_set_table_entry()
{
    return true;
}

void recv_msg_from_control_plane()
{
    if (is_received_action_set_table_entry()) {
        dbg_printf(1, "receiving messages from control plane...\n");
        handle_set_table_action(msg1);
        handle_set_table_action(msg2);

        dbg_printf(1, "current dmac entry count %d\n", table_dmac.entry_count);
        dbg_printf(1, "current smac entry count %d\n", table_smac.entry_count);
    }
}
