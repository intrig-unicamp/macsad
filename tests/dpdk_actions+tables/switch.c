#include "dpdk_lib.h"
#include <unistd.h>

extern void init_control_plane();
extern int launch_dpdk();

#include "table_info.h" // only once

#define IPv4(a,b,c,d) ((uint32_t)(((a) & 0xff) << 24) | (((b) & 0xff) << 16) | (((c) & 0xff) << 8)  | ((d) & 0xff))

extern backend bg;

static void
fake_digest(uint16_t pr, int i3)
{
    uint32_t ip = IPv4(65, 208, 228, 223); // 41d0e4df
    int i1 = 11;
    int i2 = 22;

    struct type_field_list fields;
    fields.fields_quantity = 5;
    fields.field_offsets = malloc(sizeof(uint8_t*)*fields.fields_quantity);
    fields.field_widths = malloc(sizeof(uint8_t*)*fields.fields_quantity);
    //fields.field_offsets = (uint8_t[]) {(uint8_t*)&ip, (uint8_t*)&pr, (uint8_t*)&i1, (uint8_t*)&i2, (uint8_t*)&i3};
    fields.field_offsets[0] = (uint8_t*)&ip;
    fields.field_offsets[1] = (uint8_t*)&pr;
    fields.field_offsets[2] = (uint8_t*)&i1;
    fields.field_offsets[3] = (uint8_t*)&i2;
    fields.field_offsets[4] = (uint8_t*)&i3;
    fields.field_widths[0] = sizeof(ip)*8;
    fields.field_widths[1] = sizeof(pr)*8;
    fields.field_widths[2] = sizeof(int)*8;
    fields.field_widths[3] = sizeof(int)*8;
    fields.field_widths[4] = sizeof(int)*8;

    generate_digest(bg, "test_learn_ip", 0, &fields);
}

int main(int argc, char** argv)
{
    printf("\n\n### HELLO DPDK SWITCH!\n\n");
    initialize(argc, argv);
    printf("\n\n### Backend initialized.\n\n");
    init_control_plane();
    printf("\n\n### Control plane initialized.\n\n");
    fake_digest(32, 33);
    fake_digest(24, 42);
    printf("### Waiting for fake digests to be applied...\n");
    sleep(1);
    printf("\n\n### Control plane filled the tables.\n\n");
    return launch_dpdk();
}
