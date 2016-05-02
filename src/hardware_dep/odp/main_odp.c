#include <odp_api.h>
#include "odp_lib.h"
#include <odp/api/packet.h>
#include <odp/helper/linux.h>
#include <odp/helper/eth.h>
#include <odp/helper/ip.h>

//extern void p4_handle_packet(packet* p, unsigned portid);

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
uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;

#define PKT_MBUF_DATA_SIZE      RTE_MBUF_DEFAULT_BUF_SIZE
#define NB_PKT_MBUF				8192

#define HDR_MBUF_DATA_SIZE      (2 * RTE_PKTMBUF_HEADROOM)
#define NB_HDR_MBUF				(NB_PKT_MBUF * MAX_PORTS)

#define NB_CLONE_MBUF		    (NB_PKT_MBUF * MCAST_CLONE_PORTS * MCAST_CLONE_SEGS * 2)

#define BURST_TX_DRAIN_US		100 /* TX drain every ~100us */

// note: this much space MUST be able to hold all deparsed content
#define DEPARSE_BUFFER_SIZE		1024

/** Global barrier to synchronize main and workers */
static odp_barrier_t barrier;

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
	char buf[100];
	int len = 0;
	for (int i = 0; i < HEADER_INSTANCE_COUNT; ++i) {
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
#if 0
static void swap_dmac_addr(odp_packet_t pkt, unsigned port) {

	odph_ethhdr_t *eth;
	odph_ethaddr_t tmp_addr;
	odph_ipv4hdr_t *ip;
	odp_u32be_t ip_tmp_addr; /* tmp ip addr */
	unsigned i;

	if (odp_packet_has_eth(pkt)) {
		eth = (odph_ethhdr_t *)odp_packet_l2_ptr(pkt, NULL);

		tmp_addr = eth->dst;
		eth->dst = gconf->;
		eth->src = tmp_addr;
	}
}
#endif 

/* send one pkt each time */
static void odp_send_packet(odp_packet_t *p, uint8_t port)
{
	int sent;
	odp_packet_t pkt = *p;
//	swap_dmac_addr (p, port);

	sent = odp_pktout_send(gconf->pktios[port].pktout[0], pkt, 1);
	if (sent < 0)
	{
//		odp_packet_free(pkt);
		debug("pkt sent failed \n");
	}
}

#define EXTRACT_EGRESSPORT(p) (*(uint32_t *)(((uint8_t*)(p)->headers[/*header instance id - hopefully it's the very first one*/0].pointer)+/*byteoffset*/6) & /*mask*/0x7fc) >> /*bitoffset*/2
#define EXTRACT_INGRESSPORT(p) (*(uint32_t *)(((uint8_t*)(p)->headers[/*header instance id - hopefully it's the very first one*/0].pointer)+/*byteoffset*/0) & /*mask*/0x1ff) >> /*bitoffset*/0

static void maco_bcast_packet(packet_descriptor_t* pd, uint8_t ingress_port)
{
	appl_args_t *appl = &gconf->appl;
	uint32_t total_ports= appl->if_count;
	int port;
	odp_bool_t first = 1;

	odp_packet_t pkt_cp_tmp;

	pkt_cp_tmp = odp_packet_copy((odp_packet_t *)pd->packet, gconf->pool);
	if (pkt_cp_tmp == ODP_PACKET_INVALID) {
		debug("Error: Tmp packet copy failed for sending %s \n", __FILE__);
		return;
	}

	for (port = 0;port < total_ports;port++){

		if (port == ingress_port)
			continue;
		if (first) { 
			info("Bcast - i/p %d, o/p port id %d\n", ingress_port, port);
			odp_send_packet((odp_packet_t *)pd->packet, port);
			first = 0;
		} else {
			odp_packet_t pkt_cp;

			pkt_cp = odp_packet_copy(pkt_cp_tmp, gconf->pool);
			if (pkt_cp == ODP_PACKET_INVALID) {
				debug("Error: packet copy failed fo sending %s \n", __FILE__);
				continue;
			}
			info("Bcast - i/p %d, o/p port id %d\n", ingress_port, port);
			odp_send_packet(pkt_cp, port);
		}
	}
	
	odp_packet_free (pkt_cp_tmp);
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

int bcast_buf(packet_descriptor_t* pd, macs_conf_t* mconf, int port_in)
{
	odp_bool_t first = 1;
	uint8_t port_out;
	unsigned buf_len;

	for (port_out = 0; port_out < gconf->appl.if_count; port_out++) {
		if (port_out == port_in)
			continue;

		buf_len = mconf->pktio[port_out].buf.len;

		if (first) { /* No need to copy for the first interface */
			mconf->pktio[port_out].buf.pkt[buf_len] = pkt;
			first = 0;
		} else {
			odp_packet_t pkt_cp;

			pkt_cp = odp_packet_copy(pkt, gbl_args->pool);
			if (pkt_cp == ODP_PACKET_INVALID) {
				printf("Error: packet copy failed\n");
				continue;
			}
			mconf->pktio[port_out].buf.pkt[buf_len] = pkt_cp;
		}
		mconf->pktio[port_out].buf.len++;
	}
}

static inline int updt_send_packet_buf(packet_descriptor_t* pd, macs_conf_t* mconf, int if_idx)
{
    unsigned buf_id;
	int port = EXTRACT_EGRESSPORT(pd);
	int inport = EXTRACT_INGRESSPORT(pd);

	info("ingress port is %d and egress port is %d \n", inport, port);
	

	if (port==100) {
	//	maco_bcast_packet(pd, inport);
	bcast_buf (pd, mconf, if_idx, port);
	} else {
		/* o/p queue, pkt, no. of pkt to send */
	//	odp_send_packet((odp_packet_t *)pd->packet, port);
	buf_id = mconf->pktio[port].buf.len;
	mconf->pktio[port].buf.pkt[buf_id] = pkt;
	mconf->pktio[port].buf.len++;
	}

}

static inline int send_packet(packet_descriptor_t* pd)
{
	int port = EXTRACT_EGRESSPORT(pd);
	int inport = EXTRACT_INGRESSPORT(pd);

	dbg_print_headers(pd);
	info("ingress port is %d and egress port is %d \n", inport, port);

	if (port==100) {
		maco_bcast_packet(pd, inport);
	} else {
		/* o/p queue, pkt, no. of pkt to send */
		odp_send_packet((odp_packet_t *)pd->packet, port);
	}

	return 0;
}

void packet_received(packet_descriptor_t *pd, odp_packet_t *p, unsigned portid, int thr_idx)
{
	struct lcore_state *state_tmp = &gconf->state;
	pd->pointer = (uint8_t *)odp_packet_data(*p);
	pd->packet = (packet *)p;

//	set_metadata_inport(pd, portid);
	init_metadata(pd, portid);
	handle_packet(pd, state_tmp->tables);
}

void maco_pktio_queue_thread (void *arg)
{
        int pkts, i;
        int portid;
        int thr;
        macs_conf_t *thr_args;
        odp_pktio_t pktio;
        odp_pktin_queue_t pktin;
        odp_pktout_queue_t pktout;
        int pkts_ok;
	odp_packet_t pkt;
        odp_packet_t pkt_tbl[MAX_PKT_BURST];
        unsigned long pkt_cnt = 0;
        unsigned long err_cnt = 0;
        unsigned long tmp = 0;
        int if_idx;
	odp_queue_t inq;
    odp_event_t ev;
        packet_descriptor_t pd;

//      init_dataplane(&pd, gconf->state.tables);

        info(":: INSIDE odp_main_worker\n");
        thr = odp_thread_id();
        info("  the thread id is %d\n",thr);
        thr_args = arg;

        pktio = odp_pktio_lookup(thr_args->pktio_dev);
        if_idx = odp_dev_name_to_id (thr_args->pktio_dev);

        if (gconf->pktios[if_idx].pktio == ODP_PKTIO_INVALID) {
                debug("  [%02i] Error: lookup of pktio %s failed\n",
                                thr, thr_args->pktio_dev);
                return;
        }
        if (pktio == ODP_PKTIO_INVALID) {
                debug("  [%02i] Error: lookup of pktio for if %s failed\n",
                                thr, thr_args->pktio_dev);
                return;
        }
        info("  [%02i] looked up pktio:%02" PRIu64 ", burst mode\n",
                        thr, odp_pktio_to_u64(pktio));

//	if ((thr_args->mode == APPL_MODE_PKT_QUEUE) &&	
        if (odp_pktin_event_queue(pktio, &inq, 1) != 1) {
                debug("  [%02i] Error: no i/p event queue %s \n", thr, thr_args->pktio_dev);
                return;
        }

        /* Currently Only one interface supported per thread.
         * Hence only one pktio[0] */
        thr_args->pktio[if_idx].pktout = pktout;

        /* Loop packets */
        while (1) {
        odp_pktio_t pktio_tmp;

        if (inq != ODP_QUEUE_INVALID)
            ev = odp_queue_deq(inq);

        if (ev == ODP_EVENT_INVALID)
            continue;

        pkt = odp_packet_from_event(ev);
        if (!odp_packet_is_valid(pkt))
            continue;

        /* Drop packets with errors */
//        if (odp_unlikely(drop_err_pkts(&pkt, 1) == 0)) {
  //          debug("Drop frame - err_cnt:%lu\n", ++err_cnt);
    //        continue;
      //  }

        //pktio_tmp = odp_packet_input(pkt);

	/* Save interface ethernet address */
	if (odp_pktio_mac_addr(gconf->pktios[if_idx].pktio,
				gconf->port_eth_addr[if_idx].addr,
				ODPH_ETHADDR_LEN) != ODPH_ETHADDR_LEN) {
		debug("Error: interface ethernet address unknown\n");
		exit(1);
	}

	packet_received(&pd, &pkt, if_idx, thr_args->thr_idx);
	send_packet (&pd);
}
        return;
}

void odp_main_worker (void *arg)
{
	int pkts, i;
	int portid;
	//	struct macs_conf *macs = &mconf_list[0];;
	int thr;
	//thread_args_t *thr_args;
	macs_conf_t *thr_args;
	odp_pktio_t pktio;
	odp_pktin_queue_t pktin;
	odp_pktout_queue_t pktout;
	int pkts_ok;
	odp_packet_t pkt_tbl[MAX_PKT_BURST];
	unsigned long pkt_cnt = 0;
	unsigned long err_cnt = 0;
	unsigned long tmp = 0;
	int if_idx;
	packet_descriptor_t pd;

//	init_dataplane(&pd, gconf->state.tables);

	info(":: INSIDE odp_main_worker\n");
	thr = odp_thread_id();
	info("	the thread id is %d\n",thr);
	thr_args = arg;

	pktio = odp_pktio_lookup(thr_args->pktio_dev);
	if_idx = odp_dev_name_to_id (thr_args->pktio_dev);

	if (gconf->pktios[if_idx].pktio == ODP_PKTIO_INVALID) {
		debug("  [%02i] Error: lookup of pktio %s failed\n",
				thr, thr_args->pktio_dev);
		return;
	}
	if (pktio == ODP_PKTIO_INVALID) {
		debug("  [%02i] Error: lookup of pktio for if %s failed\n",
				thr, thr_args->pktio_dev);
		return;
	}
	info("  [%02i] looked up pktio:%02" PRIu64 ", burst mode\n",
			thr, odp_pktio_to_u64(pktio));

	/* Loop packets */
	for (;;) {
		pkts = odp_pktin_recv(gconf->pktios[if_idx].pktin[0], pkt_tbl, MAX_PKT_BURST);
		if (odp_unlikely(pkts <= 0))
			continue;
		thr_args->stats[if_idx]->s.rx_packets += pkts;

		for (i = 0; i < pkts; i++) {
			odp_packet_t pkt = pkt_tbl[i];
			/* Save interface ethernet address */
			if (odp_pktio_mac_addr(gconf->pktios[if_idx].pktio,
						gconf->port_eth_addr[if_idx].addr,
						ODPH_ETHADDR_LEN) != ODPH_ETHADDR_LEN) {
				debug("Error: interface ethernet address unknown\n");
				exit(1);
			}

			packet_received(&pd, &pkt, if_idx, thr_args->thr_idx);
			mconf = gconf->mconf[if_idx];

			send_packet (&pd);
#if 0	
			updt_send_packet_buf (&pd, gconf->mconf[if_idx]);
			/* Empty all thread local tx buffers */
			for (port_out = 0; port_out < gbl_args->appl.if_count;
					port_out++) {
				unsigned tx_pkts;
				odp_packet_t *tx_pkt_tbl;

				if (port_out == if_idx ||
						mconf->pktio[port_out].buf.len == 0)
					continue;

				tx_pkts = mconf->pktio[port_out].buf.len;
				mconf->pktio[port_out].buf.len = 0;

				tx_pkt_tbl = mconf->pktio[port_out].buf.pkt;

				pktout = mconf->pktio[port_out].pktout;

				sent = odp_pktout_send(pktout, tx_pkt_tbl, tx_pkts);
				sent = odp_unlikely(sent < 0) ? 0 : sent;
			//	mconf->stats[port_out]->s.tx_packets += sent;

				drops = tx_pkts - sent;

				if (odp_unlikely(drops)) {
					unsigned i;

			//		mconf->stats[port_out]->s.tx_drops += drops;

					/* Drop rejected packets */
					for (i = sent; i < tx_pkts; i++)
						odp_packet_free(tx_pkt_tbl[i]);
				}
			}
#endif
		}

		/* Print packet counts every once in a while */
		tmp += pkts_ok;
		if (odp_unlikely((tmp >= 100000) || /* OR first print:*/
					((pkt_cnt == 0) && ((tmp-1) < MAX_PKT_BURST)))) {
			pkt_cnt += tmp;
			info("  [%02i] pkt_cnt:%lu\n", thr, pkt_cnt);
			fflush(NULL);
			tmp = 0;
		}
	}

	return;
}
