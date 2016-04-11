#include <odp_api.h>
#include "odp_lib.h"
#include <odp/api/packet.h>
#include <odp/helper/linux.h>
#include <odp/helper/eth.h>
#include <odp/helper/ip.h>

//extern void p4_handle_packet(packet* p, unsigned portid);

//=   shared   ================================================================

uint32_t enabled_port_mask = 0;
struct ether_addr ports_eth_addr[MAX_ETHPORTS];
//typedef struct odp_packet_t packet;
//=   used only here   ========================================================

/* A tsc-based timer responsible for triggering statistics printout */
#define TIMER_MILLISECOND 2000000ULL /* around 1ms at 2 Ghz */
#define MAX_TIMER_PERIOD 86400 /* 1 day max */
int64_t timer_period = 10 * TIMER_MILLISECOND * 1000; /* default period is 10 seconds */

#define MAX_PORTS 16

#define MCAST_CLONE_PORTS       2
#define MCAST_CLONE_SEGS        2

#define RTE_TEST_RX_DESC_DEFAULT 128
#define RTE_TEST_TX_DESC_DEFAULT 512
uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;

#define PKT_MBUF_DATA_SIZE      RTE_MBUF_DEFAULT_BUF_SIZE
#define NB_PKT_MBUF     8192

#define HDR_MBUF_DATA_SIZE      (2 * RTE_PKTMBUF_HEADROOM)
#define NB_HDR_MBUF     (NB_PKT_MBUF * MAX_PORTS)

#define NB_CLONE_MBUF   (NB_PKT_MBUF * MCAST_CLONE_PORTS * MCAST_CLONE_SEGS * 2)

#define BURST_TX_DRAIN_US 100 /* TX drain every ~100us */

// note: this much space MUST be able to hold all deparsed content
#define DEPARSE_BUFFER_SIZE 1024

//=============================================================================

/* Send burst of packets on an output interface */
static inline int
send_burst(struct lcore_conf *qconf, uint16_t n, uint8_t port)
{
/*
	struct rte_mbuf **m_table;
	int ret;
	uint16_t queueid;

	queueid = qconf->tx_queue_id[port];
	m_table = (struct rte_mbuf **)qconf->tx_mbufs[port].m_table;

	ret = rte_eth_tx_burst(port, queueid, m_table, n);
	if (unlikely(ret < n)) {
		do {
			rte_pktmbuf_free(m_table[ret]);
		} while (++ret < n);
	}
*/
	return 0;
}

/* Send burst of outgoing packet, if timeout expires. */
static inline void
send_timeout_burst(struct lcore_conf *qconf)
{
/* 
       uint64_t cur_tsc;
        uint8_t portid;
        const uint64_t drain_tsc = (rte_get_tsc_hz() + US_PER_S - 1) / US_PER_S * BURST_TX_DRAIN_US;

        cur_tsc = rte_rdtsc();
        if (likely (cur_tsc < qconf->tx_tsc + drain_tsc))
                return;

        for (portid = 0; portid < MAX_PORTS; portid++) {
                if (qconf->tx_mbufs[portid].len != 0) {
                        send_burst(qconf, qconf->tx_mbufs[portid].len, portid);
			qconf->tx_mbufs[portid].len = 0; 
		}
        }
        qconf->tx_tsc = cur_tsc;
*/
}

static int
get_socketid(unsigned lcore_id)
{
/*
    if (numa_on)
        return rte_lcore_to_socket_id(lcore_id);
    else
        return 0;
*/ 
return 0;
}

static inline void
dbg_print_headers(packet_descriptor_t* pd)
{
    for (int i = 0; i < HEADER_INSTANCE_COUNT; ++i) {
        printf("    :: header %d (type=%d, len=%d) = ", i, pd->headers[i].type, pd->headers[i].length);
        for (int j = 0; j < pd->headers[i].length; ++j) {
            printf("%02x ", ((uint8_t*)(pd->headers[i].pointer))[j]);
        }
        printf("\n");
    }
}

static inline unsigned
deparse_headers(packet_descriptor_t* pd, int socketid)
{
/*
	uint8_t* deparse_buffer = (uint8_t*)rte_pktmbuf_append(deparse_mbuf, DEPARSE_BUFFER_SIZE);
	int len = 0;
    for (int i = 0; i < HEADER_INSTANCE_COUNT; ++i) {
    	uint8_t* hdr_ptr = (uint8_t*)(pd->headers[i].pointer);
    	unsigned hdr_len = pd->headers[i].length;
        for (int j = 0; j < hdr_len; ++j) {
            *deparse_buffer = *hdr_ptr;
            ++deparse_buffer;
            ++hdr_ptr;
        }
        len += hdr_len;
    }
    return len;
*/
    return -1;
}

/* Get number of bits set. */
static inline uint32_t
bitcnt(uint32_t v)
{
        uint32_t n;

        for (n = 0; v != 0; v &= v - 1, n++)
                ;

        return (n);
}


/* send one pkt each time */
static void
opd_send_packet(struct odp_packet_t *p, uint8_t port)
{
	int sent;
//	struct odp_packet_t pkt = *p;
	struct macs_conf *macs = &mconf_list[0];;

	sent = odp_pktout_send(macs->if1out, p, 1);
//	sent = odp_pktout_send(gconf.if1out, p, 1);
	if (sent < 0)
	{
		printf("pkt sent failed \n");
		sent = 0;
	}
}

#define EXTRACT_EGRESSPORT(p) (*(uint32_t *)(((uint8_t*)(p)->headers[/*header instance id - hopefully it's the very first one*/0].pointer)+/*byteoffset*/6) & /*mask*/0x7fc) >> /*bitoffset*/2
#define EXTRACT_INGRESSPORT(p) (*(uint32_t *)(((uint8_t*)(p)->headers[/*header instance id - hopefully it's the very first one*/0].pointer)+/*byteoffset*/0) & /*mask*/0x1ff) >> /*bitoffset*/0

/* Enqueue a single packet, and send burst if queue is filled */
static inline int
send_packet(packet_descriptor_t* pd)
{
    int port = EXTRACT_EGRESSPORT(pd);
    int inport = EXTRACT_INGRESSPORT(pd);

    printf("  :::: EGRESSING\n");
    dbg_print_headers(pd);
    printf("    :: deparsing headers\n");
    printf("    :: sendPacket called\n");
    printf("ingress port is %d and egress port is %d \n", inport, port);

	//port = -1;
/*
	if (port==100)
		dpdk_bcast_packet((struct rte_mbuf *)pd->packet, inport, lcore_id);
	else
		dpdk_send_packet((struct rte_mbuf *)pd->packet, port, lcore_id);
*/
  opd_send_packet((struct odp_packet_t *)pd->packet, port);
	return 0;
}

//remove this after enabling odp_primitives file
void
modify_field_to_const(packet_descriptor_t* p, field_reference_t f, uint8_t *src, int srclen)
{
#if 0
    if(f.bytewidth <= 4) {
        uint32_t src_val = 0;
        memcpy(&src_val, src, srclen);
        modify_field_to_const32(p, f, src_val);
    }
    else if(f.bytewidth == srclen)
        memcpy(FIELD_BYTE_ADDR(p, f), src, srclen);
    //else
    //    TODO
#endif
} 

static void init_metadata(packet_descriptor_t* packet_desc, uint32_t inport)
{
    packet_desc->headers[header_instance_standard_metadata] =
      (header_descriptor_t) {
        .type = header_instance_standard_metadata,
        .length = header_instance_byte_width[header_instance_standard_metadata],
        .pointer = calloc(header_instance_byte_width[header_instance_standard_metadata], sizeof(uint8_t))
      };
    modify_field_to_const(packet_desc, field_desc(field_instance_standard_metadata_ingress_port), (uint8_t*)&inport, 2);
}

void packet_received(odp_packet_t *p, unsigned portid)
{
	struct macs_conf *macs = &mconf_list[0];;
	printf(":::: EXECUTING packet recieved\n");
	packet_descriptor_t packet_desc;
	packet_desc.pointer = (uint8_t *)odp_packet_data(*p);
	packet_desc.packet = (packet *)p;

	init_metadata(&packet_desc, portid);
	struct lcore_state *state_tmp = &macs->state;
	handle_packet(&packet_desc, state_tmp->tables);
//	handle_packet(&packet_desc, &state->tables);
	send_packet(&packet_desc);
}

void odp_main_worker (void)
{
        odp_packet_t pkt_tbl[MAX_PKT_BURST];
        int pkts, i;
	int portid;
	struct macs_conf *macs = &mconf_list[0];;

        if (odp_pktio_start(macs->if0)) {
                printf("unable to start input interface\n");
                exit(1);
        }
        printf("started input interface\n");

        if (odp_pktio_start(macs->if1)) {
                printf("unable to start output interface\n");
                exit(1);
        }
        printf("started output interface\n");
        printf("started all\n");

        for (;;) {
/* Use timeout version of the pktin recv call to reduce CPU
load when there are no packets available. 
		pkts = odp_pktin_recv_tmo(global.if0in, pkt_tbl, MAX_PKT_BURST,
					  ODP_PKTIN_WAIT);
*/
		pkts = odp_pktin_recv(macs->if0in, pkt_tbl, MAX_PKT_BURST);
                if (odp_unlikely(pkts <= 0))
                        continue;
                for (i = 0; i < pkts; i++) {
                        odp_packet_t pkt = pkt_tbl[i];
/*                        odph_ethhdr_t *eth;

                        if (odp_unlikely(!odp_packet_has_eth(pkt))) {
                                printf("warning: packet has no eth header\n");
                                return NULL;
                        }

                        eth = (odph_ethhdr_t *)odp_packet_l2_ptr(pkt, NULL);
                        eth->src = global.src;
                        eth->dst = global.dst;
*/
			packet_received(&pkt, portid=0);
                }
/*
                sent = odp_pktout_send(global.if1out, pkt_tbl, pkts);
                if (sent < 0)
                        sent = 0;
                tx_drops = pkts - sent;
                if (odp_unlikely(tx_drops))
                        odp_packet_free_multi(&pkt_tbl[sent], tx_drops);
*/
        }
        return;
}

