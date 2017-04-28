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

#define BURST_TX_DRAIN_US		100 /* TX drain every ~100us */

// note: this much space MUST be able to hold all deparsed content
#define DEPARSE_BUFFER_SIZE		1024

extern int odp_dev_name_to_id (char *if_name);
//============================================================================

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

/* send one pkt each time */
static void odp_send_packet(odp_packet_t *p, uint8_t port, int thr_idx)
{
	odp_packet_t pkt = *p;
    unsigned buf_id;
	info("Inside odp_send_packet \n");
	buf_id = gconf->mconf[thr_idx].tx_pktios[port].buf.len;

	gconf->mconf[thr_idx].tx_pktios[port].buf.pkt[buf_id] = pkt;
	gconf->mconf[thr_idx].tx_pktios[port].buf.len++;
}

#define EXTRACT_EGRESSPORT(p) GET_INT32_AUTO(p, field_instance_standard_metadata_egress_port)

#define EXTRACT_INGRESSPORT(p) GET_INT32_AUTO(p, field_instance_standard_metadata_ingress_port)

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
			//			odp_send_packet((odp_packet_t *)pd->packet, port);
			first = 0;
		} else {
			odp_packet_t pkt_cp;

			pkt_cp = odp_packet_copy(pkt, gconf->pool);
			if (pkt_cp == ODP_PACKET_INVALID) {
				debug("Error: packet copy failed fo sending %s \n", __FILE__);
				continue;
			}
			info("Bcast - i/p %d, o/p port id %d\n", ingress_port, port);
			//	odp_send_packet(&pkt_cp, port);
			gconf->mconf[thr_idx].tx_pktios[port].buf.pkt[buf_len] = pkt_cp;
		}
		gconf->mconf[thr_idx].tx_pktios[port].buf.len++;
		sigg("[Broadcast] recvd port id - %d, sent port id - %d\n", ingress_port, port);
	}

	odp_packet_free (pkt);
	return;
}

static void init_metadata(packet_descriptor_t* packet_desc, uint32_t inport)
{
    info("Entry init_metadata \n ");
	int res32; // needs for the macro
	MODIFY_INT32_INT32_BITS(packet_desc, field_instance_standard_metadata_ingress_port, inport);

	/* validate ingressport set */
#if 0 //To test this function
	res32 = 0;
	int inp = EXTRACT_INGRESSPORT(packet_desc);
	info("[initMetadata] Inport %d is set as %d\n",inport, inp);
#endif
}

static inline int send_packet(packet_descriptor_t* pd, int thr_idx)
{
	int port = EXTRACT_EGRESSPORT(pd);
	int inport = EXTRACT_INGRESSPORT(pd);

//	dbg_print_headers(pd);
	info("send_packet: initial i/p port=%d and o/p port=%d \n", inport, port);
    reset_headers(pd);// sugar@292

	if (port==100) {
		maco_bcast_packet(pd, inport, thr_idx);
	} else {
		odp_send_packet((odp_packet_t *)pd->wrapper, port, thr_idx);
		sigg("[Unicast] recvd port id - %d, sent port id - %d\n", inport, port);
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

	num_pktio = mconf->num_rx_pktio;
    pktin     = mconf->rx_pktios[idx].pktin;
    port_in  = mconf->rx_pktios[idx].rx_idx;

	init_dataplane(&pd, gconf->state.tables);

	/* Loop packets */
    while (!exit_threads) {
        int sent;
        unsigned drops;
        if (num_pktio > 1) {
            pktin   = mconf->rx_pktios[idx].pktin;
            port_in = mconf->rx_pktios[idx].rx_idx;
            idx++;
            if (idx == num_pktio)
                idx = 0;
        }

		pkts = odp_pktin_recv(pktin, pkt_tbl, MAX_PKT_BURST);
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
			packet_received(&pd, &pkt, port_in, mconf->thr_idx);
			send_packet (&pd, mconf->thr_idx);
		}
        /* Empty all thread local tx buffers */
        for (port_out = 0; port_out < gconf->appl.if_count;
				port_out++) {
			info("	Start emptying Tx buffer for port=%d\n",port_out);
			unsigned tx_pkts;
            odp_packet_t *tx_pkt_tbl;

            if (port_out == port_in ||
                mconf->tx_pktios[port_out].buf.len == 0)
                continue;

            tx_pkts = mconf->tx_pktios[port_out].buf.len;
            mconf->tx_pktios[port_out].buf.len = 0;

            tx_pkt_tbl = mconf->tx_pktios[port_out].buf.pkt;

            pktout = mconf->tx_pktios[port_out].pktout;

			sent = odp_pktout_send(pktout, tx_pkt_tbl, tx_pkts);
            sent = (sent < 0) ? 0 : sent;
            drops = tx_pkts - sent;
            if (odp_unlikely(drops)) {
                /* Drop rejected packets */
	odp_packet_free_multi(&tx_pkt_tbl[sent], drops);
#if 0
				unsigned i;
                for (i = sent; i < tx_pkts; i++)
                    odp_packet_free(tx_pkt_tbl[i]);
#endif
			}
			info("	Finish emptying Tx buffer for port=%d\n",port_out);
        }

			info("	Finish emptying all Tx buffers for thread %d\n",mconf->thr_idx);
#endif
	}

    /* Make sure that latest stat writes are visible to other threads */
	return 0;
}
