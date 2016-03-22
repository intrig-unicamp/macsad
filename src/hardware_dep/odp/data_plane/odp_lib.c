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

//=   shared   ================================================================

extern void init_control_plane();
//extern __m128i val_eth[MAX_ETHPORTS];

extern uint32_t enabled_port_mask;
int promiscuous_on = 0; /**< Ports set in promiscuous mode off by default. */
int numa_on = 1; /**< NUMA is enabled by default. */

#define MAX_LCORE_PARAMS 1024
uint16_t nb_lcore_params;

struct l2fwd_port_statistics port_statistics[MAX_ETHPORTS];

//=   used only here   ========================================================

#define MAX_JUMBO_PKT_LEN  9600

#define RTE_TEST_RX_DESC_DEFAULT 128
#define RTE_TEST_TX_DESC_DEFAULT 512
static uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
static uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;

/* odp */
#define POOL_NUM_PKT 8192
#define POOL_SEG_LEN 1856
#define MAX_PKT_BURST 32

struct {
        odp_pktio_t if0, if1;
        odp_pktin_queue_t if0in, if1in;
        odp_pktout_queue_t if0out, if1out;
        odph_ethaddr_t src, dst;
} global;

extern void
odp_main_worker (void);

/*
void init_control_plane() {
    printf("init control plane stub");
}
*/
//=============================================================================

#define UNUSED(x) (void)(x)

/* display usage */
static void
print_usage(const char *prgname)
{
	printf ("%s [EAL options] -- -p PORTMASK -P"
		"  [--config (port,queue,lcore)[,(port,queue,lcore]]"
		"  [--enable-jumbo [--max-pkt-len PKTLEN]]\n"
		"  -p PORTMASK: hexadecimal bitmask of ports to configure\n"
		"  -P : enable promiscuous mode\n"
		"  --config (port,queue,lcore): rx queues configuration\n"
		"  --no-numa: optional, disable numa awareness\n"
		"  --enable-jumbo: enable jumbo frame"
		" which max packet len is PKTLEN in decimal (64-9600)\n"
		"  --hash-entry-num: specify the hash entry number in hexadecimal to be setup\n",
		prgname);
}

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

static int
parse_portmask(const char *portmask)
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

static int
parse_config(const char *q_arg)
{
#if 0
	char s[256];
	const char *p, *p0 = q_arg;
	char *end;
	enum fieldnames {
		FLD_PORT = 0,
		FLD_QUEUE,
		FLD_LCORE,
		_NUM_FLD
	};
	unsigned long int_fld[_NUM_FLD];
	char *str_fld[_NUM_FLD];
	int i;
	unsigned size;

	nb_lcore_params = 0;

	while ((p = strchr(p0,'(')) != NULL) {
		++p;
		if((p0 = strchr(p,')')) == NULL)
			return -1;

		size = p0 - p;
		if(size >= sizeof(s))
			return -1;

		snprintf(s, sizeof(s), "%.*s", size, p);
		if (rte_strsplit(s, sizeof(s), str_fld, _NUM_FLD, ',') != _NUM_FLD)
			return -1;
		for (i = 0; i < _NUM_FLD; i++){
			errno = 0;
			int_fld[i] = strtoul(str_fld[i], &end, 0);
			if (errno != 0 || end == str_fld[i] || int_fld[i] > 255)
				return -1;
		}
		if (nb_lcore_params >= MAX_LCORE_PARAMS) {
			printf("exceeded max number of lcore params: %hu\n",
				nb_lcore_params);
			return -1;
		}
		lcore_params_array[nb_lcore_params].port_id = (uint8_t)int_fld[FLD_PORT];
		lcore_params_array[nb_lcore_params].queue_id = (uint8_t)int_fld[FLD_QUEUE];
		lcore_params_array[nb_lcore_params].lcore_id = (uint8_t)int_fld[FLD_LCORE];
		++nb_lcore_params;
	}
	lcore_params = lcore_params_array;
#endif
	return 0;
}

#define CMD_LINE_OPT_CONFIG "config"
#define CMD_LINE_OPT_NO_NUMA "no-numa"
#define CMD_LINE_OPT_ENABLE_JUMBO "enable-jumbo"
#define CMD_LINE_OPT_HASH_ENTRY_NUM "hash-entry-num"

/* Parse the argument given in the command line of the application */
static int
parse_args(int argc, char **argv)
{
	int ret;
#if 0
	int opt;
	char **argvopt;
	int option_index;
	char *prgname = argv[0];
	static struct option lgopts[] = {
		{CMD_LINE_OPT_CONFIG, 1, 0, 0},
		{CMD_LINE_OPT_NO_NUMA, 0, 0, 0},
		{CMD_LINE_OPT_ENABLE_JUMBO, 0, 0, 0},
		{CMD_LINE_OPT_HASH_ENTRY_NUM, 1, 0, 0},
		{NULL, 0, 0, 0}
	};

	argvopt = argv;

	while ((opt = getopt_long(argc, argvopt, "p:P",
				lgopts, &option_index)) != EOF) {

		switch (opt) {
		/* portmask */
		case 'p':
			enabled_port_mask = parse_portmask(optarg);
			if (enabled_port_mask == 0) {
				printf("invalid portmask\n");
				print_usage(prgname);
				return -1;
			}
			break;
		case 'P':
			printf("Promiscuous mode selected\n");
			promiscuous_on = 1;
			break;

		/* long options */
		case 0:
			if (!strncmp(lgopts[option_index].name, CMD_LINE_OPT_CONFIG,
				sizeof (CMD_LINE_OPT_CONFIG))) {
				ret = parse_config(optarg);
				if (ret) {
					printf("invalid config\n");
					print_usage(prgname);
					return -1;
				}
			}

			if (!strncmp(lgopts[option_index].name, CMD_LINE_OPT_NO_NUMA,
				sizeof(CMD_LINE_OPT_NO_NUMA))) {
				printf("numa is disabled \n");
				numa_on = 0;
			}

			if (!strncmp(lgopts[option_index].name, CMD_LINE_OPT_ENABLE_JUMBO,
				sizeof (CMD_LINE_OPT_ENABLE_JUMBO))) {
				struct option lenopts = {"max-pkt-len", required_argument, 0, 0};

				printf("jumbo frame is enabled - disabling simple TX path\n");
				port_conf.rxmode.jumbo_frame = 1;

				/* if no max-pkt-len set, use the default value ETHER_MAX_LEN */
				if (0 == getopt_long(argc, argvopt, "", &lenopts, &option_index)) {
					ret = parse_max_pkt_len(optarg);
					if ((ret < 64) || (ret > MAX_JUMBO_PKT_LEN)){
						printf("invalid packet length\n");
						print_usage(prgname);
						return -1;
					}
					port_conf.rxmode.max_rx_pkt_len = ret;
				}
				printf("set jumbo frame max packet length to %u\n",
						(unsigned int)port_conf.rxmode.max_rx_pkt_len);
			}
			break;

		default:
			print_usage(prgname);
			return -1;
		}
	}

	if (optind >= 0)
		argv[optind-1] = prgname;

	ret = optind-1;
	optind = 0; /* reset getopt lib */
#endif 
	return ret;
}

static void
print_ethaddr(const char *name, const struct ether_addr *eth_addr)
{
#if 0
	char buf[ETHER_ADDR_FMT_SIZE];
	ether_format_addr(buf, ETHER_ADDR_FMT_SIZE, eth_addr);
	printf("%s%s", name, buf);
#endif
}

static void
create_tables_on_socket (int socketid)
{
	// if the table is defined from p4
	if (table_config == NULL) return;
	int i;
	for (i=0;i < NB_TABLES; i++) {
		printf("creting table %d \n", i);
		lookup_table_t t = table_config[i];
		int j;
		for(j = 0; j < NB_REPLICA; j++) {
			tables_on_sockets[socketid][i][j] = malloc(sizeof(lookup_table_t));
			memcpy(tables_on_sockets[socketid][i][j], &t, sizeof(lookup_table_t));
			table_create(tables_on_sockets[socketid][i][j], socketid);
		}
		active_replica[socketid][i] = 0;
	}
}

int init_lookup_tables()
{
  int socketid = SOCKET_DEF;
  printf("Initializing tables...\n");
  if (tables_on_sockets[socketid][0][0] == NULL)
//	  create_tables_on_socket(socketid);
    printf("Initializing tables Done.\n");
    return 0;
}

static void change_replica(int socketid, int tid, int replica) {
//stub code
}

#define CHANGE_TABLE(fun, par...) \
{ \
    int current_replica = active_replica[socketid][tableid]; \
    int next_replica = (current_replica+1)%NB_REPLICA; \
    fun(tables_on_sockets[socketid][tableid][next_replica], par); \
    change_replica(socketid, tableid, next_replica); \
    usleep(TABCHANGE_DELAY); \
    for(current_replica = 0; current_replica < NB_REPLICA; current_replica++) \
        if(current_replica != next_replica) fun(tables_on_sockets[socketid][tableid][current_replica], par); \
}

#define FORALLNUMANODES(b) \
    int socketid; \
    for(socketid = 0; socketid < NB_SOCKETS; socketid++) \
        if(tables_on_sockets[socketid][0][0] != NULL) \
            b
void
exact_add_promote(int tableid, uint8_t* key, uint8_t* value) {
    FORALLNUMANODES(CHANGE_TABLE(exact_add, key, value))
}
void
lpm_add_promote(int tableid, uint8_t* key, uint8_t depth, uint8_t* value) {
    FORALLNUMANODES(CHANGE_TABLE(lpm_add, key, depth, value))
}
void
ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value) {
    FORALLNUMANODES(CHANGE_TABLE(ternary_add, key, mask, value))
}
void
table_setdefault_promote(int tableid, uint8_t* value) {
    FORALLNUMANODES(CHANGE_TABLE(table_setdefault, value))
}

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

static void
*launch_worker(void *arg ODP_UNUSED)
{
        odp_main_worker();
        return NULL;
}

uint8_t
odp_initialize(int argc, char **argv)
{
        odp_pool_t pool;
        odp_pool_param_t params;
        odp_cpumask_t cpumask;
        odph_linux_pthread_t thd;

	/* initialize ports */
	int nb_ports = 2;

	/* init ODP */
        if (odp_init_global(NULL, NULL)) {
                printf("Error: ODP global init failed.\n");
                exit(1);
        }
        if (odp_init_local(ODP_THREAD_CONTROL)) {
                printf("Error: ODP local init failed.\n");
                exit(1);
        }

	/* create the packet pool */
        odp_pool_param_init(&params);
        params.pkt.seg_len = POOL_SEG_LEN;
        params.pkt.len     = POOL_SEG_LEN;
        params.pkt.num     = POOL_NUM_PKT;
        params.type        = ODP_POOL_PACKET;

        pool = odp_pool_create("packet pool", &params);
        if (pool == ODP_POOL_INVALID) {
                printf("Error: packet pool create failed.\n");
                exit(1);
        }	

        //global.if0 = create_pktio(argv[1], pool, &global.if0in, &global.if0out);
        global.if0 = create_pktio("eth1", pool, &global.if0in, &global.if0out);
        //global.if1 = create_pktio(argv[2], pool, &global.if1in, &global.if1out);	
        global.if1 = create_pktio("eth2", pool, &global.if1in, &global.if1out);	

/* ToDo dpdk */
/* check if similar thing on ODP */
//	check_all_ports_link_status(nb_ports, enabled_port_mask);
/* for now only exact table is possible on ODP-hash table */
	init_lookup_tables();
/* dpdk */	

/* initiate control plane */
	init_control_plane();

        odp_cpumask_default_worker(&cpumask, 1);
        odph_linux_pthread_create(&thd, &cpumask, launch_worker, NULL,
                                  ODP_THREAD_WORKER);
        odph_linux_pthread_join(&thd, 1);

	return nb_ports;
}
