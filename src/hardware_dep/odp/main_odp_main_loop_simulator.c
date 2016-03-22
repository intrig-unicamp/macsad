#include "dpdk_lib.h"
#include <unistd.h>

__m128i val_eth[RTE_MAX_ETHPORTS];
struct rte_mempool * pktmbuf_pool[NB_SOCKETS];
struct rte_mempool *header_pool, *clone_pool;
struct lcore_conf lcore_conf[RTE_MAX_LCORE];

//=   shared   ================================================================

uint32_t enabled_port_mask = 0;
struct ether_addr ports_eth_addr[RTE_MAX_ETHPORTS];

//=============================================================================

#define EXTRACT_EGRESSPORT(p) (*(uint32_t *)(((uint8_t*)(p)->headers[/*header instance id - hopefully it's the very first one*/0].pointer)+/*byteoffset*/6) & /*mask*/0x7fc) >> /*bitoffset*/2

static inline int
send_packet(packet_descriptor_t* packet_desc)
{
    int port = EXTRACT_EGRESSPORT(packet_desc);
	uint32_t lcore_id;
	lcore_id = rte_lcore_id();
    printf("  :::: PACKET HANDLED SUCCESSFULLY BY LCORE %d, TRANSMITTING ON PORT %d\n", lcore_id, port);
	return 0;
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

struct rte_mbuf* fake_packets[2][4];

void
dpdk_main_loop(void)
{
    unsigned lcore_id = rte_lcore_id();
    struct lcore_conf *conf = &lcore_conf[lcore_id];
	RTE_LOG(INFO, L2FWD, "entering main loop on lcore %u\n", lcore_id);
    int index = 0;
    struct rte_mbuf* p;
	while(index < 4) {
        p = fake_packets[lcore_id][index];
        packet_descriptor_t packet_desc;
        packet_desc.pointer = rte_pktmbuf_mtod(p, uint8_t *);
        init_metadata(&packet_desc, (lcore_id*2+index+1)); // fake input port
        handle_packet(&packet_desc, conf->tables);
        send_packet(&packet_desc);
        if(index == 1) {
            printf("### PACKET_GEN IS WAITING FOR THE DATAPLANE TO LEARN\n");
            sleep(1);
        }
        index++;
    }
}


static int
launch_one_lcore(__attribute__((unused)) void *dummy)
{
	dpdk_main_loop();
	return 0;
}

struct rte_mempool* fakemempool;

static
void str2bytes(const char* str, size_t len, char* res)
{
    const char *pos = str;
    size_t count = 0;
    for(count = 0; count < len; count++) {
        sscanf(pos, "%2hhx", &res[count]);
        pos += 2;
    }
}

static struct rte_mbuf*
fake_incoming_ipv4_packet(const char* ipv4_dstaddr)
{
    struct rte_mbuf *p = rte_pktmbuf_alloc(fakemempool);
    char* payload = rte_pktmbuf_prepend(p, 4);
    str2bytes("ffffffff", 4, payload);
    char* ipv4 = rte_pktmbuf_prepend(p, 20);
    str2bytes("450000280f444000800691f091fea0ed", 16, ipv4);
    str2bytes(ipv4_dstaddr, 4, ipv4+16);
    char* ethernet = rte_pktmbuf_prepend(p, 14);
    str2bytes("080027fa4ff60000010000000800", 14, ethernet);
    return p;
}

static struct rte_mbuf*
fake_incoming_eth_packet(const char* dst, const char* src)
{
    struct rte_mbuf *p = rte_pktmbuf_alloc(fakemempool);
    char* payload = rte_pktmbuf_prepend(p, 4);
    str2bytes("ffffffff", 4, payload);
    char* ethernet = rte_pktmbuf_prepend(p, 14);
    str2bytes(dst, 6, ethernet+0);
    str2bytes(src, 6, ethernet+6);
    str2bytes("0800", 2, ethernet+12);
    return p;
}

static void
init_fake_packets()
{
    fakemempool = rte_mempool_create("test_mbuf_pool", (unsigned)1023, MBUF_SIZE, MEMPOOL_CACHE_SIZE, sizeof(struct rte_pktmbuf_pool_private), rte_pktmbuf_pool_init, NULL, rte_pktmbuf_init, NULL, 0, 0);
    //fake_packets[0][0] = fake_incoming_ipv4_packet("41d0e4df");
    //fake_packets[0][1] = fake_incoming_ipv4_packet("41d0e400");
    //fake_packets[1][0] = fake_incoming_ipv4_packet("41d0e4df");
    //fake_packets[1][1] = fake_incoming_ipv4_packet("41d00000");

    // TEACH THE DATAPLANE
    fake_packets[0][0] = fake_incoming_eth_packet("001234567890", "000001000000");
    fake_packets[0][1] = fake_incoming_eth_packet("001234567890", "000002000000");
    fake_packets[1][0] = fake_incoming_eth_packet("001234567890", "000003000000");
    fake_packets[1][1] = fake_incoming_eth_packet("001234567890", "000004000000");

    // CHECK IF IT KNOWS THE ADDRESSES NOW
    fake_packets[0][2] = fake_incoming_eth_packet("000001000000", "001234567890");
    fake_packets[0][3] = fake_incoming_eth_packet("000002000000", "001234567890");
    fake_packets[1][2] = fake_incoming_eth_packet("000003000000", "001234567890");
    fake_packets[1][3] = fake_incoming_eth_packet("000004000000", "001234567890");
}

int launch_dpdk()
{
    printf("Creating fake packets for test...\n");
    init_fake_packets();
    printf("Faked packets created.\n");
    printf("Executing packet handlers on each core...\n");
	rte_eal_mp_remote_launch(launch_one_lcore, NULL, CALL_MASTER);
	unsigned lcore_id;
	RTE_LCORE_FOREACH_SLAVE(lcore_id)
		if (rte_eal_wait_lcore(lcore_id) < 0)
			return -1;
	return 0;
}
