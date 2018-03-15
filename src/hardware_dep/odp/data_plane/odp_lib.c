#include <stdlib.h>
#include <unistd.h>
#include <net/ethernet.h>
#include <signal.h>
#include <execinfo.h>
#include "odp_api.h"
#include "aliases.h"
#include "backend.h"
#include "ctrl_plane_backend.h"
#include "dataplane.h"
#include "odp_lib.h"

struct socket_state state[NB_SOCKETS];
/** Global pointer to mac_global */
mac_global_t *gconf;
odp_instance_t instance;

//=   shared   ================================================================
extern void init_control_plane();

extern uint32_t enabled_port_mask;
int promiscuous_on = 0; /**< Ports set in promiscuous mode off by default. */
//TODO
int numa_on = 0; /**< NUMA is not enabled by default. */
uint16_t nb_lcore_params;

//=   used only here   ========================================================
#define MAX_JUMBO_PKT_LEN  9600

#define RTE_TEST_RX_DESC_DEFAULT 128
#define RTE_TEST_TX_DESC_DEFAULT 512

extern void odp_main_worker (void);
//=============================================================================

#define UNUSED(x) (void)(x)

static void sig_handler(int signo)
{
	size_t num_stack_frames;
	const char  *signal_name;
	void  *bt_array[128];

	switch (signo) {
		case SIGINT:
			signal_name = "SIGINT";   break;
		case SIGILL:
			signal_name = "SIGILL";   break;
		case SIGFPE:
			signal_name = "SIGFPE";   break;
		case SIGSEGV:
			signal_name = "SIGSEGV";  break;
		case SIGTERM:
			signal_name = "SIGTERM";  break;
		case SIGBUS:
			signal_name = "SIGBUS";   break;
		default:
			signal_name = "UNKNOWN";  break;
	}

	if (signo == SIGINT)
	{
		exit_threads = 1;
		printf("Received signal=%u (%s) exiting.", signo, signal_name);
	}else{
		num_stack_frames = backtrace(bt_array, 100);
		printf("2 Received signal=%u (%s) exiting.", signo, signal_name);
		backtrace_symbols_fd(bt_array, num_stack_frames, fileno(stderr));
		fflush(NULL);
		sync();
		abort();
	}
}

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
static void create_counters_on_socket(int socketid)
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
    odp_atomic_inc_u32((odp_atomic_u32_t *)&gconf->state.counters[counterid]->values[index]);
}

uint32_t read_counter(int counterid, int index)
{
    uint32_t cnt = 0;
#if 0
    int socketid;
    for(socketid = 0; socketid < NB_SOCKETS; socketid++)
        if(state[socketid].tables[0][0] != NULL)
            cnt += odp_atomic_load_u32((odp_atomic_u32_t *)&state[socketid].counters[counterid]->values[index]);
#endif
    cnt = odp_atomic_load_u32((odp_atomic_u32_t *)&gconf->state.counters[counterid]->values[index]);
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
        int num_tx, odp_pool_t pool)
{
    odp_pktio_t pktio;
    odp_pktio_capability_t capa;
	odp_pktio_config_t config;
    odp_pktio_param_t pktio_param;
    odp_pktin_queue_param_t pktin_param;
    odp_pktio_op_mode_t mode_rx, mode_tx;
    odp_pktout_queue_param_t pktout_param;
    odp_schedule_sync_t  sync_mode;
//    int num_tx_shared;

    pktin_mode_t in_mode = gconf->appl.in_mode;

    odp_pktio_param_init(&pktio_param);

    pktio_param.in_mode = ODP_PKTIN_MODE_DIRECT;
    pktio_param.out_mode = ODP_PKTOUT_MODE_DIRECT;

    pktio = odp_pktio_open(name, pool, &pktio_param);
    if (pktio == ODP_PKTIO_INVALID) {
        debug("Error: pktio open failed for %s\n", name);
        return -1;
    }
    printf("Pktio open success for %" PRIu64 " (%s)\n", \
            odp_pktio_to_u64(pktio), name);

    if (odp_pktio_capability(pktio, &capa)) {
        debug("Error: capability query failed %s\n", name);
        return -1;
    }

    odp_pktio_config_init(&config);
    config.parser.layer = ODP_PKTIO_PARSER_LAYER_NONE;
    odp_pktio_config(pktio, &config);

    odp_pktin_queue_param_init(&pktin_param);
    odp_pktout_queue_param_init(&pktout_param);

    mode_tx = ODP_PKTIO_OP_MT_UNSAFE;
    mode_rx = ODP_PKTIO_OP_MT_UNSAFE;

    //num_tx_shared = capa.max_output_queues;

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

    if (gconf->appl.in_mode == DIRECT_RECV) {
        if (odp_pktin_queue(pktio, gconf->pktios[if_idx].pktin, num_rx)
                != num_rx)
        {
            debug("  Error: no pktin queue for %s\n", name);
            return -1;
        }
    }

    if (gconf->appl.out_mode == PKTOUT_DIRECT) {
        if (odp_pktout_queue(pktio, gconf->pktios[if_idx].pktout, num_tx)
                != num_tx)
        {
            debug("  Error: no pktout queue for %s\n", name);
            return -1;
        }
    }

    printf("created %i input and %i output queues on (%s)\n", num_rx,
            num_tx, name);

    gconf->pktios[if_idx].num_rx_queue = num_rx;
    gconf->pktios[if_idx].num_tx_queue = num_tx;
    gconf->pktios[if_idx].pktio        = pktio;

    return 0;
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
            "  -C, --cpu_mask CPU mask to be set\n"
            //			 "  -m, --multi <0/1> Use Multiple CPU(Boolean).\n"
            "  -d, --timeout <in ns> Timeout for packet recv.\n"
            "  -m, --mode      Packet input mode\n"
            "                  0: Direct mode: PKTIN_MODE_DIRECT (default)\n"
            "                  1: Scheduler mode with parallel queues: PKTIN_MODE_SCHED + SCHED_SYNC_PARALLEL\n"
            "                  2: Scheduler mode with atomic queues:   PKTIN_MODE_SCHED + SCHED_SYNC_ATOMIC\n"
            "                  3: Scheduler mode with ordered queues:  PKTIN_MODE_SCHED + SCHED_SYNC_ORDERED\n"
            "                  4: Plain queue mode: ODP_PKTIN_MODE_QUEUE\n"
            "  -o, --out_mode  Packet output mode\n"
            "                  0: Direct mode: PKTOUT_MODE_DIRECT (default)\n"
            "                  1: Queue mode:  PKTOUT_MODE_QUEUE\n"
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
    size_t len;
    odp_cpumask_t cpumask, cpumask_args, cpumask_and;
    int i, num_workers;
    static struct option longopts[] = {
        {"count", required_argument, NULL, 'c'},
        {"cpu_mask", required_argument, NULL, 'C'},
        {"interface", required_argument, NULL, 'i'},
        {"mode", required_argument, NULL, 'm'},
        {"out_mode", required_argument, NULL, 'o'},
        {"timeout", required_argument, NULL, 'd'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    static const char *shortopts = "+c:+C:+i:+m:h";

    /* let helper collect its own arguments (e.g. --odph_proc) */
    odph_parse_options(argc, argv, shortopts, longopts);

    appl_args->time = 0; /* loop forever if time to run is 0 */
    appl_args->accuracy = 15; /* get and print pps stats second */
    appl_args->cpu_count = 0; /* single cpu by default */
    appl_args->recv_tmo = DEF_RX_PKT_TMO_US; /* default- do not wait*/

    opterr = 0; /* do not issue errors on helper options */

    while (1) {
        opt = getopt_long(argc, argv, shortopts, longopts, &long_index);

        if (opt == -1)
            break;  /* No more options */

        switch (opt) {
            case 'c':
                appl_args->cpu_count = atoi(optarg);
                break;
            case 'C':
                appl_args->cpu_mask = optarg;
                odp_cpumask_from_str(&cpumask_args, appl_args->cpu_mask);
                num_workers = odp_cpumask_default_worker(&cpumask, 0);
                odp_cpumask_and(&cpumask_and, &cpumask_args, &cpumask);
                if (odp_cpumask_count(&cpumask_and) <
                                odp_cpumask_count(&cpumask_args)) {
                        debug("Wrong cpu mask, max cpu's:%d\n",
                                        num_workers);
                exit(EXIT_FAILURE);
                }
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

            case 'd':
                appl_args->recv_tmo = odp_pktin_wait_time(atol(optarg));
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
    printf("\n"
            "Mode:            ");
        printf("PKTIN_DIRECT, ");
        printf("PKTOUT_DIRECT");

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
        uint8_t rx_idx;
        int queue_idx;
        macs_conf_t *thr_args = &gconf->mconf[thr];
        int num = thr_args->num_rx_pktio;

        printf("Worker %i\n", thr);

        for (pktio = 0; pktio < num; pktio++) {
            rx_idx = thr_args->rx_pktios[pktio].rx_idx;
            queue_idx =  thr_args->rx_pktios[pktio].rqueue_idx;
            printf("  %i[%i]\n", rx_idx, queue_idx);
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
    uint64_t rx_pkts_tot = 0;
    int elapsed = 0;
//    int stats_enabled = 1;
    int loop_forever = (duration == 0);

    if (timeout <= 0) {
//        stats_enabled = 0;
        timeout = 1;
    }

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
    int rx_idx, tx_idx, mconf_id, pktio_id;
    macs_conf_t *mconf;

    if_count    = gconf->appl.if_count;
    num_workers = gconf->appl.num_workers;

    if (if_count > num_workers) {
        mconf_id = 0;

        for (rx_idx = 0; rx_idx < if_count; rx_idx++) {
            mconf = &gconf->mconf[mconf_id];
            pktio_id = mconf->num_rx_pktio;
            mconf->rx_pktios[pktio_id].rx_idx = rx_idx;
            mconf->num_rx_pktio++;
            gconf->pktios[rx_idx].num_rx_thr++;
printf("rx_idx %d, num_rx_thr %d\n", rx_idx, gconf->pktios[rx_idx].num_rx_thr);
            mconf_id++;
            if (mconf_id >= num_workers)
                mconf_id = 0;
        }
    } else {
        rx_idx = 0;

        for (mconf_id = 0; mconf_id < num_workers; mconf_id++) {
            mconf = &gconf->mconf[mconf_id];
	    pktio_id = mconf->num_rx_pktio;
	    mconf->rx_pktios[pktio_id].rx_idx = rx_idx;
	    for (tx_idx = 0; tx_idx < if_count; tx_idx++){
		    if(tx_idx != rx_idx){
			    mconf->tx_pktios[tx_idx].tx_idx = tx_idx;
	    mconf->num_tx_pktio++;
	    gconf->pktios[tx_idx].num_tx_thr++;
printf("=tx_idx %d, num_tx_thr %d\n", tx_idx, gconf->pktios[tx_idx].num_rx_thr);
		    }
	    }	
	    mconf->num_rx_pktio++;
	    gconf->pktios[rx_idx].num_rx_thr++;
printf("=rx_idx %d, num_rx_thr %d\n", rx_idx, gconf->pktios[rx_idx].num_rx_thr);
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
  printf("\nQueue binding (indexes)\n-----------------------\n");
    for (mconf_id = 0; mconf_id < num_workers; mconf_id++) {
        mconf = &gconf->mconf[mconf_id];
        int num = mconf->num_rx_pktio;
                printf("worker %i\n", mconf_id);
        /* Receive only from selected ports */
        for (pktio = 0; pktio < num; pktio++) {
            rx_idx   = mconf->rx_pktios[pktio].rx_idx;
            rx_queue = gconf->pktios[rx_idx].next_rx_queue;

            mconf->rx_pktios[pktio].rqueue_idx = rx_queue;
            mconf->rx_pktios[pktio].pktin =
                gconf->pktios[rx_idx].pktin[rx_queue];
            mconf->rx_pktios[pktio].rx_queue =
                gconf->pktios[rx_idx].rx_q[rx_queue];
 printf("  rx: pktio %i, queue %i\n", rx_idx, rx_queue);
            rx_queue++;
            if (rx_queue >= gconf->pktios[rx_idx].num_rx_queue)
                rx_queue = 0;
            gconf->pktios[rx_idx].next_rx_queue = rx_queue;
        }
        /* Send to all ports */
        for (pktio = 0; pktio < (int)gconf->appl.if_count; pktio++) {
            tx_queue = gconf->pktios[pktio].next_tx_queue;

            mconf->tx_pktios[pktio].tqueue_idx = tx_queue;
            mconf->tx_pktios[pktio].pktout =
                gconf->pktios[pktio].pktout[tx_queue];
            mconf->tx_pktios[pktio].tx_queue =
                gconf->pktios[pktio].tx_q[tx_queue];
 printf("  tx: pktio %i, queue %i\n", pktio, tx_queue);

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

        for (queue = 0; queue < MAX_QUEUES; queue++)
            gconf->pktios[pktio].rx_q[queue] = ODP_QUEUE_INVALID;
    }

   gconf->appl.in_mode  = DIRECT_RECV;
   gconf->appl.out_mode = PKTOUT_DIRECT;
}

//static inline odp_u16sum_t odph_chksum(void *buffer, int len)
//return checksum value in host cpu order
/*uint16_t calculate_csum16(void* buf, uint16_t length) {
    uint16_t value16 = odph_chksum(buf, length);
    return value16;
}*/

uint32_t packet_length(packet_descriptor_t* pd) {
    //Returns sum of buffer lengths over all packet segments.
    return odp_packet_buf_len((odp_packet_t) pd->wrapper);
}

uint8_t maco_initialize(int argc, char **argv)
{
    odp_pool_param_t params;
    odp_cpumask_t cpumask;
    char cpumaskstr[ODP_CPUMASK_STR_SIZE];
    int num_workers, i, j, if_count, ret;
    int cpu, ctrl_cpu, affinity;
    stats_t (*stats)[MAX_PKTIOS];
    odp_shm_t shm;
    odp_pktio_info_t info;
    odph_odpthread_t thread_tbl[MAC_MAX_LCORE];
    int (*thr_run_func)(void *);

/* Code for handling signals.
   TODO: should we write a func for this
*/
    struct sigaction signal_action;
    memset(&signal_action, 0, sizeof(signal_action));
    signal_action.sa_handler = sig_handler;
    sigfillset(&signal_action.sa_mask);
    sigaction(SIGILL,  &signal_action, NULL);
    sigaction(SIGFPE,  &signal_action, NULL);
    sigaction(SIGSEGV, &signal_action, NULL);
    sigaction(SIGTERM, &signal_action, NULL);
    sigaction(SIGBUS,  &signal_action, NULL);
    sigaction(SIGINT,  &signal_action, NULL);

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

	if (gconf->appl.cpu_mask != NULL) {
		odp_cpumask_from_str(&cpumask, gconf->appl.cpu_mask);
		ctrl_cpu = odp_cpumask_first(&cpumask);
		(void)odp_cpumask_to_str(&cpumask, cpumaskstr, sizeof(cpumaskstr));
		printf("Control Thread: CPU %d, Mask %s \n",ctrl_cpu, cpumaskstr);

		if (odph_odpthread_setaffinity(ctrl_cpu) != 0) {
			debug("Set main process affinify to "
					"cpu(%d) failed.\n", ctrl_cpu);
			exit(EXIT_FAILURE);
		}
		odp_cpumask_clr(&cpumask, ctrl_cpu);

		num_workers = odp_cpumask_count (&cpumask);
		num_workers = odp_cpumask_default_worker(&cpumask, num_workers);
		cpu = odp_cpumask_first(&cpumask);
		(void)odp_cpumask_to_str(&cpumask, cpumaskstr, sizeof(cpumaskstr));
		printf("Worker Thread : CPUs %d, 1st CPU %d, Mask %s \n",num_workers, cpu, cpumaskstr);

	} else if (odp_cpu_count() > 2)
	{
		odp_cpumask_zero(&cpumask);
		/* allocate the 1st available control cpu to main process */
		if (odp_cpumask_default_control(&cpumask, 1) != 1) {
            debug("Allocate main process CPU core failed.\n");
            exit(EXIT_FAILURE);
        }
        cpu = odp_cpumask_first(&cpumask);
        (void)odp_cpumask_to_str(&cpumask, cpumaskstr, sizeof(cpumaskstr));
        printf("Control Thread: CPU %d, Mask %s \n",cpu, cpumaskstr);

        if (odph_odpthread_setaffinity(cpu) != 0) {
            debug("Set main process affinify to "
                    "cpu(%d) failed.\n", cpu);
            exit(EXIT_FAILURE);
        }

        /* read back affinity to verify */
        affinity = odph_odpthread_getaffinity();
        if ((affinity < 0) || (cpu != affinity)) {
            debug("Verify main process affinity failed: "
                    "set(%d) read(%d).\n", cpu, affinity);
            exit(EXIT_FAILURE);
        }
        cpu = 0;
        affinity = 0;
        odp_cpumask_zero(&cpumask);

        /* Default to MAC_MAX_LCORE  unless user specified */
        if (gconf->appl.cpu_count) {
            num_workers = gconf->appl.cpu_count;
        } else {
            num_workers = MAC_MAX_LCORE;
        }

        num_workers = odp_cpumask_default_worker(&cpumask, num_workers);
        cpu = odp_cpumask_first(&cpumask);
        (void)odp_cpumask_to_str(&cpumask, cpumaskstr, sizeof(cpumaskstr));
        printf("Worker Thread : CPUs %d, 1st CPU %d, Mask %s \n",num_workers, cpu, cpumaskstr);
    }
    else {
        /* Default to MAC_MAX_LCORE  unless user specified */
        if (gconf->appl.cpu_count) {
            num_workers = gconf->appl.cpu_count;
        } else {
            num_workers = MAC_MAX_LCORE;
        }
        /* Get default worker cpumask */
        num_workers = odp_cpumask_default_worker(&cpumask, num_workers);
        cpu = odp_cpumask_first(&cpumask);
        (void)odp_cpumask_to_str(&cpumask, cpumaskstr, sizeof(cpumaskstr));
        printf("Worker: CPUs %d, 1st CPU %d, Mask %s \n",num_workers, cpu, cpumaskstr);
    }
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
        printf("Error: packet pool create failed.\n");
        exit(EXIT_FAILURE);
    }
    odp_pool_print(gconf->pool);

    macs_set_worker_afinity();

    /* Create a pktio instance for each interface */
    for (i = 0; i < if_count; ++i)
    {
        const char *dev = gconf->appl.if_names[i];
        int num_rx;// num_tx;

        /* A queue per assigned worker */
        num_rx = gconf->pktios[i].num_rx_thr;
//      num_tx = gconf->pktios[i].num_tx_thr;

        if (create_pktio(dev, i, num_rx, num_workers, gconf->pool))
            exit(EXIT_FAILURE);

        if (odp_pktio_info(gconf->pktios[i].pktio, &info)) {
            debug("Error: pktio info failed %s\n", dev);
            return -1;
        }

        printf("interface id %d, ifname %s, drv: %s, pktio:%lu, num of rx q=%d \n", i, gconf->appl.if_names[i], info.drv_name, odp_pktio_to_u64(gconf->pktios[i].pktio),gconf->pktios[i].num_rx_thr);

        ret = odp_pktio_promisc_mode_set(gconf->pktios[i].pktio, 1);
        if (ret != 0) {
            printf("Error: failed to set port to promiscuous mode.\n");
            exit(EXIT_FAILURE);
        }
    }
    gconf->pktios[i].pktio = ODP_PKTIO_INVALID;

    macs_set_queue_afinity();

    if (gconf->appl.in_mode == DIRECT_RECV)
        print_port_mapping();

    /* Initialize all the tables defined in p4 src */
    odpc_lookup_tbls_init();

    /* initiate control plane */
    init_control_plane();

    // TODO
    odpc_lcore_conf_init();

    /* Create and init worker threads */
    memset(thread_tbl, 0, sizeof(thread_tbl));

    stats = gconf->stats;

    thr_run_func = odpc_worker_mode_direct;

    /* Create worker threads */
    cpu = maco_get_first_cpu(&cpumask);
    info("count threadmask %d\n ", odp_cpumask_count(&cpumask));
    info("num worksers is %d\n", num_workers);

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

    for (i = 0; i < num_workers; ++i) {
        odp_cpumask_t thd_mask;
        odph_odpthread_params_t thr_params;

        memset(&thr_params, 0, sizeof(thr_params));
        thr_params.thr_type = ODP_THREAD_WORKER;
        thr_params.instance = instance;
        thr_params.arg      = &gconf->mconf[i];
        thr_params.start    = thr_run_func;

        for (j = 0; j < MAX_PKTIOS; j++)
            gconf->mconf[i].stats[j] = &stats[i][j];

        odp_cpumask_zero(&thd_mask);
        odp_cpumask_set(&thd_mask, cpu);
        odph_odpthreads_create(&thread_tbl[i], &thd_mask, &thr_params);

        // Enable this to use one cpu per thread per interface
        if (gconf->appl.cpu_count > 0)
            cpu = maco_get_next_cpu(&cpumask, cpu);
    }

    /* Master thread waits for other threads to exit */
    for (i = 0; i < num_workers; ++i)
        odph_odpthreads_join(&thread_tbl[i]);

    return 0;
}

void maco_terminate()
{
#if 0
	free(gconf->appl.if_names);
	free(gconf->appl.if_str);
	if (odpc_lookup_tbls_des()) {
		info("Lookup table destroy failed.\n");
	}
	if (odp_pool_destroy(gconf->pool)) {
		debug("Error: pool destroy\n");
		exit(EXIT_FAILURE);
	}
	if (odp_term_local()) {
		debug("Error: term local\n");
		exit(EXIT_FAILURE);
	}
	if (odp_term_global(instance)) {
		debug("Error: term global\n");
		exit(EXIT_FAILURE);
	}
	printf("\nMACSAD Exiting\n\n");
#endif
	return;
}
