#ifndef ODP_LIB_H
#define ODP_LIB_H

#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include "data_plane_data.h"
#include "backend.h"
#include "dataplane.h" // lookup_table_t
#include "ctrl_plane_backend.h"
#include "odp_vss_extern.h"

// ODP headers
#include "odp_api.h"
#include <odp/helper/eth.h>
#include <odp/helper/chksum.h>
#include <odp/helper/ip.h>
#include <odp/helper/table.h>
#include <net/ethernet.h>
#include <odp/helper/threads.h>

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
#define RTE_LOGTYPE_P4_FWD RTE_LOGTYPE_USER1 // rte_log.h

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

/* there was 1 1 1 */
#define MAX_RX_QUEUE_PER_LCORE 16
#define MAX_TX_QUEUE_PER_PORT RTE_MAX_ETHPORTS
#define MAX_RX_QUEUE_PER_PORT 128

#define NB_SOCKETS 8
//#define	BAD_PORT	((uint16_t)-1)
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
/** @def MAX_PKT_BURST
 * @brief Maximum number of packet in a burst
 */
#define MAX_PKT_BURST 32
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
    int cpu_count;
    const char *cpu_mask;
	int mcpu_enable;
    int if_count;       /**< Number of interfaces to be used */
    int num_workers;    /**< Number of worker threads */
    char **if_names;    /**< Array of pointers to interface names */
    pktin_mode_t in_mode;   /**< Packet input mode */
    pktout_mode_t out_mode; /**< Packet output mode */
//TODO need to use the in_mode, out_mode and remove this variable
	int mode;
	int time;       /**< Time in seconds to run. */
    int accuracy;       /**< Number of seconds to get and print statistics */
    char *if_str;       /**< Storage for interface names */
    int error_check;        /**< Check packet errors */
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
    int              active_replica [NB_TABLES];
    counter_t      * counters       [NB_COUNTERS][MAC_MAX_LCORE];
    // p4_register_t  * registers      [NB_REGISTERS];
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
    int mode;       /* TODO: Thread mode (process/thread) */
    int thr_idx;
    /** Number of interfaces from which to receive packets */
    int num_rx_pktio;
    /** Number of interfaces to which to send packets */
    int num_tx_pktio;

    /* rx_pktio */
    struct {
        odp_pktin_queue_t pktin;   /** Packet input queue */
        odp_queue_t rx_queue;
        uint8_t rx_idx;      /** Rx Port index */
        int rqueue_idx;      /** Queue index */
    } rx_pktios[MAX_PKTIOS];
    /* tx_pktio */
    struct {
        odp_pktout_queue_t pktout; /** Packet output queue */
        odp_queue_t tx_queue;
        uint8_t tx_idx;      /** Tx Port index */
        int tqueue_idx;         /** Queue index */
        pkt_buf_t buf;         /** Packet TX buffer */
    } tx_pktios[MAX_PKTIOS];

    stats_t *stats[MAX_PKTIOS];    /** Interface statistics */
} macs_conf_t;

/**
 * Grouping of all global data
 */
typedef struct mac_global{
	/** Per thread interface statistics */
	stats_t stats[MAX_WORKERS][MAX_PKTIOS];
    /** Application (parsed) arguments */
    appl_args_t appl;
	/* pkt pool */
    odp_pool_t pool;
    /** Thread specific arguments */
	macs_conf_t mconf[MAC_MAX_LCORE];
	/** ptr to statefull memories */
	lcore_state_t state;

	int num_pktio;
	/** Table of pktio handles */
    struct {
        odp_pktio_t pktio;
        odp_pktin_queue_t pktin[MAX_QUEUES];
        odp_pktout_queue_t pktout[MAX_QUEUES];
        odp_queue_t rx_q[MAX_QUEUES];
        odp_queue_t tx_q[MAX_QUEUES];
        int num_rx_thr;
        int num_tx_thr;
        int num_rx_queue;
        int num_tx_queue;
        int next_rx_queue;
        int next_tx_queue;
    } pktios[MAX_PKTIOS];
}mac_global_t;

/** Global pointer to mac_global */
extern mac_global_t *gconf;

extern odp_instance_t instance;

#define TABCHANGE_DELAY 50 // microseconds

/* Per-port statistics struct */
struct l2fwd_port_statistics {
	uint64_t tx;
	uint64_t rx;
	uint64_t dropped;
} __rte_cache_aligned;

extern struct l2fwd_port_statistics port_statistics[MAX_PKTIOS];

#define PRINT_OPAQUE_STRUCT(p)  print_mem((p), sizeof(*(p)))
static void print_mem_hex(void const *vp, size_t n) {
    unsigned char const *p = vp;
    for (size_t i=0; i<n; i++) printf("%02x ", p[i]);
    putchar('\n');
};
static void print_mem_bin(void const * const ptr, size_t const size)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
//    for (i=size-1;i>=0;i--) for (j=7;j>=0;j--) {
    for (i=0;i<size;i++) for (j=0;j<8;j++) {
        byte = b[i] & (1<<j);
        byte >>= j;
        printf("%u", byte);
    }
    puts("");
}

uint8_t maco_initialize(int argc, char **argv);
void maco_terminate();
	
int odpc_worker_mode_direct(void *arg);
int odpc_worker_mode_queue(void *arg);
int odpc_worker_mode_sched(void *arg);

//TODO where to defien these two
uint32_t value32;
uint32_t res32;

#endif // ODP_LIB_H
