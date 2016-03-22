#ifndef ODP_LIB_H
#define ODP_LIB_H

//=============================================================================

#include <getopt.h>

//=============================================================================

#include "data_plane_data.h"
#include "backend.h"
#include "dataplane.h" // lookup_table_t
#include "odp_tables.h"
#include "ctrl_plane_backend.h"
#include <stdint.h>
#include <stdio.h>
#include <net/ethernet.h>
//=============================================================================
// Backend-specific aliases

#include "aliases.h"

#define parse_as rte_pktmbuf_mtod
#define MAX_ETHPORTS 32

//=============================================================================
// Shared types and constants

extern uint32_t enabled_port_mask;
extern int promiscuous_on;
extern int numa_on;

#define RTE_LOGTYPE_L3FWD RTE_LOGTYPE_USER1 // rte_log.h
#define RTE_LOGTYPE_L2FWD RTE_LOGTYPE_USER1 // rte_log.h

#define MBUF_SIZE (2048 + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM)
// TODO is the longer version better?
/*
#define NB_MBUF RTE_MAX	(																	\
				(nb_ports*nb_rx_queue*RTE_TEST_RX_DESC_DEFAULT +							\
				nb_ports*nb_lcores*MAX_PKT_BURST +											\
				nb_ports*n_tx_queue*RTE_TEST_TX_DESC_DEFAULT +								\
				nb_lcores*MEMPOOL_CACHE_SIZE),												\
				(unsigned)8192)
*/
#define NB_MBUF 8192
#define MEMPOOL_CACHE_SIZE 256

#define MAX_PKT_BURST     32
#define BURST_TX_DRAIN_US 100 /* TX drain every ~100us */

struct ether_addr ports_eth_addr[MAX_ETHPORTS];

struct mbuf_table {
	uint16_t len;
	struct rte_mbuf *m_table[MAX_PKT_BURST];
};

#define MAX_RX_QUEUE_PER_LCORE 1//16
#define MAX_TX_QUEUE_PER_PORT 1//RTE_MAX_ETHPORTS
#define MAX_RX_QUEUE_PER_PORT 1//128

#define NB_SOCKETS 8
//#define	BAD_PORT	((uint16_t)-1)

#ifndef NB_TABLES
#define NB_TABLES 0
#endif

// TODO: Is it used somewhere???? Why do we need this in addition to lcore_conf???

#define NB_REPLICA 2

#define SOCKET_DEF 0

lookup_table_t *tables_on_sockets[NB_SOCKETS][NB_TABLES][NB_REPLICA];
int active_replica[NB_SOCKETS][NB_TABLES];

#define TABCHANGE_DELAY 50 // microseconds

/* Per-port statistics struct */
struct l2fwd_port_statistics {
	uint64_t tx;
	uint64_t rx;
	uint64_t dropped;
} __rte_cache_aligned;

extern struct l2fwd_port_statistics port_statistics[MAX_ETHPORTS];

#endif // ODP_LIB_H
