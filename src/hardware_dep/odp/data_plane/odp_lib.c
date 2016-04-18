#include "odp_api.h"
#include "aliases.h"
#include "backend.h"
#include "ctrl_plane_backend.h"
#include "dataplane.h"
#include <unistd.h>

#include "odp_lib.h"
#include <odp/helper/linux.h>
#include <odp_api.h>
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

extern void
odp_main_worker (void);
//=============================================================================

#define UNUSED(x) (void)(x)

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
	struct macs_conf *qconf;
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
		qconf = &mconf_list[lcore_id];
		int i;
		for(i = 0; i < NB_TABLES; i++)
			qconf->state.tables[i] = state[socketid].tables[i][0];
		for(i = 0; i < NB_COUNTERS; i++)
			qconf->state.counters[i] = state[socketid].counters[i];
	}
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
    struct macs_conf *conf;
 //   int lcore_id = rte_lcore_id();
	int lcore_id = 0;
    conf = &mconf_list[lcore_id];
//TODO
//    rte_atomic32_inc(&conf->state.counters[counterid]->cnt[index]);
}
 
uint32_t read_counter(int counterid, int index) 
{
    uint32_t cnt = 0;
    int socketid;
//TODO
//    for(socketid = 0; socketid < NB_SOCKETS; socketid++)
//        if(state[socketid].tables[0][0] != NULL)
//            cnt += rte_atomic32_read(&state[socketid].counters[counterid]->cnt[index]);
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

static odp_pktio_t create_pktio(const char *name, odp_pool_t pool,
								odp_pktin_queue_t *pktin,
								odp_pktout_queue_t *pktout)
{
	odp_pktio_param_t pktio_param;
	odp_pktin_queue_param_t in_queue_param;
	odp_pktout_queue_param_t out_queue_param;
	odp_pktio_t pktio;

	odp_pktio_param_init(&pktio_param);

	pktio = odp_pktio_open(name, pool, &pktio_param);
	if (pktio == ODP_PKTIO_INVALID) {
		printf("Failed to open %s\n", name);
		exit(1);
	}

	odp_pktin_queue_param_init(&in_queue_param);
	odp_pktout_queue_param_init(&out_queue_param);

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
	if (odp_pktin_queue(pktio, pktin, 1) != 1) {
		printf("pktin queue query failed for %s\n", name);
		exit(1);
	}
	if (odp_pktout_queue(pktio, pktout, 1) != 1) {
		printf("pktout queue query failed for %s\n", name);
		exit(1);
	}
	return pktio;
}

static void *launch_worker(void *arg ODP_UNUSED)
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

static void gbl_args_init(args_t *args)
{
    int pktio, queue;

    memset(args, 0, sizeof(args_t));

    for (pktio = 0; pktio < MAX_PKTIOS; pktio++) {
        args->pktios[pktio].pktio = ODP_PKTIO_INVALID;

        for (queue = 0; queue < MAX_QUEUES; queue++)
            args->pktios[pktio].rx_q[queue] = ODP_QUEUE_INVALID;
    }
}

uint8_t odpc_initialize(int argc, char **argv)
{
	odp_pool_t pool;
	odp_pool_param_t params;
	odp_cpumask_t cpumask;
	char cpumaskstr[ODP_CPUMASK_STR_SIZE];
	odph_linux_pthread_t thd;
	struct macs_conf *macs = &mconf_list[0];
	int num_workers;
	
	//parse args
	odp_shm_t shm;

	/* initialize ports */
	int nb_ports = 2;

	/* init ODP  before calling anything else */
	if (odp_init_global(NULL, NULL)) {
		printf("Error: ODP global init failed.\n");
		exit(1);
	}

	/* init this thread */
	if (odp_init_local(ODP_THREAD_CONTROL)) {
		printf("Error: ODP local init failed.\n");
		exit(1);
	}

    /* Reserve memory for args from shared mem */
    shm = odp_shm_reserve("shm_args", sizeof(args_t),
                  ODP_CACHE_LINE_SIZE, 0);
    gbl_args = odp_shm_addr(shm);

    if (gbl_args == NULL) {
        LOG_ERR("Error: shared mem alloc failed.\n");
        exit(EXIT_FAILURE);
    }
    gbl_args_init(gbl_args);

    /* Parse and store the application arguments */
    parse_args(argc, argv, &gbl_args->appl);

    /* Print both system and application information */
    print_info(NO_PATH(argv[0]), &gbl_args->appl);

	/* Default to system CPU count unless user specified */
    num_workers = MAX_WORKERS;
    if (gbl_args->appl.cpu_count)
        num_workers = gbl_args->appl.cpu_count;

    /* Get default worker cpumask */
    num_workers = odp_cpumask_default_worker(&cpumask, num_workers);
    (void)odp_cpumask_to_str(&cpumask, cpumaskstr, sizeof(cpumaskstr));

    for (i = 0; i < num_workers; i++)
        gbl_args->thread[i].thr_idx = i;

    if_count = gbl_args->appl.if_count;

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
		LOG_ERR("Error: packet pool create failed.\n");
		printf("Error: packet pool create failed.\n");
		exit(1);
	}	
//    odp_pool_print(pool);

//    macs->if0 = create_pktio("pcap:in=mac.pcap", pool, &(macs->if0in), &macs->if0out);
//    macs->if0 = create_pktio(argv[1], pool, &(macs->if0in), &macs->if0out);
	macs->if0 = create_pktio("veth1.0", pool, &(macs->if0in), &macs->if0out);
//	global.if1 = create_pktio(argv[2], pool, &global.if1in, &global.if1out);	
	macs->if1 = create_pktio("veth2.1", pool, &(macs->if1in), &macs->if1out);	

	/* ToDo */
	/* check if similar thing on ODP */
//	check_all_ports_link_status(nb_ports, enabled_port_mask);

	/* Initialize all the tables defined in p4 src */
	odpc_lookup_tbls_init();

	/* initiate control plane */
	init_control_plane();

	// TODO
	odpc_lcore_conf_init();

	odp_cpumask_default_worker(&cpumask, 1);
	odph_linux_pthread_create(&thd, &cpumask, launch_worker, NULL,
			ODP_THREAD_WORKER);
	odph_linux_pthread_join(&thd, 1);

	return nb_ports;
}

uint8_t odpc_des () 
{
#if 0
	if (odpc_lookup_tbls_des()) {
		printf("Lookup table destroy failed.\n");
	}


	if (odp_term_local()) {
		LOG_ERR("Error: ODP local term failed.\n");
		exit(EXIT_FAILURE);
	}

	if (odp_term_global()) {
		LOG_ERR("Error: ODP global term failed.\n");
		exit(EXIT_FAILURE);
	}
#endif
	return 0;
}

