#include <odp_api.h>
#include "odp_lib.h"
#include <odp/api/packet.h>
#include <odp/helper/eth.h>
#include <odp/helper/ip.h>

uint32_t enabled_port_mask = 0;
struct ether_addr ports_eth_addr[MAX_ETHPORTS];

/* A tsc-based timer responsible for triggering statistics printout */
#define TIMER_MILLISECOND 2000000ULL /* around 1ms at 2 Ghz */
#define MAX_TIMER_PERIOD  86400 /* 1 day max */
int64_t timer_period = 10 * TIMER_MILLISECOND * 1000; /* default period is 10 seconds */
bool exit_threads = 0;
#define MAX_PORTS 16

#define MCAST_CLONE_PORTS       2
#define MCAST_CLONE_SEGS        2

#define RTE_TEST_RX_DESC_DEFAULT 128
#define RTE_TEST_TX_DESC_DEFAULT 512

#define PKT_MBUF_DATA_SIZE      RTE_MBUF_DEFAULT_BUF_SIZE
#define NB_PKT_MBUF				8192

#define HDR_MBUF_DATA_SIZE      (2 * RTE_PKTMBUF_HEADROOM)
#define NB_HDR_MBUF				(NB_PKT_MBUF * MAX_PORTS)

#define NB_CLONE_MBUF		    (NB_PKT_MBUF * MCAST_CLONE_PORTS * MCAST_CLONE_SEGS * 2)


#define NS_PER_US 1000
#define US_PER_MS 1000
#define MS_PER_S 1000
#define US_PER_S (US_PER_MS * MS_PER_S)

#define BURST_TX_DRAIN_US		100 /* TX drain every ~100us */

// note: this much space MUST be able to hold all deparsed content
#define DEPARSE_BUFFER_SIZE		1024

extern int odp_dev_name_to_id (char *if_name);
//============================================================================

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
	char buf[100];
	int len = 0;
//	for (int i = 0; i < HEADER_INSTANCE_COUNT; ++i) {
	for (int i = 1; i < HEADER_INSTANCE_COUNT; ++i) {
		info("    :: header %d (type=%d, len=%d) = ", i, pd->headers[i].type, pd->headers[i].length);
		for (int j = 0; j < pd->headers[i].length; ++j) {
			if (len < 100) {
				//	info("%02x ", ((uint8_t*)(pd->headers[i].pointer))[j]);
				len += sprintf(buf+len, "%02x ", ((uint8_t*)(pd->headers[i].pointer))[j]);
			}
		}
		info ("%s \n", buf);
	}
#if 0
Try printing the hdeader  by specifying the length :   '.*' in the  printf statement
printf("%.*s",len,buf);
#endif
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
static inline uint32_t bitcnt(uint32_t v)
{
	uint32_t n;

	for (n = 0; v != 0; v &= v - 1, n++)
		;

	return (n);
}

/* Send burst of packets on an output interface */
static inline int maco_send_burst(uint16_t n, uint8_t port, int thr_idx)
{
	odp_packet_t *tx_pkt_tbl;
	uint16_t sent, drops;
	odp_pktout_queue_t pktout;
	tx_pkt_tbl = gconf->mconf[thr_idx].tx_pktios[port].buf.pkt;
	pktout = gconf->mconf[thr_idx].tx_pktios[port].pktout;
	sent = odp_pktout_send(pktout, tx_pkt_tbl, n);
	sent = (sent < 0) ? 0 : sent;
	drops = n - sent;
	if (odp_unlikely(drops)) {
		printf(" maco_send drop %d pkts \n", drops);
		/* Drop rejected packets */
		odp_packet_free_multi(&tx_pkt_tbl[sent], drops);
	}
	return 0;
}

#if 0
/* send one pkt each time */
static void odp_send_packet(odp_packet_t *p, uint8_t port, int thr_idx)
{
    pkt_buf_t *buf = &(gconf->mconf[thr_idx].tx_pktios[port].buf);
	uint16_t buf_id = buf->len;

	buf->pkt[buf_id] = *p;
	buf->len++;

	buf_id++;
	if (odp_unlikely(buf_id == MAX_PKT_BURST))
	{
		debug("Tx q full(%d), burst sending pkt \n", buf_id);
		maco_send_burst (buf_id, port, thr_idx);
		buf_id = 0;
	}
	buf->len = buf_id;
	return;
}
#endif

//#define EXTRACT_EGRESSPORT(p) GET_INT32_AUTO(p, field_instance_standard_metadata_egress_port)
#define EXTRACT_EGRESSPORT(p)  GET_INT32_AUTO_PACKET(p, header_instance_standard_metadata, field_standard_metadata_t_egress_port)

#define EXTRACT_INGRESSPORT(p) GET_INT32_AUTO(p, field_instance_standard_metadata_ingress_port)

//TODO use odp_packet_ref_static() for performance improvement. (ref_static creates readonly copy. After reference creation no pkt manipuation allowed
static void maco_bcast_packet(packet_descriptor_t* pd, uint8_t ingress_port, int thr_idx)
{
	appl_args_t *appl = &gconf->appl;
	uint32_t total_ports= appl->if_count;
	int port;
	unsigned buf_len;
	odp_bool_t first = 1;
	odp_packet_t pkt = *((odp_packet_t *)pd->wrapper);

	odp_packet_t pkt_cp_tmp;
	pkt_cp_tmp = odp_packet_copy(*((odp_packet_t *)pd->wrapper), gconf->pool);
	if (pkt_cp_tmp == ODP_PACKET_INVALID) {
		debug("Error: Tmp packet copy failed for sending %s \n", __FILE__);
		return;
	}
	info ("pkt cpy passed\n");

	for (port = 0; port < total_ports; port++) {
		if (port == ingress_port)
			continue;
		buf_len = gconf->mconf[thr_idx].tx_pktios[port].buf.len;
		if (first) {
			gconf->mconf[thr_idx].tx_pktios[port].buf.pkt[buf_len] = pkt_cp_tmp;
			info("Bcast - i/p %d, o/p port id %d\n", ingress_port, port);
			first = 0;
		} else {
			odp_packet_t pkt_cp;

			pkt_cp = odp_packet_copy(pkt, gconf->pool);
			if (pkt_cp == ODP_PACKET_INVALID) {
				debug("Error: packet copy failed fo sending %s \n", __FILE__);
				continue;
			}
			info("Bcast - i/p %d, o/p port id %d\n", ingress_port, port);
			gconf->mconf[thr_idx].tx_pktios[port].buf.pkt[buf_len] = pkt_cp;
		}
		gconf->mconf[thr_idx].tx_pktios[port].buf.len++;
		sigg("[Broadcast] recvd port id - %d, sent port id - %d\n", ingress_port, port);
	}

	odp_packet_free (pkt);
	return;
}

#if 1
//TODO Will be commented in future. Before that need to update create_digest function to use inport from ODP api.
static void init_metadata(packet_descriptor_t* packet_desc, uint32_t inport)
{
    info("Entry init_metadata \n ");
	int res32; // needs for the macro
	MODIFY_INT32_INT32_BITS_PACKET(packet_desc, header_instance_standard_metadata, field_instance_standard_metadata_ingress_port, inport);
}
#endif

static inline int send_packet(packet_descriptor_t* pd, int thr_idx)
{
	int port = EXTRACT_EGRESSPORT(pd);

	info("send_packet: initial i/p port=%d and o/p port=%d \n", odp_packet_input_index(*((odp_packet_t *)pd->wrapper)), port);
	reset_headers(pd);

	if (odp_unlikely(port==100)) {
		int inport = odp_packet_input_index(*((odp_packet_t *)pd->wrapper));
		maco_bcast_packet(pd, inport, thr_idx);
	} else {
		pkt_buf_t *buf = &(gconf->mconf[thr_idx].tx_pktios[port].buf);
		uint16_t buf_id = buf->len;
		buf->pkt[buf_id] = *(odp_packet_t *)pd->wrapper;
		buf->len++;
		sigg("[Unicast] recvd port id - %d, sent port id - %d\n", odp_packet_input_index(*((odp_packet_t *)pd->wrapper)), port);
	}
	return 0;
}

void packet_received(packet_descriptor_t *pd, odp_packet_t *p, unsigned portid, int thr_idx)
{
	struct lcore_state *state_tmp = &gconf->state;
	//odp_packet_prefetch(*p, 0, 12);
	pd->pointer = (uint8_t *)odp_packet_data(*p);
	pd->wrapper = (packet *)p;
	info("Entry packet_received \n ");
	init_metadata(pd, portid);
	handle_packet(pd, state_tmp->tables);
}

/**
 * Switch worker thread
 * @param arg  Thread arguments of type 'macs_conf_t *'
 */
int odpc_worker_mode_direct(void *arg)
{
	macs_conf_t *mconf = arg;
	odp_pktin_queue_t pktin;
	odp_pktout_queue_t pktout;
	odp_packet_t pkt_tbl[MAX_PKT_BURST];
	odp_queue_t tx_queue;
	packet_descriptor_t pd;
	odp_packet_t pkt;
	int pkts, i;
	int port_in, num_pktio, port_out;
	int idx = 0;
	int portid;
	int thr_idx = mconf->thr_idx;

	//	uint64_t prev_tsc = 0, diff_tsc, cur_tsc;
	//	const uint64_t drain_tsc = (odp_time_local_res() + US_PER_S - 1) / US_PER_S * BURST_TX_DRAIN_US;

	num_pktio = mconf->num_rx_pktio;
	init_dataplane(&pd, gconf->state.tables);

	/* Loop packets */
	while (odp_likely(!exit_threads)) {
#if 0
		cur_tsc = odp_time_to_ns(odp_time_local());
		diff_tsc = cur_tsc - prev_tsc;
		if (odp_unlikely(diff_tsc > drain_tsc)) {
			for (portid = 0; portid < gconf->appl.if_count; portid++) {
				if (mconf->tx_pktios[portid].buf.len == 0)
					continue;
				maco_send_burst (mconf->tx_pktios[portid].buf.len, portid, thr_idx);
				mconf->tx_pktios[portid].buf.len = 0;
			}
			prev_tsc = cur_tsc;
		}
#endif

		int sent;
		unsigned drops;
		pktin   = mconf->rx_pktios[idx].pktin;
		port_in = mconf->rx_pktios[idx].rx_idx;
		idx++;
		if (idx == num_pktio)
			idx = 0;

		pkts = odp_pktin_recv_tmo(pktin, pkt_tbl, MAX_PKT_BURST, DEF_RX_PKT_TMO_US);
		if (odp_unlikely(pkts <= 0))
			continue;
#if 0
		if(port_in) {pktout = mconf->tx_pktios[0].pktout;
		}else {pktout = mconf->tx_pktios[1].pktout;}
		sent = odp_pktout_send(pktout, pkt_tbl, pkts);
		sent = (sent < 0) ? 0 : sent;
		drops = pkts - sent;
		if (odp_unlikely(drops)) {
			/* Drop rejected packets */
			odp_packet_free_multi(&pkt_tbl[sent], drops);
		}
#endif
#if 1
		for (i = 0; i < pkts; i++) {
			pkt = pkt_tbl[i];
			//odp_packet_print((odp_packet_t) pkt);
			packet_received(&pd, &pkt, port_in, thr_idx);
			send_packet (&pd, thr_idx);
		}

		for (portid = 0; portid < gconf->appl.if_count; portid++) {
			if (odp_unlikely((portid == port_in) || (mconf->tx_pktios[portid].buf.len == 0)))
				continue;
			maco_send_burst (mconf->tx_pktios[portid].buf.len, portid, thr_idx);
			mconf->tx_pktios[portid].buf.len = 0;
		}
#endif
	}

	/* Make sure that latest stat writes are visible to other threads */
	return 0;
}
