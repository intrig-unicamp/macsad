// Copyright 2018 INTRIG/FEEC/UNICAMP (University of Campinas), Brazil
//
//Licensed under the Apache License, Version 2.0 (the "License");
//you may not use this file except in compliance with the License.
//You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
//Unless required by applicable law or agreed to in writing, software
//distributed under the License is distributed on an "AS IS" BASIS,
//WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//See the License for the specific language governing permissions and
//limitations under the License.

#ifndef ODP_LIB_H
#define ODP_LIB_H

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include "data_plane_data.h"
#include "backend.h"
#include "actions.h"
#include "dataplane.h" // lookup_table_t
#include "ctrl_plane_backend.h"
#include "odp_tables.h"
#include <net/ethernet.h>
#include "odp_primitives.h"

// ODP headers
#include "odp_api.h"
#include <odp/helper/odph_api.h>

// Backend-specific aliases
#include "aliases.h"

#define parse_as rte_pktmbuf_mtod
#define MAX_ETHPORTS 32

// Shared types and constants
extern uint32_t enabled_port_mask;
extern int promiscuous_on;
extern int numa_on;

//#define debug 1
#define RTE_LOGTYPE_L3FWD RTE_LOGTYPE_USER1 // rte_log.h
#define RTE_LOGTYPE_L2FWD RTE_LOGTYPE_USER1 // rte_log.h

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

#define MBUF_SIZE (2048 + sizeof(struct rte_mbuf) + RTE_PKTMBUF_HEADROOM)
// TODO is the longer version better?
/*
#define NB_MBUF RTE_MAX	(					\
				(nb_ports*nb_rx_queue*RTE_TEST_RX_DESC_DEFAULT +	\
				nb_ports*nb_lcores*MAX_PKT_BURST +				\
				nb_ports*n_tx_queue*RTE_TEST_TX_DESC_DEFAULT +	\
				nb_lcores*MEMPOOL_CACHE_SIZE),					\
				(unsigned)8192)
*/
#define NB_MBUF 8192
#define MEMPOOL_CACHE_SIZE 256

#define MAX_PKT_BURST     32
#define BURST_TX_DRAIN_US 100 /* TX drain every ~100us */
#define DEF_RX_PKT_TMO_US ODP_PKTIN_NO_WAIT /* default- do not wait */

struct ether_addr ports_eth_addr[MAX_ETHPORTS];

struct mbuf_table {
	uint16_t len;
	struct rte_mbuf *m_table[MAX_PKT_BURST];
};

#define MAX_RX_QUEUE_PER_LCORE 1//16
#define MAX_TX_QUEUE_PER_PORT 1//RTE_MAX_ETHPORTS
#define MAX_RX_QUEUE_PER_PORT 1//128

#define NB_SOCKETS 8

//TODO update this counter variable
#define NB_COUNTERS 0

#ifndef NB_TABLES
#define NB_TABLES 0
#endif

#define MAC_MAX_LCORE 32
#define NB_REPLICA 2
#define SOCKET_DEF 0

/** @def PKT_POOL_SIZE
 * @brief Size of the shared memory block
 */
#define PKT_POOL_SIZE 8192
//#define PKT_POOL_SIZE (512*2048)
/** @def PKT_POOL_BUF_SIZE
 * @brief Buffer size of the packet pool buffer
 */
#define PKT_POOL_BUF_SIZE 1856
/** @def MAX_WORKERS
 * @brief Maximum number of worker threads
 */
#define MAX_WORKERS            32
/** Maximum number of pktio queues per interface */
#define MAX_QUEUES             32
/** Maximum number of pktio interfaces */
#define MAX_PKTIOS             8

/**
 * Packet input mode
 */
typedef enum pktin_mode_t {
    DIRECT_RECV,  /* PKT_BURST */
    PLAIN_QUEUE, /* PKT_QUEUE */
    SCHED_PARALLEL, /* PKT_SCHED_PARALLEL */
    SCHED_ATOMIC,/* PKT_SCHED_ATOMIC */
    SCHED_ORDERED,/* PKT_SCHED_ORDERED */
} pktin_mode_t;

/**
 *Packet output modes
 **/
typedef enum pktout_mode_t {
    PKTOUT_DIRECT,
    PKTOUT_QUEUE
} pktout_mode_t;

static inline int sched_mode(pktin_mode_t in_mode)
{
    return (in_mode == SCHED_PARALLEL) ||
           (in_mode == SCHED_ATOMIC)   ||
           (in_mode == SCHED_ORDERED);
}

/** Get rid of path in filename - only for unix-type paths using '/' */
#define NO_PATH(file_name) (strrchr((file_name), '/') ? \
                        strrchr((file_name), '/') + 1 : (file_name))

/**
 * Parsed command line application arguments
 */
typedef struct appl_args {
    uint16_t if_count;       /**< Number of interfaces to be used */
    uint16_t num_workers;    /**< Number of worker threads */
    uint16_t cpu_count;
    uint16_t accuracy;       /**< Number of seconds to get and print statistics */
  //  int mcpu_enable;
    pktin_mode_t in_mode;   /**< Packet input mode */
    pktout_mode_t out_mode; /**< Packet output mode */
//TODO need to use the in_mode, out_mode and remove this variable
//	int mode;
    uint16_t time;       /**< Time in seconds to run. */
    char **if_names;    /**< Array of pointers to interface names */
    char *if_str;       /**< Storage for interface names */
    const char *cpu_mask;
    //int error_check;        /**< Check packet errors */
    uint64_t recv_tmo;        /**< Check packet errors */
} appl_args_t;

extern bool exit_threads;

typedef struct lcore_state {
	//ptrs to the containing socket's instance
	lookup_table_t * tables	  [NB_TABLES];
	counter_t      * counters [NB_COUNTERS];
}lcore_state_t;

struct socket_state {
    // pointers to the instances created on each socket
    lookup_table_t * tables         [NB_TABLES][NB_REPLICA];
    int            active_replica [NB_TABLES];
    counter_t      * counters       [NB_COUNTERS];
};

struct socket_state state[NB_SOCKETS];

/**
 * Statistics
 */
typedef union {
    struct {
        /** Number of forwarded packets */
        /** Number of forwarded packets */
        uint64_t packets;
        /** Number of received packets */
        uint64_t rx_packets;
        /** Number of transmitted packets */
        uint64_t tx_packets;
        /** Packets dropped due to receive error */
        uint64_t rx_drops;
        /** Packets dropped due to transmit error */
        uint64_t tx_drops;
    } s;

    uint8_t padding[ODP_CACHE_LINE_SIZE];
} stats_t ODP_ALIGNED_CACHE;

/**
 * Packet buffer
 */
typedef struct pkt_buf_t {
	odp_packet_t pkt[MAX_PKT_BURST]; /**< Array of packet handles */
	unsigned len;            /**< Number of packets in buffer */
} pkt_buf_t;

typedef struct macs_conf{
    //int mode;       /* TODO: Thread mode (process/thread) */
    uint8_t thr_idx;
    /** Number of interfaces from which to receive packets */
    uint8_t num_rx_pktio;
    /** Number of interfaces to which to send packets */
    uint8_t num_tx_pktio;

    /* rx_pktio */
    struct {
        odp_pktin_queue_t pktin;   /** Packet input queue */
        //odp_queue_t rx_queue;
        uint8_t rx_idx;      /** Rx Port index */
        uint8_t rqueue_idx;      /** Queue index */
    } rx_pktios[MAX_PKTIOS];
    /* tx_pktio */
    struct {
        pkt_buf_t buf;         /** Packet TX buffer */
        odp_pktout_queue_t pktout; /** Packet output queue */
        //odp_queue_t tx_queue;
        uint8_t tx_idx;      /** Tx Port index */
        uint8_t tqueue_idx;         /** Queue index */
    } tx_pktios[MAX_PKTIOS];

//    stats_t *stats[MAX_PKTIOS];    /** Interface statistics */
} macs_conf_t ODP_ALIGNED_CACHE;

/**
 * Grouping of all global data
 */
typedef struct mac_global{
    /** Thread specific arguments */
	macs_conf_t mconf[MAC_MAX_LCORE];
	/** Per thread interface statistics */
//	stats_t stats[MAX_WORKERS][MAX_PKTIOS];
	/* pkt pool */
    odp_pool_t pool;
	/** ptr to statefull memories */
	lcore_state_t state;

	uint16_t num_pktio;
	/** Table of pktio handles */
    struct {
        odp_pktio_t pktio;
        odp_pktin_queue_t pktin[MAX_QUEUES];
        odp_pktout_queue_t pktout[MAX_QUEUES];
        odp_queue_t rx_q[MAX_QUEUES];
        odp_queue_t tx_q[MAX_QUEUES];
        uint16_t num_rx_thr;
        uint16_t num_tx_thr;
        uint16_t num_rx_queue;
        uint16_t num_tx_queue;
        uint16_t next_rx_queue;
        uint16_t next_tx_queue;
    } pktios[MAX_PKTIOS];
    /** Application (parsed) arguments */
    appl_args_t appl;
}mac_global_t ODP_ALIGNED_CACHE;

/** Global pointer to mac_global */
extern mac_global_t *gconf;

extern odp_instance_t instance;

#define TABCHANGE_DELAY 50 // microseconds

uint8_t maco_initialize(int argc, char **argv);
void maco_terminate();

int odpc_worker_mode_direct(void *arg);
int odpc_worker_mode_queue(void *arg);
int odpc_worker_mode_sched(void *arg);

//TODO where to defien these two
uint32_t value32;
uint32_t res32;

#endif // ODP_LIB_H
