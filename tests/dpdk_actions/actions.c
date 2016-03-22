#include "dpdk_lib.h"
#include <assert.h>

lookup_table_t table_config[] = {}; // irrelevant here

static void
parse_header(uint8_t* buffer, packet_descriptor_t* packet_desc, header_instance_t h)
{
    packet_desc->headers[h] =
      (header_descriptor_t) {
        .type = h,
        .pointer = buffer,
        .length = header_instance_byte_width[h]
      };
}
static void
test_parse_packet(uint8_t* buffer, int len, packet_descriptor_t* packet_desc)
{
    packet_desc->pointer = buffer;
    parse_header(buffer, packet_desc, header_instance_whatever);
    buffer += 4;
    parse_header(buffer, packet_desc, header_instance_whatever2);
}

static void pb(size_t const size, void const * const ptr){
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    for (j = size-1; j >= 0; j--) {
        i = j/8;
        byte = b[i] & (1<<(j%8));
        byte >>= (j%8);
        printf("%u", byte);
    }
    puts("");
}
static void ipb(uint32_t v){pb(32, &v);}

/*

header_type whatever_t {
    fields {
        field1 : 16;
        field2 : 16;
    }
}

header_type whatever2_t {
    fields {
        field3 : 11;
        field4 : 21;
    }
}

header whatever_t whatever;
header whatever2_t whatever2;

action ekson1() {
    add_header(whatever2);

    add(whatever.field1, whatever.field2, 0x0002);

    modify_field(whatever2.field3, whatever.field1);
    modify_field(whatever2.field4, 0xaaaa);

    add_to_field(whatever2.field4, 0x0005);

    remove_header(whatever);
}

*/

// ============================================================================
// CAN BE GENERATED FROM THE ABOVE
// ============================================================================

static void
ekson1(packet_descriptor_t* p, void* action_data)
{
    printf("Hello DPDK!\n");
    modify_field_to_const(p, field_desc(field_instance_whatever2_field3), (uint8_t[]) {0x05,0x00}, 2);
    modify_field_to_const(p, field_desc(field_instance_whatever2_field4), (uint8_t[]) {0x09,0x00,0x00}, 3);
    add_const_to_field(p, field_desc(field_instance_whatever2_field3), (uint8_t[]) {0x09,0x00}, 2);
    add_const_to_field(p, field_desc(field_instance_whatever2_field4), (uint8_t[]) {0x09,0x00,0x00}, 3);

    add_field_to_field(p, field_desc(field_instance_whatever2_field3), field_desc(field_instance_whatever2_field4));
    add_fields(p, field_desc(field_instance_whatever_field1), field_desc(field_instance_whatever2_field3), field_desc(field_instance_whatever2_field4));
    modify_field_to_field(p, field_desc(field_instance_whatever_field2), field_desc(field_instance_whatever_field1));
}

// ============================================================================
// TEST IT
// ============================================================================

static struct rte_mbuf*
fake_incoming_packet()
{
    struct rte_mempool* mempool;
    mempool = rte_mempool_create("test_mbuf_pool", (unsigned)1023, MBUF_SIZE, MEMPOOL_CACHE_SIZE, sizeof(struct rte_pktmbuf_pool_private), rte_pktmbuf_pool_init, NULL, rte_pktmbuf_init, NULL, 0, 0);
    struct rte_mbuf *p = NULL;
    p = rte_pktmbuf_alloc(mempool);
    char* whatever2 = rte_pktmbuf_prepend(p, 4);
    memcpy(whatever2, (uint8_t[]) {0x00,0x00,0x00,0x00}, 4);
    char* whatever = rte_pktmbuf_prepend(p, 4);
    memcpy(whatever, (uint8_t[]) {0x01,0x00,0x02,0x00}, 4);
    return p;
}

static void
debug_field(packet_descriptor_t* p, field_reference_t f)
{
    printf("field width is %d bits\n", f.bitwidth);
    uint32_t extract = extract_intvalue(p, f);
    pb(f.bitwidth, &extract);
    printf("Decimal: %" PRIu32 "\n", extract);
}

static void
debug(packet_descriptor_t* packet_desc)
{
    //rte_be_to_cpu_16(*(uint16_t*)field);
    debug_field(packet_desc, field_desc(field_instance_whatever_field1));
    debug_field(packet_desc, field_desc(field_instance_whatever_field2));
    debug_field(packet_desc, field_desc(field_instance_whatever2_field3));
    debug_field(packet_desc, field_desc(field_instance_whatever2_field4));
    debug_field(packet_desc, field_desc(field_instance_standard_metadata_ingress_port));
}

static void
init_metadata(packet_descriptor_t* packet_desc, uint32_t inport)
{
    packet_desc->headers[header_instance_standard_metadata] =
      (header_descriptor_t) {
        .type = header_instance_standard_metadata,
        .length = header_instance_byte_width[header_instance_standard_metadata],
        .pointer = calloc(header_instance_byte_width[header_instance_standard_metadata], sizeof(uint8_t))
      };
    modify_field_to_const(packet_desc, field_desc(field_instance_standard_metadata_ingress_port), (uint8_t*)&inport, 2);
}


int main(int argc, char **argv)
{
    initialize(argc, argv);
    struct rte_mbuf* p = fake_incoming_packet();
    packet_descriptor_t packet_desc;

    init_metadata(&packet_desc, 111);

    uint8_t *buffer = rte_pktmbuf_mtod(p, uint8_t *);

    test_parse_packet(buffer, rte_pktmbuf_pkt_len(p), &packet_desc);

    ekson1(&packet_desc, NULL);

    debug(&packet_desc);

    return 0;
}
