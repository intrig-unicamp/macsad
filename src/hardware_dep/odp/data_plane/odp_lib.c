#include "odp_api.h"
#include "aliases.h"
#include "backend.h"
#include "ctrl_plane_backend.h"
#include "dataplane.h"
#include <unistd.h>

#include "odp_lib.h"
#include <odp/helper/linux.h>
//#include <odp_api.h>
#include <odp/helper/eth.h>
#include <odp/helper/ip.h>
#include <odp/helper/table.h>
#include <net/ethernet.h>

struct socket_state state[NB_SOCKETS];

//=   shared   ================================================================
extern void init_control_plane();
//extern __m128i val_eth[MAX_ETHPORTS];

extern uint32_t enabled_port_mask;
int promiscuous_on = 0; /**< Ports set in promiscuous mode off by default. */
//TODO
int numa_on = 0; /**< NUMA is not enabled by default. */

#define MAX_LCORE_PARAMS 1024
uint16_t nb_lcore_params;

//=   used only here   ========================================================
#define MAX_JUMBO_PKT_LEN  9600

#define RTE_TEST_RX_DESC_DEFAULT 128
#define RTE_TEST_TX_DESC_DEFAULT 512
static uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
static uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;

/** Global barrier to synchronize main and workers */
odp_barrier_t barrier;
int exit_threads;    /**< Break workers loop if set to 1 */

extern void odp_main_worker (void);
//=============================================================================

#define UNUSED(x) (void)(x)

//--------
static int parse_max_pkt_len(const char *pktlen)
{
	char *end = NULL;
	unsigned long len;

	/* parse decimal string */
	len = strtoul(pktlen, &end, 10);
	if ((pktlen[0] == '\0') || (end == NULL) || (*end != '\0'))
		return -1;

	if (len == 0)
		return -1;

	return len;
}

static int parse_portmask(const char *portmask)
{
	char *end = NULL;
	unsigned long pm;

	/* parse hexadecimal string */
	pm = strtoul(portmask, &end, 16);
	if ((portmask[0] == '\0') || (end == NULL) || (*end != '\0'))
		return -1;

	if (pm == 0)
		return -1;

	return pm;
}

static int maco_get_first_cpu(const odp_cpumask_t * mask)
{
	int cpu;
	cpu = odp_cpumask_first(mask);
	return cpu;
}

static int maco_get_next_cpu(const odp_cpumask_t * mask, int cpu)
{
	int cpu_first = odp_cpumask_first(mask);
	int cpu_last = odp_cpumask_last(mask);
/* if only one cpu available */
	if (cpu_first == cpu_last)
		return cpu_first;

/* send the next cpumask if available */
	for (cpu += 1; cpu <= cpu_last; cpu++)
		if (odp_cpumask_isset(mask, cpu))
			return cpu;

/* if "cpu" is the last one, send again the first cpumask */
	return cpu_first;

	return -1;

}

static void print_ethaddr(const char *name, const struct ether_addr *eth_addr)
{
#if 0
	char buf[ETHER_ADDR_FMT_SIZE];
	ether_format_addr(buf, ETHER_ADDR_FMT_SIZE, eth_addr);
	info("%s%s", name, buf);
#endif
}

//int init_lcore_confs()
int odpc_lcore_conf_init ()
{
	info("Configuring lcore structs...\n");
//	struct macs_conf *qconf;
	int socketid;
	unsigned lcore_id;
	for (lcore_id = 0; lcore_id < MAC_MAX_LCORE; lcore_id++) {
/*		if (rte_lcore_is_enabled(lcore_id) == 0) continue;
		if (numa_on) socketid = rte_lcore_to_socket_id(lcore_id);
		else socketid = 0;
		if (socketid >= NB_SOCKETS) {
			rte_exit(EXIT_FAILURE, "Socket %d of lcore %u is out of range %d\n",
					socketid, lcore_id, NB_SOCKETS);
			info("socket is out of range.\n");
		}
		*/
		//TODO remove below socketid set and uncomment above code
		socketid = 0;
//		qconf = &mconf_list[lcore_id];
		int i;
		for(i = 0; i < NB_TABLES; i++)
			gconf->state.tables[i] = state[socketid].tables[i][0];
		for(i = 0; i < NB_COUNTERS; i++)
			gconf->state.counters[i] = state[socketid].counters[i];
	}
	info("Configuring lcore structs done...\n");
	return 0;
}

static void change_replica(int socketid, int tid, int replica) {
	/*
	   struct lcore_conf *qconf;
	   int core_socketid;
	   unsigned lcore_id;
	   for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++) {
	   if (rte_lcore_is_enabled(lcore_id) == 0) continue;
	   core_socketid = rte_lcore_to_socket_id(lcore_id);
	   if(core_socketid != socketid) continue;
	   qconf = &lcore_conf[lcore_id];
	   qconf->state.tables[tid] = state[socketid].tables[tid][replica]; // TODO should this be atomic?
	   state[socketid].active_replica[tid] = replica;
	//info("\n\n\nCHANGING REPLICA of TABLE %d: core %d on socket %d now uses replica %d\n\n\n", tid, lcore_id, socketid, replica);
	}
	*/
}

//	int current_replica = active_replica[socketid][tableid];
//	fun(tables_on_sockets[socketid][tableid][next_replica], par);
//		if(current_replica != next_replica) fun(tables_on_sockets[socketid][tableid][current_replica], par);
#define CHANGE_TABLE(fun, par...) \
{ \
    int current_replica = state[socketid].active_replica[tableid]; \
	int next_replica = (current_replica+1)%NB_REPLICA; \
    fun(state[socketid].tables[tableid][next_replica], par);	\
	change_replica(socketid, tableid, next_replica); \
	usleep(TABCHANGE_DELAY); \
	for(current_replica = 0; current_replica < NB_REPLICA; current_replica++) \
		if(current_replica != next_replica) fun(state[socketid].tables[tableid][current_replica], par); \
}

//if(tables_on_sockets[socketid][0][0] != NULL)
#define FORALLNUMANODES(b) \
	int socketid; \
for(socketid = 0; socketid < NB_SOCKETS; socketid++) \
if(state[socketid].tables[0][0] != NULL) \
b

void exact_add_promote(int tableid, uint8_t* key, uint8_t* value)
{
//	info(":::: EXECUTING exact add promote \n");
	FORALLNUMANODES(CHANGE_TABLE(exact_add, key, value))
}
void lpm_add_promote(int tableid, uint8_t* key, uint8_t depth, uint8_t* value)
{
	FORALLNUMANODES(CHANGE_TABLE(lpm_add, key, depth, value))
}
void ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value) {
	FORALLNUMANODES(CHANGE_TABLE(ternary_add, key, mask, value))
}

void table_setdefault_promote(int tableid, uint8_t* value)
{
	FORALLNUMANODES(CHANGE_TABLE(table_setdefault, value))
}

int odp_dev_name_to_id (char *if_name) {
	int i;

	for (i=0; i < gconf->appl.if_count; i++) {
			if(strcmp (gconf->appl.if_names[i], if_name) == 0) {
				info("interface id %d found for %s\n",i, if_name);
				return i;
			}
	}
	debug("interface id not found for %s\n",if_name);
	return -1;
}

#if 0
static void
create_counters_on_socket(int socketid)
{
    if(counter_config == NULL) return;
    info("Initializing counters on socket %d\n", socketid);
    int i;
    for(i = 0; i < NB_COUNTERS; i++) {
        info("Creating counter %d on socket %d\n", i, socketid);
        counter_t c = counter_config[i];
        state[socketid].counters[i] = malloc(sizeof(counter_t));
		memcpy(state[socketid].counters[i], &c, sizeof(counter_t));
		// init
		info("Initializing counter %d on socket %d\n", i, socketid);
		state[socketid].counters[i]->values = (vector_t*)malloc(sizeof(vector_t));
		vector_init(state[socketid].counters[i]->values, 1 /* initial size */, c.size, sizeof(odp_atomic32_t), &odp_atomic32_init, socketid);
	}
}
#endif

void increase_counter(int counterid, int index)
{
    odp_atomic_inc_u32(&gconf->state.counters[counterid]->values[index]);
}

uint32_t read_counter(int counterid, int index)
{
    uint32_t cnt = 0;
    int socketid;
    for(socketid = 0; socketid < NB_SOCKETS; socketid++)
        if(state[socketid].tables[0][0] != NULL)
            cnt += odp_atomic_load_u32(&state[socketid].counters[counterid]->values[index]);
    return cnt;
}

/*
static int
init_mem(unsigned nb_mbuf)
{
    unsigned lcore_id;
    char s[64];

    for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++) {
        if (rte_lcore_is_enabled(lcore_id) == 0)
            continue;

        int socketid = get_socketid(lcore_id);

        if (socketid >= NB_SOCKETS) {
            rte_exit(EXIT_FAILURE, "Socket %d of lcore %u is out of range %d\n",
                socketid, lcore_id, NB_SOCKETS);
        }
        if (pktmbuf_pool[socketid] == NULL) {
            snprintf(s, sizeof(s), "mbuf_pool_%d", socketid);
            pktmbuf_pool[socketid] =
                rte_mempool_create(s, nb_mbuf, MBUF_SIZE, MEMPOOL_CACHE_SIZE,
                    sizeof(struct rte_pktmbuf_pool_private),
                    rte_pktmbuf_pool_init, NULL,
                    rte_pktmbuf_init, NULL,
                    socketid, 0);
            if (pktmbuf_pool[socketid] == NULL)
                rte_exit(EXIT_FAILURE,
                        "Cannot init mbuf pool on socket %d\n", socketid);
            else
                info("Allocated mbuf pool on socket %d\n", socketid);
        }
    }
    return 0;
}
*/

/*
int odpc_stfull_memories_init()
{
    info("Initializing stateful memories...\n");
    int socketid;
    unsigned lcore_id;
    for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++) {
        if (rte_lcore_is_enabled(lcore_id) == 0) continue;
        if (numa_on) socketid = rte_lcore_to_socket_id(lcore_id);
        else socketid = 0;
        if (socketid >= NB_SOCKETS) {
            rte_exit(EXIT_FAILURE, "Socket %d of lcore %u is out of range %d\n",
                socketid, lcore_id, NB_SOCKETS);
        }
    }
    return 0;
}
*/

/**
 * Create a pktio handle
 *
 * @param dev        Name of device to open
 * @param index      Pktio index
 * @param num_rx     Number of RX queues
 * @param num_tx     Number of TX queues
 * @param pool       Pool to associate with device for packet RX/TX
 *
 * @retval 0 on success
 * @retval -1 on failure
 */

static int create_pktio(const char *name, int if_idx, int num_rx,
							int num_tx, odp_pool_t pool, int mode)
{
	odp_pktio_t pktio;
    odp_pktio_capability_t capa;
	odp_pktio_param_t pktio_param;
	odp_pktin_queue_param_t pktin_param;
	odp_pktin_queue_param_t in_queue_param;
	odp_pktout_queue_param_t pktout_param;
	odp_pktout_queue_param_t out_queue_param;
    odp_pktio_op_mode_t mode_rx;
    odp_pktio_op_mode_t mode_tx;
	int ret;

	odp_pktio_param_init(&pktio_param);

	switch (mode) {
		case  APPL_MODE_PKT_BURST:
			pktio_param.in_mode = ODP_PKTIN_MODE_DIRECT;
			break;
		case APPL_MODE_PKT_QUEUE:
			pktio_param.in_mode = ODP_PKTIN_MODE_QUEUE;
			break;
		case APPL_MODE_PKT_SCHED:
			pktio_param.in_mode = ODP_PKTIN_MODE_SCHED;
			break;
		default:
			debug("invalid mode %d\n", mode);
	}

    /* Open a packet IO instance */
	pktio = odp_pktio_open(name, pool, &pktio_param);
	if (pktio == ODP_PKTIO_INVALID) {
		debug("Error: pktio create failed for %s\n", name);
        return -1;
	}

    printf("created pktio %" PRIu64 " (%s)\n", odp_pktio_to_u64(pktio),
           name);

    if (odp_pktio_capability(pktio, &capa)) {
        printf("Error: capability query failed %s\n", name);
        return -1;
    }

	odp_pktin_queue_param_init(&pktin_param);
	odp_pktout_queue_param_init(&pktout_param);

    mode_tx = ODP_PKTIO_OP_MT_UNSAFE;
    mode_rx = ODP_PKTIO_OP_MT_UNSAFE;

    if (num_rx > (int)capa.max_input_queues) {
        printf("Sharing %i input queues between %i workers\n",
               capa.max_input_queues, num_rx);
        num_rx  = capa.max_input_queues;
        mode_rx = ODP_PKTIO_OP_MT;
    }

    if (num_tx > (int)capa.max_output_queues) {
        printf("Sharing %i output queues between %i workers\n",
               capa.max_output_queues, num_tx);
        num_tx  = capa.max_output_queues;
        mode_tx = ODP_PKTIO_OP_MT;
    }

    pktin_param.hash_enable = 1;
    pktin_param.hash_proto.proto.ipv4 = 1;
//    pktin_param.hash_proto.proto.ipv4_tcp = 1;
//    pktin_param.hash_proto.proto.ipv4_udp = 1;
    pktin_param.num_queues  = num_rx;
    pktin_param.op_mode     = mode_rx;

    pktout_param.num_queues = num_tx;
    pktout_param.op_mode    = mode_tx;

	if (odp_pktin_queue_config(pktio, &pktin_param))
	{
		debug("Error: pktin config failed for %s\n", name);
		return -1;
	}

	if (odp_pktout_queue_config(pktio, &pktout_param))
	{
		debug("Error: pktout config failed for %s\n", name);
		return -1;
	}

	if (odp_pktin_queue(pktio, &gconf->pktios[if_idx].pktin, num_rx)
						!= num_rx)
	{
		debug("  Error: no pktin queue for %s\n", name);
        return -1;
	}
	if (odp_pktout_queue(pktio, &gconf->pktios[if_idx].pktout, num_tx)
						!= num_tx)
	{
		debug("  Error: no pktout queue for %s\n", name);
        return -1;
	}

    printf("created %i input and %i output queues on (%s)\n", num_rx,
           num_tx, name);

    gconf->pktios[if_idx].num_rx_queue = num_rx;
    gconf->pktios[if_idx].num_tx_queue = num_tx;
    gconf->pktios[if_idx].pktio        = pktio;

	return 0;
}

static void *launch_worker(void *arg)
{
	odp_main_worker();
	return NULL;
}

/**
 * Prinf usage information
 */
static void usage(char *progname)
{
    printf("\n"
           "MACSAD forwarding plane\n"
           "\n"
           "Usage: %s OPTIONS\n"
           "  E.g. %s -i eth0,eth1 \n"
           "\n"
           "Mandatory OPTIONS:\n"
           "  -i, --interface Eth interfaces (comma-separated, no spaces)\n"
           "                  Interface count min 1, max %i\n"
           "\n"
           "Optional OPTIONS:\n"
             "  -c, --count <number> CPU count.\n"
			 "  -m, --multi <0/1> Use Multiple CPU(Boolean).\n"
 //          "  -m, --mode      0: Receive and send directly (no queues)\n"
//           "                  1: Receive and send via queues.\n"
//           "                  2: Receive via scheduler, send via queues.\n"
           "  -h, --help           Display help and exit.\n\n"
           "\n", NO_PATH(progname), NO_PATH(progname), MAX_PKTIOS
        );
}

/**
 * Parse command line arguments
 *
 * @param argc       argument count
 * @param argv[]     argument vector
 * @param appl_args  Store application arguments here
 */
static void parse_args(int argc, char *argv[], appl_args_t *appl_args)
{
    int opt;
    int long_index;
    char *token;
    char *addr_str;
    size_t len;
    int i;
    static struct option longopts[] = {
        {"count", required_argument, NULL, 'c'},
        {"interface", required_argument, NULL, 'i'},
//        {"mode", required_argument, NULL, 'm'},     /* return 'm' */
        {"multi", required_argument, NULL, 'm'},     /* return 'm' */
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

	static const char *shortopts = "+c:+i:+m:h";

	/* let helper collect its own arguments (e.g. --odph_proc) */
	odph_parse_options(argc, argv, shortopts, longopts);

    appl_args->time = 0; /* loop forever if time to run is 0 */
    appl_args->accuracy = 10; /* get and print pps stats second */

	opterr = 0; /* do not issue errors on helper options */

    while (1) {
		opt = getopt_long(argc, argv, shortopts, longopts, &long_index);

		if (opt == -1)
            break;  /* No more options */

        switch (opt) {
        case 'c':
            appl_args->cpu_count = atoi(optarg);
            break;
        case 'i':
            len = strlen(optarg);
            if (len == 0) {
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
            len += 1;   /* add room for '\0' */

            appl_args->if_str = malloc(len);
            if (appl_args->if_str == NULL) {
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }

            /* count the number of tokens separated by ',' */
            strcpy(appl_args->if_str, optarg);
            for (token = strtok(appl_args->if_str, ","), i = 0;
                 token != NULL;
                 token = strtok(NULL, ","), i++)
                ;

            appl_args->if_count = i;

            if (appl_args->if_count < 1 ||
                appl_args->if_count > MAX_PKTIOS) {
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }

            /* allocate storage for the if names */
            appl_args->if_names =
                calloc(appl_args->if_count, sizeof(char *));

            /* store the if names (reset names string) */
            strcpy(appl_args->if_str, optarg);
            for (token = strtok(appl_args->if_str, ","), i = 0;
                 token != NULL; token = strtok(NULL, ","), i++) {
                appl_args->if_names[i] = token;
            }
            break;

        case 'm':
            i = atoi(optarg);
			appl_args->mcpu_enable = atoi(optarg);
            break;
        case 'h':
            usage(argv[0]);
            exit(EXIT_SUCCESS);
            break;
        default:
            break;
        }
    }

    if (appl_args->if_count == 0) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    optind = 1;     /* reset 'extern optind' from the getopt lib */
}

/**
 * Print system and application info
 */
static void print_info(char *progname, appl_args_t *appl_args)
{
    int i;

    printf("\n"
           "ODP system info\n"
           "---------------\n"
           "ODP API version: %s\n"
           "ODP impl name:   %s\n"
           "CPU model:       %s\n"
           "CPU freq (hz):   %" PRIu64 "\n"
           "Cache line size: %i\n"
           "CPU count:       %i\n"
           "\n",
           odp_version_api_str(), odp_version_impl_name(),
           odp_cpu_model_str(), odp_cpu_hz_max(),
           odp_sys_cache_line_size(), odp_cpu_count());

    printf("Running ODP appl: \"%s\"\n"
           "-----------------\n"
           "IF-count:        %i\n"
           "Using IFs:      ",
           progname, appl_args->if_count);
    for (i = 0; i < appl_args->if_count; ++i)
        printf(" %s", appl_args->if_names[i]);
    printf("\n\n");
    fflush(NULL);
}

static void print_port_mapping(void)
{
    int if_count, num_workers;
    int thr, pktio;

    if_count    = gconf->appl.if_count;
    num_workers = gconf->appl.num_workers;

    printf("\nWorker mapping table (port[queue])\n--------------------\n");
    for (thr = 0; thr < num_workers; thr++) {
        uint8_t port_idx;
        int queue_idx;
        macs_conf_t *thr_args = &gconf->mconf[thr];
        int num = thr_args->num_rx_pktio;

        printf("Worker %i\n", thr);

        for (pktio = 0; pktio < num; pktio++) {
            port_idx = thr_args->pktios[pktio].port_idx;
            queue_idx =  thr_args->pktios[pktio].rqueue_idx;
            printf("  %i[%i]\n", port_idx, queue_idx);
        }
    }

    printf("\nPort config\n--------------------\n");
    for (pktio = 0; pktio < if_count; pktio++) {
        const char *dev = gconf->appl.if_names[pktio];

        printf("Port %i (%s)\n", pktio, dev);
        printf("  rx workers %i\n",
               gconf->pktios[pktio].num_rx_thr);
        printf("  rx queues %i\n",
               gconf->pktios[pktio].num_rx_queue);
        printf("  tx queues %i\n",
               gconf->pktios[pktio].num_tx_queue);
    }
    printf("\n");
}

static int print_speed_stats(int num_workers, stats_t (*thr_stats)[MAX_PKTIOS],
                 int duration, int timeout)
{
    uint64_t rx_pkts_prev[MAX_PKTIOS] = {0};
    uint64_t tx_pkts_prev[MAX_PKTIOS] = {0};
    uint64_t rx_pkts_tot;
    uint64_t tx_pkts_tot;
    uint64_t rx_pps;
    uint64_t tx_pps;
    int i, j;
    int elapsed = 0;
    int stats_enabled = 1;
    int loop_forever = (duration == 0);
    int num_ifaces = gconf->appl.if_count;

    if (timeout <= 0) {
        stats_enabled = 0;
        timeout = 1;
    }
    /* Wait for all threads to be ready*/
    odp_barrier_wait(&barrier);

    do {
#if 0
        uint64_t rx_pkts[MAX_PKTIOS] = {0};
        uint64_t tx_pkts[MAX_PKTIOS] = {0};
        uint64_t rx_drops = 0;
        uint64_t tx_drops = 0;

        rx_pkts_tot = 0;
        tx_pkts_tot = 0;

        sleep(timeout);
        elapsed += timeout;

        for (i = 0; i < num_workers; i++) {
            for (j = 0; j < num_ifaces; j++) {
                rx_pkts[j] += thr_stats[i][j].s.rx_packets;
                tx_pkts[j] += thr_stats[i][j].s.tx_packets;
                rx_drops += thr_stats[i][j].s.rx_drops;
                tx_drops += thr_stats[i][j].s.tx_drops;
            }
        }

        if (!stats_enabled)
            continue;

        for (j = 0; j < num_ifaces; j++) {
            rx_pps = (rx_pkts[j] - rx_pkts_prev[j]) / timeout;
            tx_pps = (tx_pkts[j] - tx_pkts_prev[j]) / timeout;
            printf("  Port %d: %" PRIu64 " rx pps, %" PRIu64
                   " tx pps, %" PRIu64 " rx pkts, %" PRIu64
                   " tx pkts\n", j, rx_pps, tx_pps, rx_pkts[j],
                   tx_pkts[j]);

            rx_pkts_prev[j] = rx_pkts[j];
            tx_pkts_prev[j] = tx_pkts[j];
            rx_pkts_tot += rx_pkts[j];
            tx_pkts_tot += tx_pkts[j];
        }

        printf("Total: %" PRIu64 " rx pkts, %" PRIu64 " tx pkts, %"
               PRIu64 " rx drops, %" PRIu64 " tx drops\n", rx_pkts_tot,
               tx_pkts_tot, rx_drops, tx_drops);
#endif
    } while (loop_forever || (elapsed < duration));

    return rx_pkts_tot > 100 ? 0 : -1;
}

/*
 * Set worker thread afinity for switch ports and calculate number of
 * queues needed.
 * less workers (N) than interfaces (M)
 *  - assign each worker to process every Nth interface
 *  - workers process inequal number of interfaces, when M is not divisible by N
 *  - needs only single queue per interface
 * otherwise
 *  - assign an interface to every Mth worker
 *  - interfaces are processed by inequal number of workers, when N is not
 *    divisible by M
 *  - tries to configure a queue per worker per interface
 *  - shares queues, if interface capability does not allows a queue per worker
 */
static void macs_set_worker_afinity(void)
{
    int if_count, num_workers;
    int rx_idx, mconf_id, pktio;
    macs_conf_t *mconf;

    if_count    = gconf->appl.if_count;
    num_workers = gconf->appl.num_workers;

    if (if_count > num_workers) {
        mconf_id = 0;

        for (rx_idx = 0; rx_idx < if_count; rx_idx++) {
            mconf = &gconf->mconf[mconf_id];
            pktio    = mconf->num_rx_pktio;
            mconf->pktios[pktio].port_idx = rx_idx;
            mconf->num_rx_pktio++;

            gconf->pktios[rx_idx].num_rx_thr++;

            mconf_id++;
            if (mconf_id >= num_workers)
                mconf_id = 0;
        }
    } else {
        rx_idx = 0;

        for (mconf_id = 0; mconf_id < num_workers; mconf_id++) {
            mconf = &gconf->mconf[mconf_id];
            pktio    = mconf->num_rx_pktio;
            mconf->pktios[pktio].port_idx = rx_idx;
            mconf->num_rx_pktio++;

            gconf->pktios[rx_idx].num_rx_thr++;

            rx_idx++;
            if (rx_idx >= if_count)
                rx_idx = 0;
        }
    }
	return;
}

/*
 * Set queue afinity with threads and fill in missing thread arguments (handles)
 */
static void macs_set_queue_afinity(void)
{
    int num_workers;
    int mconf_id, pktio;
	int rx_idx, tx_queue, rx_queue;
	macs_conf_t *mconf = NULL;
    num_workers = gconf->appl.num_workers;

    for (mconf_id = 0; mconf_id < num_workers; mconf_id++) {
        mconf = &gconf->mconf[mconf_id];
        int num = mconf->num_rx_pktio;

        /* Receive only from selected ports */
        for (pktio = 0; pktio < num; pktio++) {
            rx_idx   = mconf->pktios[pktio].port_idx;
            rx_queue = gconf->pktios[rx_idx].next_rx_queue;

            mconf->pktios[pktio].pktin =
                gconf->pktios[rx_idx].pktin[rx_queue];
            mconf->pktios[pktio].rqueue_idx = rx_queue;

            rx_queue++;
            if (rx_queue >= gconf->pktios[rx_idx].num_rx_queue)
                rx_queue = 0;
            gconf->pktios[rx_idx].next_rx_queue = rx_queue;
        }
        /* Send to all ports */
        for (pktio = 0; pktio < (int)gconf->appl.if_count; pktio++) {
            tx_queue = gconf->pktios[pktio].next_tx_queue;

            mconf->pktios[pktio].pktout =
                gconf->pktios[pktio].pktout[tx_queue];
            mconf->pktios[pktio].tqueue_idx = tx_queue;

            tx_queue++;
            if (tx_queue >= gconf->pktios[pktio].num_tx_queue)
                tx_queue = 0;
            gconf->pktios[pktio].next_tx_queue = tx_queue;
        }
    }
}

static void gconf_init(mac_global_t *gconf)
{
	int pktio, queue;

	memset(gconf, 0, sizeof(mac_global_t));
	for (pktio = 0; pktio < MAX_PKTIOS; pktio++) {
		gconf->pktios[pktio].pktio = ODP_PKTIO_INVALID;
	}
}

uint8_t odpc_initialize(int argc, char **argv)
{
	odph_linux_pthread_t thd;
	odp_pool_param_t params;
	odp_instance_t instance;
	odp_cpumask_t cpumask;
	odph_linux_thr_params_t thr_params;
	char cpumaskstr[ODP_CPUMASK_STR_SIZE];
	int num_workers, i, j;
	int cpu, if_count;
	int ret;
	stats_t (*stats)[MAX_PKTIOS];
	odp_shm_t shm;
	odph_odpthread_t thread_tbl[MAC_MAX_LCORE];

	/* initialize ports */
	int nb_ports = 2;

	/* init ODP  before calling anything else */
	if (odp_init_global(&instance, NULL, NULL)) {
		debug("Error: ODP global init failed.\n");
		exit(1);
	}

	/* init this thread */
	if (odp_init_local(instance, ODP_THREAD_CONTROL)) {
		debug("Error: ODP local init failed.\n");
		exit(1);
	}

	/* Reserve memory for args from shared mem */
	shm = odp_shm_reserve("shm_gconf", sizeof(mac_global_t),
			ODP_CACHE_LINE_SIZE, 0);
	gconf = odp_shm_addr(shm);

	if (gconf == NULL) {
		debug("Error: Shared mem alloc failed for global configuration.\n");
		exit(1);
	}
	gconf_init(gconf);

	/* Parse and store the application arguments */
	parse_args(argc, argv, &gconf->appl);

	/* Print both system and application information */
	print_info(NO_PATH(argv[0]), &gconf->appl);

	/* Default to MAC_MAX_LCORE  unless user specified */
	if (gconf->appl.cpu_count) {
		num_workers = gconf->appl.cpu_count;
	} else {
		num_workers = MAC_MAX_LCORE;
	}

	/* Get default worker cpumask */
	num_workers = odp_cpumask_default_worker(&cpumask, num_workers);
	(void)odp_cpumask_to_str(&cpumask, cpumaskstr, sizeof(cpumaskstr));
	gconf->appl.num_workers = num_workers;

	for (i = 0; i < num_workers; i++)
		gconf->mconf[i].thr_idx = i;

    if_count = gconf->appl.if_count;

	info("num worker threads: %i\n", num_workers);
	info("first CPU:          %i\n", odp_cpumask_first(&cpumask));
	info("cpu mask:           %s\n", cpumaskstr);

	/* create the packet pool */
	odp_pool_param_init(&params);
	params.pkt.seg_len = PKT_POOL_BUF_SIZE;
	params.pkt.len     = PKT_POOL_BUF_SIZE;
	params.pkt.num     = PKT_POOL_SIZE;
	params.type        = ODP_POOL_PACKET;

	gconf->pool = odp_pool_create("PktsPool", &params);
	if (gconf->pool == ODP_POOL_INVALID) {
		debug("Error: packet pool create failed.\n");
		exit(1);
	}
	odp_pool_print(gconf->pool);

	macs_set_worker_afinity();

	/* Create a pktio instance for each interface */
	for (i = 0; i < if_count; ++i)
	{
        const char *dev = gconf->appl.if_names[i];
        int num_rx;

        /* An RX queue per assigned worker thread.
		 * A private TX queue each worker thread */
        num_rx = gconf->pktios[i].num_rx_thr;

		if (create_pktio(dev, i, num_rx, num_workers,
					gconf->pool, gconf->appl.mode))
		{
			exit(EXIT_FAILURE);
		}

		info("interface id %d, ifname %s, pktio:%02" PRIu64 " \n", i, gconf->appl.if_names[i], odp_pktio_to_u64(gconf->pktios[i].pktio));

        ret = odp_pktio_promisc_mode_set(gconf->pktios[i].pktio, 1);
        if (ret != 0) {
            printf("Error: failed to set port to promiscuous mode.\n");
            exit(EXIT_FAILURE);
        }
	}
    gconf->pktios[i].pktio = ODP_PKTIO_INVALID;

	macs_set_queue_afinity();

    print_port_mapping();

	/* Initialize all the tables defined in p4 src */
	odpc_lookup_tbls_init();

	/* initiate control plane */
	init_control_plane();

	// TODO
	odpc_lcore_conf_init();

	/* Create and init worker threads */
	memset(thread_tbl, 0, sizeof(thread_tbl));

    odp_barrier_init(&barrier, num_workers + 1);

	stats = gconf->stats;

	memset(&thr_params, 0, sizeof(thr_params));
	thr_params.thr_type = ODP_THREAD_WORKER;
	thr_params.instance = instance;
	thr_params.start    = launch_worker;

	    /* Create worker threads */
	cpu = maco_get_first_cpu(&cpumask);
	info("count threadmask %d\n ", odp_cpumask_count(&cpumask));
	info("num worksers is %d\n", num_workers);

	for (i = 0; i < num_workers; ++i) {
        odp_cpumask_t thd_mask;

		for (j = 0; j < MAX_PKTIOS; j++)
			gconf->mconf[i].stats[j] = &stats[i][j];

		//void *(*thr_run_func) (void *);

		gconf->mconf[i].mode = gconf->appl.mode;

//		gconf->mconf[i].pktio_dev = gconf->appl.if_names[i];
//		info("if %s and pktio_dev %s \n ", gconf->appl.if_names[i], gconf->mconf[i].pktio_dev);

//		if (gconf->appl.mode == APPL_MODE_PKT_BURST) {
//			thr_run_func = launch_worker;
			info("thread func set to launch_worker\n ");
//		}
		/*
		 * Create threads one-by-one instead of all-at-once,
		 * because each thread gets different arguments.
		 */

		thr_params.arg      = &gconf->mconf[i];
//		thr_params.start    = thr_run_func;

		odp_cpumask_zero(&thd_mask);
		odp_cpumask_set(&thd_mask, cpu);
		odph_odpthreads_create(&thread_tbl[i],
				&thd_mask, &thr_params);

//		cpu = maco_get_next_cpu(&cpumask, cpu);
		// Enable this to use one cpu per thread per interface
		if (gconf->appl.mcpu_enable == 1)
			cpu = maco_get_next_cpu(&cpumask, cpu);
	}

    /* Start packet receive and transmit */
    for (i = 0; i < if_count; ++i) {
        odp_pktio_t pktio;

        pktio = gconf->pktios[i].pktio;
        ret   = odp_pktio_start(pktio);
        if (ret) {
            printf("Error: unable to start %s\n",
                   gconf->appl.if_names[i]);
            exit(EXIT_FAILURE);
        }
    }

    ret = print_speed_stats(num_workers, gconf->stats,
                gconf->appl.time, gconf->appl.accuracy);
    exit_threads = 1;

    /* Master thread waits for other threads to exit */
	for (i = 0; i < num_workers; ++i)
		odph_odpthreads_join(&thread_tbl[i]);

	free(gconf->appl.if_names);
	free(gconf->appl.if_str);
// TODO
//	free(gconf);
	printf("Exit\n\n");

	return 0;
}

uint8_t odpc_des ()
{
#if 0
	if (odpc_lookup_tbls_des()) {
		info("Lookup table destroy failed.\n");
	}


	if (odp_term_local()) {
		info("Error: ODP local term failed.\n");
		exit(EXIT_FAILURE);
	}

	if (odp_term_global()) {
		debug("Error: ODP global term failed.\n");
		exit(EXIT_FAILURE);
	}
#endif
	return 0;
}

