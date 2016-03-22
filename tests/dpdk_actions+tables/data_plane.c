#include "dpdk_lib.h"
#include "action_info.h"

extern void parse_packet(packet_descriptor_t* packet_desc);

static uint8_t* table1_lookup(lookup_table_t** tables, uint8_t field1[4])
{
    return lpm_lookup(tables[TABLE1], (uint8_t*)field1);
}

static void action_code_alma(packet_descriptor_t* packet, lookup_table_t** tables, struct action_alma_params params)
{
    printf("### RUNNING ACTION alma...\n");
    int val = 55;
    modify_field_to_const(packet, field_desc(field_instance_standard_metadata_egress_port), &val, 4);
}

static void action_code_korte(packet_descriptor_t* packet, lookup_table_t** tables, struct action_korte_params params)
{
    printf("### RUNNING ACTION korte with params (%d, %d, %d)...\n", params.i1, params.i2, params.i3);
    int val = params.i3;
//    modify_field_to_field(packet, field_desc(field_instance_standard_metadata_egress_port), field_desc(field_instance_standard_metadata_ingress_port));
    modify_field_to_const(packet, field_desc(field_instance_standard_metadata_egress_port), &val, 4);
}

void table1(packet_descriptor_t* packet, lookup_table_t** tables) {

    uint32_t ip = extract_intvalue(packet, field_desc(field_instance_ipv4_dstAddr));

    struct table1_action* res = (struct table1_action*)table1_lookup(tables, (uint8_t*)&ip);
    if(res == NULL) {
        printf("### NO RESULT, NO DEFAULT ACTION, IGNORING PACKET...\n");
        return; // nothing to do if there is no default action
    }
    switch (res->action_id) {
    case action_korte:
        action_code_korte(packet, tables, res->korte_params);
        break;
    case action_alma:
        action_code_alma(packet, tables, res->alma_params);
        break;
    }
        
}

static void printip(uint32_t ip) {
    int a = (ip & 0xff000000) >> 24;
    int b = (ip & 0x00ff0000) >> 16;
    int c = (ip & 0x0000ff00) >> 8;
    int d = ip & 0x000000ff;
    printf("%d.%d.%d.%d", a, b, c, d);
}

void handle_packet(packet_descriptor_t* packet, lookup_table_t** tables) {

    parse_packet(packet);

    printf("\n\n### HANDLING PACKET...\n");
    printf("  ipv4 dstaddr: %" PRIx32 ", that is ", extract_intvalue(packet, field_desc(field_instance_ipv4_dstAddr)));
    printip(extract_intvalue(packet, field_desc(field_instance_ipv4_dstAddr)));
    printf("\n");

    table1(packet, tables);
}

