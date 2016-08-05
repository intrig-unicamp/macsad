#include <odp_api.h>
#include "odp_lib.h"
#include <odp/api/packet.h>
#include <odp/helper/linux.h>
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

#define EXTRACT_EGRESSPORT(p) (*(uint32_t *)(((uint8_t*)(p)->headers[/*header instance id - hopefully it's the very first one*/0].pointer)+/*byteoffset*/6) & /*mask*/0x7fc) >> /*bitoffset*/2
#define EXTRACT_INGRESSPORT(p) (*(uint32_t *)(((uint8_t*)(p)->headers[/*header instance id - hopefully it's the very first one*/0].pointer)+/*byteoffset*/0) & /*mask*/0x1ff) >> /*bitoffset*/0

static void maco_bcast_packet(packet_descriptor_t* pd, uint8_t ingress_port, int thr_idx)
{
	appl_args_t *appl = &gconf->appl;
	uint32_t total_ports= appl->if_count;
	int port;
	unsigned buf_len;
	odp_bool_t first = 1;
	odp_packet_t pkt = *((odp_packet_t *)pd->packet);

	odp_packet_t pkt_cp_tmp;
	pkt_cp_tmp = odp_packet_copy(*((odp_packet_t *)pd->packet), gconf->pool);
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
			gconf->mconf[thr_idx].tx_pktios[port].buf.pkt[buf_len] = pkt;
			info("Bcast - i/p %d, o/p port id %d\n", ingress_port, port);
			//			odp_send_packet((odp_packet_t *)pd->packet, port);
			first = 0;
		} else {
			odp_packet_t pkt_cp;

			pkt_cp = odp_packet_copy(pkt_cp_tmp, gconf->pool);
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

//	odp_packet_free (pkt_cp_tmp);
	return;
}

static void init_metadata(packet_descriptor_t* packet_desc, uint32_t inport)
{
	packet_desc->headers[header_instance_standard_metadata] =
		(header_descriptor_t) {
			.type = header_instance_standard_metadata,
			.length = header_instance_byte_width[header_instance_standard_metadata],
			.pointer = calloc(header_instance_byte_width[header_instance_standard_metadata], sizeof(uint8_t))
		};

	int res32; // needs for the macro
	MODIFY_INT32_INT32(packet_desc, field_instance_standard_metadata_ingress_port, inport);
}

static inline int send_packet(packet_descriptor_t* pd, int thr_idx)
{
	int port = EXTRACT_EGRESSPORT(pd);
	int inport = EXTRACT_INGRESSPORT(pd);

	dbg_print_headers(pd);
	info("send_packet: i/p port=%d and o/p port=%d \n", inport, port);

	if (port==100) {
		maco_bcast_packet(pd, inport, thr_idx);
	} else {
		odp_send_packet((odp_packet_t *)pd->packet, port, thr_idx);
		sigg("[Unicast] recvd port id - %d, sent port id - %d\n", inport, port);
	}

	return 0;
}

void packet_received(packet_descriptor_t *pd, odp_packet_t *p, unsigned portid, int thr_idx)
{
	struct lcore_state *state_tmp = &gconf->state;
	pd->pointer = (uint8_t *)odp_packet_data(*p);
	pd->packet = (packet *)p;
    info("Inside packet received \n ");
//	set_metadata_inport(pd, portid);
	init_metadata(pd, portid);
	handle_packet(pd, state_tmp->tables);
}


static inline int event_queue_send(odp_queue_t queue, odp_packet_t *pkt_tbl,
		unsigned pkts)
{
	int ret;
	unsigned i;
	unsigned sent = 0;
	odp_event_t ev_tbl[pkts];

	for (i = 0; i < pkts; i++)
		ev_tbl[i] = odp_packet_to_event(pkt_tbl[i]);

	while (sent < pkts) {
		ret = odp_queue_enq_multi(queue, &ev_tbl[sent], pkts - sent);

		if (ret < 0) {
			debug("Failed to send packet as events\n");
			break;
		}

		sent += ret;
	}

	return sent;
}

/**
 * Packet IO worker thread using scheduled queues
 *
 * @param arg  thread arguments of type 'macs_conf_t *'
 */
int odpc_worker_mode_sched (void *arg)
{
    odp_event_t  ev_tbl[MAX_PKT_BURST];
    odp_packet_t pkt_tbl[MAX_PKT_BURST];
    int pkts;
    int thr;
    uint64_t wait;
    int port_in, port_out;
    int thr_idx;
    int i;
    odp_pktout_queue_t pktout[MAX_PKTIOS];
    odp_queue_t tx_queue[MAX_PKTIOS];
    macs_conf_t *mconf = arg;
    packet_descriptor_t pd;
    odp_packet_t pkt;
    int use_event_queue = gconf->appl.out_mode;
    pktin_mode_t in_mode = gconf->appl.in_mode;
	unsigned drops;
	int idx = 0;

    thr = odp_thread_id();
    thr_idx = mconf->thr_idx;
    port_in  = mconf->rx_pktios[idx].rx_idx;

    memset(pktout, 0, sizeof(pktout));

    for (i = 0; i < MAX_PKTIOS; i++)
        tx_queue[i] = ODP_QUEUE_INVALID;

    for (i = 0; i < gconf->appl.if_count; i++) {
        if (gconf->pktios[i].num_tx_queue ==
            gconf->appl.num_workers) {
            pktout[i]   = gconf->pktios[i].pktout[thr_idx];
            tx_queue[i] = gconf->pktios[i].tx_q[thr_idx];
        } else if (gconf->pktios[i].num_tx_queue == 1) {
            pktout[i]   = gconf->pktios[i].pktout[0];
            tx_queue[i] = gconf->pktios[i].tx_q[0];
        } else {
            debug("Bad number of output queues %i\n", i);
        }
    }

    printf("[%02i] PKTIN_SCHED_%s, %s\n", thr,
           (in_mode == SCHED_PARALLEL) ? "PARALLEL" :
           ((in_mode == SCHED_ATOMIC) ? "ATOMIC" : "ORDERED"),
           (use_event_queue) ? "PKTOUT_QUEUE" : "PKTOUT_DIRECT");

    odp_barrier_wait(&barrier);

    wait = odp_schedule_wait_time(ODP_TIME_MSEC_IN_NS * 100);

    /* Loop packets */
    while (!exit_threads) {
        int sent;

        pkts = odp_schedule_multi(NULL, wait, ev_tbl, MAX_PKT_BURST);

        if (pkts <= 0)
            continue;

        for (i = 0; i < pkts; i++)
            pkt_tbl[i] = odp_packet_from_event(ev_tbl[i]);

    info("  pkt recv %d \n",pkts);
        mconf->stats[port_in]->s.rx_packets += pkts;

        for (i = 0; i < pkts; i++) {
            pkt = pkt_tbl[i];
            if (!odp_packet_has_eth(pkt)) {
                odp_packet_free(pkt);
                continue;
            }
            packet_received(&pd, &pkt, port_in, mconf->thr_idx);
        //  mconf = &(gconf->mconf[port_in]);
            send_packet (&pd, mconf->thr_idx);
        }

		/* Empty all thread local tx buffers */
		for (port_out = 0; port_out < gconf->appl.if_count;
				port_out++) {
			info("  Start emptying Tx buffer for port=%d\n",port_out);
			unsigned tx_pkts;
			odp_packet_t *tx_pkt_tbl;

			if (port_out == port_in ||
					mconf->tx_pktios[port_out].buf.len == 0)
				continue;

			tx_pkts = mconf->tx_pktios[port_out].buf.len;
			mconf->tx_pktios[port_out].buf.len = 0;

			tx_pkt_tbl = mconf->tx_pktios[port_out].buf.pkt;

			pktout[port_out] = mconf->tx_pktios[port_out].pktout;

			if (odp_unlikely(use_event_queue))
				sent = event_queue_send(tx_queue[port_out], tx_pkt_tbl, tx_pkts);
			else
				sent = odp_pktout_send(pktout[port_out], tx_pkt_tbl, tx_pkts);

			sent = odp_unlikely(sent < 0) ? 0 : sent;

			mconf->stats[port_out]->s.tx_packets += sent;

			drops = tx_pkts - sent;

			if (odp_unlikely(drops)) {
				unsigned i;

				mconf->stats[port_out]->s.tx_drops += drops;

				/* Drop rejected packets */
				for (i = sent; i < tx_pkts; i++)
					odp_packet_free(tx_pkt_tbl[i]);
			}
			info("  Finish emptying Tx buffer for port=%d\n",port_out);
		}
		info("  Finish emptying all Tx buffers for thread %d\n",mconf->thr_idx);
	}

    /* Make sure that latest stat writes are visible to other threads */
    odp_mb_full();
    return 0;
}

/**
 * Packet IO worker thread using plain queues
 *
 * @param arg  thread arguments of type 'macs_conf_t *'
 */
int odpc_worker_mode_queue(void *arg)
{
    int pkts;
    int port_in, num_pktio, port_out;
    macs_conf_t *mconf = arg;
    odp_pktin_queue_t pktin;
    odp_pktout_queue_t pktout;
    odp_packet_t pkt_tbl[MAX_PKT_BURST];
    odp_queue_t tx_queue;
	odp_queue_t queue;
    int idx = 0;
    packet_descriptor_t pd;
    odp_packet_t pkt;
//  init_dataplane(&pd, gconf->state.tables);
    int use_event_queue = gconf->appl.out_mode;

    info(":: INSIDE odp_main_worker\n");
//  thr = odp_thread_id();
//  info("  the thread id is %d\n",thr);

    num_pktio = mconf->num_rx_pktio;
    pktin     = mconf->rx_pktios[idx].pktin;
    port_in  = mconf->rx_pktios[idx].rx_idx;

    info("  before barrier %d, exit thread %d \n",mconf->thr_idx, exit_threads);
    odp_barrier_wait(&barrier);

    /* Loop packets */
    while (!exit_threads) {
        int sent;
        unsigned drops;
        odp_event_t event[MAX_PKT_BURST];
        int i;

		if (num_pktio > 1) {
			queue     = mconf->rx_pktios[idx].rx_queue;
			//   pktout    = mconf->pktio[pktio].pktout;
			idx++;
			if (idx == num_pktio)
				idx = 0;
		}
		pkts = odp_queue_deq_multi(queue, event, MAX_PKT_BURST);
		if (odp_unlikely(pkts <= 0))
			continue;

		for (i = 0; i < pkts; i++)
			pkt_tbl[i] = odp_packet_from_event(event[i]);

		mconf->stats[port_in]->s.rx_packets += pkts;

		for (i = 0; i < pkts; i++) {
			pkt = pkt_tbl[i];
			if (!odp_packet_has_eth(pkt)) {
				odp_packet_free(pkt);
				continue;
			}
			packet_received(&pd, &pkt, port_in, mconf->thr_idx);
			//  mconf = &(gconf->mconf[port_in]);
			send_packet (&pd, mconf->thr_idx);
		}

		for (port_out = 0; port_out < gconf->appl.if_count;
                port_out++) {
            info("  Start emptying Tx buffer for port=%d\n",port_out);
            unsigned tx_pkts;
            odp_packet_t *tx_pkt_tbl;

            if (port_out == port_in ||
                mconf->tx_pktios[port_out].buf.len == 0)
                continue;

            tx_pkts = mconf->tx_pktios[port_out].buf.len;
            mconf->tx_pktios[port_out].buf.len = 0;

            tx_pkt_tbl = mconf->tx_pktios[port_out].buf.pkt;

            pktout = mconf->tx_pktios[port_out].pktout;

        if (odp_unlikely(use_event_queue))
		{
			tx_queue = mconf->tx_pktios[port_out].tx_queue;
            sent = event_queue_send(tx_queue, tx_pkt_tbl, tx_pkts);
		}
	   	else
		{
            sent = odp_pktout_send(pktout, tx_pkt_tbl, tx_pkts);
		}

            sent = odp_unlikely(sent < 0) ? 0 : sent;

            mconf->stats[port_out]->s.tx_packets += sent;

            drops = tx_pkts - sent;

            if (odp_unlikely(drops)) {
                unsigned i;

                mconf->stats[port_out]->s.tx_drops += drops;

                /* Drop rejected packets */
                for (i = sent; i < tx_pkts; i++)
                    odp_packet_free(tx_pkt_tbl[i]);
            }
            info("  Finish emptying Tx buffer for port=%d\n",port_out);
        }
            info("  Finish emptying all Tx buffers for thread %d\n",mconf->thr_idx);
    }

    /* Make sure that latest stat writes are visible to other threads */
    odp_mb_full();
    return 0;
}

/**
 * Switch worker thread
 * @param arg  Thread arguments of type 'macs_conf_t *'
 */
int odpc_worker_mode_direct(void *arg)
{
	int pkts, i;
	int port_in, num_pktio, port_out;
	macs_conf_t *mconf = arg;
	odp_pktin_queue_t pktin;
	odp_pktout_queue_t pktout;
	odp_packet_t pkt_tbl[MAX_PKT_BURST];
	odp_queue_t tx_queue;
	int idx = 0;
	packet_descriptor_t pd;
	odp_packet_t pkt;
//	init_dataplane(&pd, gconf->state.tables);
    int use_event_queue = gconf->appl.out_mode;

	info(":: INSIDE odp_main_worker\n");

    num_pktio = mconf->num_rx_pktio;
    pktin     = mconf->rx_pktios[idx].pktin;
    port_in  = mconf->rx_pktios[idx].rx_idx;

	info("	before barrier %d, exit thread %d \n",mconf->thr_idx, exit_threads);
    odp_barrier_wait(&barrier);

	info("	after barrier \n");
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
//	info("	pkt recv %d \n",pkts);
		if (odp_unlikely(pkts <= 0))
			continue;
	info("	pkt recv %d \n",pkts);
		mconf->stats[port_in]->s.rx_packets += pkts;

		for (i = 0; i < pkts; i++) {
			pkt = pkt_tbl[i];
			if (!odp_packet_has_eth(pkt)) {
				odp_packet_free(pkt);
				continue;
			}
			packet_received(&pd, &pkt, port_in, mconf->thr_idx);
		//	mconf = &(gconf->mconf[port_in]);
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

			if (odp_unlikely(use_event_queue))
			{
				tx_queue = mconf->tx_pktios[port_out].tx_queue;
				sent = event_queue_send(tx_queue, tx_pkt_tbl, tx_pkts);
			}
			else
			{
				sent = odp_pktout_send(pktout, tx_pkt_tbl, tx_pkts);
			}
            sent = odp_unlikely(sent < 0) ? 0 : sent;

            mconf->stats[port_out]->s.tx_packets += sent;

            drops = tx_pkts - sent;

            if (odp_unlikely(drops)) {
                unsigned i;

                mconf->stats[port_out]->s.tx_drops += drops;

                /* Drop rejected packets */
                for (i = sent; i < tx_pkts; i++)
                    odp_packet_free(tx_pkt_tbl[i]);
            }
			info("	Finish emptying Tx buffer for port=%d\n",port_out);
        }
			info("	Finish emptying all Tx buffers for thread %d\n",mconf->thr_idx);
	}

    /* Make sure that latest stat writes are visible to other threads */
    odp_mb_full();
	return 0;
}
