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

struct l2fwd_port_statistics port_statistics[MAX_ETHPORTS];

//=   used only here   ========================================================
#define MAX_JUMBO_PKT_LEN  9600

#define RTE_TEST_RX_DESC_DEFAULT 128
#define RTE_TEST_TX_DESC_DEFAULT 512
static uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
static uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;

extern void
odp_main_worker (void);
//=============================================================================

#define UNUSED(x) (void)(x)

/** Global barrier to synchronize main and workers */
static odp_barrier_t barrier;
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

static void print_ethaddr(const char *name, const struct ether_addr *eth_addr)
{
#if 0
	char buf[ETHER_ADDR_FMT_SIZE];
	ether_format_addr(buf, ETHER_ADDR_FMT_SIZE, eth_addr);
	printf("%s%s", name, buf);
#endif
}

//int init_lcore_confs()
int odpc_lcore_conf_init ()
{
	printf("Configuring lcore structs...\n");
//	struct macs_conf *qconf;
	int socketid;
	unsigned lcore_id;
	for (lcore_id = 0; lcore_id < ODP_MAX_LCORE; lcore_id++) {
/*		if (rte_lcore_is_enabled(lcore_id) == 0) continue;
		if (numa_on) socketid = rte_lcore_to_socket_id(lcore_id);
		else socketid = 0;
		if (socketid >= NB_SOCKETS) {
			rte_exit(EXIT_FAILURE, "Socket %d of lcore %u is out of range %d\n",
					socketid, lcore_id, NB_SOCKETS);
			printf("socket is out of range.\n");
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
	printf("Configuring lcore structs done...\n");
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
	//printf("\n\n\nCHANGING REPLICA of TABLE %d: core %d on socket %d now uses replica %d\n\n\n", tid, lcore_id, socketid, replica);
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
//	printf(":::: EXECUTING exact add promote \n");
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
				printf("interface id %d found for %s\n",i, if_name);
				return i;
			}
	}
	printf("interface id not found for %s\n",if_name);
	return -1;
}

#if 0
static void
create_counters_on_socket(int socketid)
{
    if(counter_config == NULL) return;
    printf("Initializing counters on socket %d\n", socketid);
    int i;
    for(i = 0; i < NB_COUNTERS; i++) {
        printf("Creating counter %d on socket %d\n", i, socketid);
        counter_t c = counter_config[i];
        state[socketid].counters[i] = malloc(sizeof(counter_t));
		memcpy(state[socketid].counters[i], &c, sizeof(counter_t));
		// init
		printf("Initializing counter %d on socket %d\n", i, socketid);
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
                printf("Allocated mbuf pool on socket %d\n", socketid);
        }
    }
    return 0;
}
*/

/*
int odpc_stfull_memories_init()
{
    printf("Initializing stateful memories...\n");
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
        if (state[socketid].tables[0][0] == NULL) {
            create_tables_on_socket(socketid);
            create_counters_on_socket(socketid);
        }
    }
    return 0;
}
*/

static odp_pktio_t create_pktio(const char *name, odp_pool_t pool, int mode)
{
	odp_pktio_param_t pktio_param;
	odp_pktin_queue_param_t pktin_param;
	odp_pktin_queue_param_t in_queue_param;
	odp_pktout_queue_param_t out_queue_param;
	odp_pktio_t pktio;

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
			printf("invalid mode %d\n", mode);
	}

    /* Open a packet IO instance */
	pktio = odp_pktio_open(name, pool, &pktio_param);
	if (pktio == ODP_PKTIO_INVALID) {
		printf("Error: pktio create failed for %s\n", name);
		exit(1);
	}

//	odp_pktout_queue_param_init(&out_queue_param);

    odp_pktin_queue_param_init(&pktin_param);

	if (odp_pktin_queue_config(pktio, &pktin_param))
		printf("Error: pktin config failed for %s\n", name);

	if (odp_pktout_queue_config(pktio, NULL))
		printf("Error: pktout config failed for %s\n", name);

#if 0
	in_queue_param.op_mode = ODP_PKTIO_OP_MT_UNSAFE;
	if (odp_pktin_queue_config(pktio, &in_queue_param)) {
		printf("Failed to config input queue for %s\n", name);
		exit(1);
	}

	out_queue_param.op_mode = ODP_PKTIO_OP_MT_UNSAFE;
	if (odp_pktout_queue_config(pktio, &out_queue_param)) {
		printf("Failed to config output queue for %s\n", name);
		exit(1);
	}

/* Get the pktin queue handle in burst mode */
	if (odp_pktin_queue(pktio, pktin, 1) != 1) {
		printf("pktin queue query failed for %s\n", name);
		exit(1);
	}
/* Get the pktout queue handle in burst mode */
	if (odp_pktout_queue(pktio, pktout, 1) != 1) {
		printf("pktout queue query failed for %s\n", name);
		exit(1);
	}
#endif


	ret = odp_pktio_start(pktio);
	if (ret != 0)
		printf("Error: unable to start %s\n", name);

//	odp_pktio_promisc_mode_set (pktio, 1);

	printf("  created pktio:%02" PRIu64
			", dev:%s, queue mode (ATOMIC queues)\n"
			"  \tdefault pktio%02" PRIu64 "\n",
			odp_pktio_to_u64(pktio), name,
			odp_pktio_to_u64(pktio));

	return pktio;
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
           "  E.g. %s -i eth0,eth1,eth2,eth3 \n"
           "\n"
           "Mandatory OPTIONS:\n"
           "  -i, --interface Eth interfaces (comma-separated, no spaces)\n"
           "                  Interface count min 1, max %i\n"
           "\n"
           "Optional OPTIONS:\n"
           "  -c, --count <number> CPU count.\n"
           "  -m, --mode      0: Receive and send directly (no queues)\n"
           "                  1: Receive and send via queues.\n"
           "                  2: Receive via scheduler, send via queues.\n"
           "  -h, --help           Display help and exit.\n\n"
           "\n", NO_PATH(progname), NO_PATH(progname), MAX_PKTIOS
        );
}

/**
 * Parse and store the command line arguments
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
        {"mode", required_argument, NULL, 'm'},     /* return 'm' */
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };

    while (1) {
        opt = getopt_long(argc, argv, "+c:+t:+a:i:m:o:r:d:s:e:h",
                  longopts, &long_index);

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
            switch (i) {
            case 0:
                appl_args->mode = APPL_MODE_PKT_BURST;
                break;
            case 1:
                appl_args->mode = APPL_MODE_PKT_QUEUE;
                break;
            case 2:
                appl_args->mode = APPL_MODE_PKT_SCHED;
                break;
            default:
                usage(argv[0]);
                exit(EXIT_FAILURE);
            }
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
#if 0
	switch (appl_args->mode) {
    case APPL_MODE_PKT_BURST:
        PRINT_APPL_MODE(APPL_MODE_PKT_BURST);
        break;
    case APPL_MODE_PKT_QUEUE:
        PRINT_APPL_MODE(APPL_MODE_PKT_QUEUE);
        break;
    case APPL_MODE_PKT_SCHED:
        PRINT_APPL_MODE(APPL_MODE_PKT_SCHED);
        break;
    }
#endif
    printf("\n\n");
    fflush(NULL);
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
}

uint8_t odpc_initialize(int argc, char **argv)
{
	odph_linux_pthread_t thd;
	odp_pool_t pool;
	odp_pool_param_t params;
	odp_instance_t instance;
	odp_cpumask_t cpumask;
	odph_linux_thr_params_t thr_params;
	char cpumaskstr[ODP_CPUMASK_STR_SIZE];
	int num_workers, i, cpu;

	//parse args
	odp_shm_t shm;

	/* initialize ports */
	int nb_ports = 2;

	/* init ODP  before calling anything else */
	if (odp_init_global(&instance, NULL, NULL)) {
		printf("Error: ODP global init failed.\n");
		exit(1);
	}

	/* init this thread */
	if (odp_init_local(instance, ODP_THREAD_CONTROL)) {
		printf("Error: ODP local init failed.\n");
		exit(1);
	}

	/* Reserve memory for args from shared mem */
	shm = odp_shm_reserve("shm_args", sizeof(mac_global_t),
			ODP_CACHE_LINE_SIZE, 0);
	gconf = odp_shm_addr(shm);

	if (gconf == NULL) {
		printf("Error: shared mem alloc failed.\n");
		//exit(EXIT_FAILURE);
		exit(1);
	}
	gconf_init(gconf);

	/* Parse and store the application arguments */
	parse_args(argc, argv, &gconf->appl);

	/* Print both system and application information */
	print_info(NO_PATH(argv[0]), &gconf->appl);

	/* Default to ODP_MAX_LCORE  unless user specified */
	num_workers = ODP_MAX_LCORE;
	if (gconf->appl.cpu_count)
		num_workers = gconf->appl.cpu_count;
	printf("odp_max num worker threads: %i\n", num_workers);

	/* Get default worker cpumask */
	odp_cpumask_default_worker(&cpumask, num_workers);
	(void)odp_cpumask_to_str(&cpumask, cpumaskstr, sizeof(cpumaskstr));

	gconf->appl.num_workers = num_workers;

	for (i = 0; i < num_workers; i++)
		gconf->mconf[i].thr_idx = i;

	printf("num worker threads: %i\n", num_workers);
	printf("first CPU:          %i\n", odp_cpumask_first(&cpumask));
	printf("cpu mask:           %s\n", cpumaskstr);

	/* create the packet pool */
	odp_pool_param_init(&params);
	params.pkt.seg_len = PKT_POOL_BUF_SIZE;
	params.pkt.len     = PKT_POOL_BUF_SIZE;
	params.pkt.num     = PKT_POOL_SIZE;
	params.type        = ODP_POOL_PACKET;

	pool = odp_pool_create("packet pool", &params);
	if (pool == ODP_POOL_INVALID) {
		printf("Error: packet pool create failed.\n");
		exit(1);
	}
	/* TODO implement this function */
	//    odp_pool_print(pool);

	//  macs->if0 = create_pktie("pcap:in=mac.pcap", pool, &(macs->if0in), &macs->if0out);
	//	macs->if0 = create_pktio("veth1.0", pool, &(macs->if0in), &macs->if0out);
	//	macs->if1 = create_pktio("veth2.1", pool, &(macs->if1in), &macs->if1out);

	/* Create a pktio instance for each interface */
	for (i = 0; i < gconf->appl.if_count; ++i)
	{
		gconf->pktios[i].pktio = create_pktio(gconf->appl.if_names[i], pool, gconf->appl.mode);
		printf("interface id %d, ifname %s, pktio:%02" PRIu64 " \n", i, gconf->appl.if_names[i],odp_pktio_to_u64(gconf->pktios[i].pktio));
	}
	/* Create and init worker threads */
	memset(gconf->thread_tbl, 0, sizeof(gconf->thread_tbl));

	memset(&thr_params, 0, sizeof(thr_params));
	thr_params.thr_type = ODP_THREAD_WORKER;
	thr_params.instance = instance;

	cpu = odp_cpumask_first(&cpumask);
	printf("count threadmask %d\n ", odp_cpumask_count(&cpumask));
	/* number of interface and workers should be same */
	for (i = 0; i < num_workers; ++i) {
		int if_idx;

		void *(*thr_run_func) (void *);

		if_idx = i % gconf->appl.if_count;

		gconf->mconf[i].pktio_dev = gconf->appl.if_names[i];
		gconf->mconf[i].mode = gconf->appl.mode;

		printf("if %s and pktio_dev %s \n ", gconf->appl.if_names[i], gconf->mconf[i].pktio_dev);

		if (gconf->appl.mode == APPL_MODE_PKT_BURST) {
			thr_run_func = launch_worker;
			printf("thread func set to launch_worker\n ");
		}
		/*
		 * Create threads one-by-one instead of all-at-once,
		 * because each thread might get different arguments.
		 * Calls odp_thread_create(cpu) for each thread
		 */

		thr_params.arg      = &gconf->mconf[i];
	    thr_params.start    = thr_run_func;

		odph_linux_pthread_create(&gconf->thread_tbl[i],
				                  &cpumask, &thr_params);
	}

	/* Initialize all the tables defined in p4 src */
	odpc_lookup_tbls_init();

	/* initiate control plane */
	init_control_plane();

	// TODO
	odpc_lcore_conf_init();

	/* Master thread waits for other threads to exit */
	odph_linux_pthread_join(gconf->thread_tbl, num_workers);

//	free(gconf->appl.if_names);
//	free(gconf->appl.if_str);
//	free(gconf);
	printf("Exit\n\n");

	return 0;
}

uint8_t odpc_des ()
{
#if 0
	if (odpc_lookup_tbls_des()) {
		printf("Lookup table destroy failed.\n");
	}


	if (odp_term_local()) {
		printf("Error: ODP local term failed.\n");
		exit(EXIT_FAILURE);
	}

	if (odp_term_global()) {
		printf("Error: ODP global term failed.\n");
		exit(EXIT_FAILURE);
	}
#endif
	return 0;
}

