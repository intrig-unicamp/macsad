#ifndef ODP_LIB_H
#define ODP_LIB_H

#include <getopt.h>

#include <stdint.h>
#include <stdio.h>
#include "data_plane_data.h"
#include "backend.h"
#include "dataplane.h" // lookup_table_t
#include "ctrl_plane_backend.h"
#include "odp_tables.h"
#include <net/ethernet.h>
#include "odp_primitives.h"

// ODP headers
#include "odp_api.h"
#include <odp/helper/linux.h>
#include <odp_api.h>
#include <odp/helper/eth.h>
#include <odp/helper/ip.h>
#include <odp/helper/table.h>
#include <net/ethernet.h>

// Backend-specific aliases
#include "aliases.h"

#define parse_as rte_pktmbuf_mtod
#define MAX_ETHPORTS 32

// Shared types and constants
extern uint32_t enabled_port_mask;
extern int promiscuous_on;
extern int numa_on;

#define debug 1

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

//TODO update this counter variable
#define NB_COUNTERS 0

#ifndef NB_TABLES
#define NB_TABLES 0
#endif

#define ODP_MAX_LCORE 2
#define NB_REPLICA 2
#define SOCKET_DEF 0

/** @def PKT_POOL_SIZE                                                           
 * @brief Size of the shared memory block                                        
 */                                                                              
#define PKT_POOL_SIZE 8192                                                       
/** @def PKT_POOL_BUF_SIZE                                                       
 * @brief Buffer size of the packet pool buffer                                  
 */                                                                              
#define PKT_POOL_BUF_SIZE 1856                                                   
/** @def MAX_PKT_BURST                                                           
 * @brief Maximum number of packet in a burst                                    
 */                                                                              
#define MAX_PKT_BURST 32                                                         
                                                                                 
/** @def MAX_WORKERS                                                             
 *  * @brief Maximum number of worker threads                                    
 *   */                                                                          
#define MAX_WORKERS            32                                                
                                                                                 
/** Maximum number of pktio queues per interface */                              
#define MAX_QUEUES             32                                                
                                                                                 
/** Maximum number of pktio interfaces */                                        
#define MAX_PKTIOS             8           

/** @def APPL_MODE_PKT_BURST
 *  * @brief The application will handle pakcets in bursts
 *   */
#define APPL_MODE_PKT_BURST    0

/** @def APPL_MODE_PKT_QUEUE
 *  * @brief The application will handle packets in queues
 *   */
#define APPL_MODE_PKT_QUEUE    1

/** @def APPL_MODE_PKT_SCHED
 *  * @brief The application will handle packets with sheduler
 *   */
#define APPL_MODE_PKT_SCHED    2

/**                                                                              
 * Packet input mode                                                             
 **/                                                                             
typedef enum pktin_mode_t {                                                      
    DIRECT_RECV,                                                                 
    PLAIN_QUEUE,                                                                 
    SCHED_PARALLEL,                                                              
    SCHED_ATOMIC,                                                                
    SCHED_ORDERED,                                                               
} pktin_mode_t;                                                                  
                                                                                 
/**                                                                              
 *Packet output modes                                                            
 **/                                                                             
typedef enum pktout_mode_t {                                                     
    PKTOUT_DIRECT,                                                               
    PKTOUT_QUEUE                                                                 
} pktout_mode_t; 

/** Get rid of path in filename - only for unix-type paths using '/' */          
#define NO_PATH(file_name) (strrchr((file_name), '/') ? \
                        strrchr((file_name), '/') + 1 : (file_name))             
                                                                                 
/**                                                                              
 * Parsed command line application arguments                                     
 */                                                                              
typedef struct appl_args {                                                      
    int cpu_count;                                                               
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
} appl_args_t;                                                                   
                                                                                 
static int exit_threads;    /**< Break workers loop if set to 1 */ 

typedef struct lcore_state {
	//ptrs to the containing socket's instance
	lookup_table_t * tables	  [NB_TABLES];
	counter_t      * counters [NB_COUNTERS];
}lcore_state_t;

struct socket_state {
    // pointers to the instances created on each socket
    lookup_table_t * tables         [NB_TABLES][NB_REPLICA];
    int            * active_replica [NB_TABLES];
    counter_t      * counters       [NB_COUNTERS];
}; 

struct socket_state state[NB_SOCKETS];

/**                                                                              
 * Statistics                                                                    
 */                                                                              
typedef union {                                                                  
    struct {                                                                     
        /** Number of forwarded packets */                                       
        uint64_t packets;                                                        
        /** Packets dropped due to receive error */                              
        uint64_t rx_drops;                                                       
        /** Packets dropped due to transmit error */                             
        uint64_t tx_drops;                                                       
    } s;                                                                         
                                                                                 
    uint8_t padding[ODP_CACHE_LINE_SIZE];                                        
} stats_t ODP_ALIGNED_CACHE;

typedef struct macs_conf{
    char *pktio_dev;    /**< Interface name to use */
    int mode;       /**< Thread mode */
    int thr_idx;                                                                 
    int num_pktio;                                                               
                                                                                 
    struct {                                                                     
        odp_pktio_t rx_pktio;                                                    
        odp_pktio_t tx_pktio;                                                    
        odp_pktin_queue_t pktin;                                                 
        odp_pktout_queue_t pktout;                                               
        odp_queue_t rx_queue;                                                    
        odp_queue_t tx_queue;                                                    
        int rx_idx;                                                              
        int tx_idx;                                                              
        int rx_queue_idx;                                                        
        int tx_queue_idx;                                                        
    } pktio[MAX_PKTIOS];                                                         
                                                                                 
    stats_t *stats; /**< Pointer to per thread stats */                          
} macs_conf_t;

/**                                                                              
 * Grouping of all global data                                                   
 */
typedef struct mac_global{
    /** Per thread packet stats */                                               
    stats_t stats[MAX_WORKERS];
    /** Application (parsed) arguments */
    appl_args_t appl;
 
    odph_linux_pthread_t thread_tbl[MAX_WORKERS];
    /** Thread specific arguments */
	macs_conf_t mconf[ODP_MAX_LCORE];
	/** ptr to statefull memories */
	lcore_state_t state;
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
mac_global_t *gconf;

#define TABCHANGE_DELAY 50 // microseconds

/* Per-port statistics struct */
struct l2fwd_port_statistics {
	uint64_t tx;
	uint64_t rx;
	uint64_t dropped;
} __rte_cache_aligned;

extern struct l2fwd_port_statistics port_statistics[MAX_ETHPORTS];

uint8_t odpc_initialize(int argc, char **argv);

//TODO where to defien these two
uint32_t value32;
uint32_t res32;

#endif // ODP_LIB_H
