#include "dpdk_lib.h"
#include "aliases.h"
#include "backend.h"
#include "ctrl_plane_backend.h"
#include "dataplane.h"
#include <unistd.h>

//=   shared   ================================================================

extern __m128i val_eth[RTE_MAX_ETHPORTS];
extern struct rte_mempool * pktmbuf_pool[NB_SOCKETS];
extern struct rte_mempool *header_pool, *clone_pool;
extern struct lcore_conf lcore_conf[RTE_MAX_LCORE];


extern uint32_t enabled_port_mask;
int promiscuous_on = 0; /**< Ports set in promiscuous mode off by default. */
int numa_on = 1; /**< NUMA is enabled by default. */

#define MAX_LCORE_PARAMS 1024
struct lcore_params {
	uint8_t port_id;
	uint8_t queue_id;
	uint8_t lcore_id;
} __rte_cache_aligned;
struct lcore_params lcore_params_array[MAX_LCORE_PARAMS];
struct lcore_params lcore_params_array_default[];
struct lcore_params * lcore_params;
uint16_t nb_lcore_params;
struct lcore_params lcore_params_array[MAX_LCORE_PARAMS];
struct lcore_params lcore_params_array_default[] = {
	{0, 0, 2},
	{0, 1, 2},
	{0, 2, 2},
	{1, 0, 2},
	{1, 1, 2},
	{1, 2, 2},
	{2, 0, 2},
	{3, 0, 3},
	{3, 1, 3},
};

struct lcore_params * lcore_params = lcore_params_array_default;
uint16_t nb_lcore_params = sizeof(lcore_params_array_default) /
				sizeof(lcore_params_array_default[0]);

struct l2fwd_port_statistics port_statistics[RTE_MAX_ETHPORTS];

//=   used only here   ========================================================

struct lcore_queue_conf lcore_queue_conf[RTE_MAX_LCORE];

unsigned int rx_queue_per_lcore = 1;

#define MAX_JUMBO_PKT_LEN  9600

#define RTE_TEST_RX_DESC_DEFAULT 128
#define RTE_TEST_TX_DESC_DEFAULT 512
static uint16_t nb_rxd = RTE_TEST_RX_DESC_DEFAULT;
static uint16_t nb_txd = RTE_TEST_TX_DESC_DEFAULT;

static struct rte_eth_conf port_conf = {
	.rxmode = {
		.mq_mode = ETH_MQ_RX_RSS,
		.max_rx_pkt_len = ETHER_MAX_LEN,
		.split_hdr_size = 0,
		.header_split   = 0, /**< Header Split disabled */
		.hw_ip_checksum = 1, /**< IP checksum offload enabled */
		.hw_vlan_filter = 0, /**< VLAN filtering disabled */
		.jumbo_frame    = 0, /**< Jumbo Frame Support disabled */
		.hw_strip_crc   = 0, /**< CRC stripped by hardware */
	},
	.rx_adv_conf = {
		.rss_conf = {
			.rss_key = NULL,
			.rss_hf = ETH_RSS_IP,
		},
	},
	.txmode = {
		.mq_mode = ETH_MQ_TX_NONE,
	},
};

//=============================================================================

#define UNUSED(x) (void)(x)

static int
check_lcore_params(void)
{
	uint8_t lcore;
	uint16_t i;
	int socketid;

	for (i = 0; i < nb_lcore_params; ++i) {
		uint8_t queue = lcore_params[i].queue_id;

		if (queue >= MAX_RX_QUEUE_PER_PORT) {
			printf("queue number %hhu over maximum receiving queues per port (%hhu)\n", queue, MAX_RX_QUEUE_PER_PORT);
			return -1;
		}

		lcore = lcore_params[i].lcore_id;

		if (!rte_lcore_is_enabled(lcore)) {
			printf("error: lcore %hhu is not enabled in lcore mask\n", lcore);
			return -1;
		}

		if ((socketid = rte_lcore_to_socket_id(lcore) != 0) &&
			(numa_on == 0)) {
			printf("warning: lcore %hhu is on socket %d with numa off \n",
				lcore, socketid);
		}
	}
	return 0;
}

static int
check_port_config(const unsigned nb_ports)
{
	unsigned portid;
	uint16_t i;

	for (i = 0; i < nb_lcore_params; ++i) {
		portid = lcore_params[i].port_id;

		if ((enabled_port_mask & (1 << portid)) == 0) {
			printf("port %u is not enabled in port mask\n", portid);
			return -1;
		}
		if (portid >= nb_ports) {
			printf("port %u is not present on the board\n", portid);
			return -1;
		}
	}
	return 0;
}

static uint8_t
get_port_n_rx_queues(const uint8_t port)
{
	int queue = -1;
	uint16_t i;

	for (i = 0; i < nb_lcore_params; ++i) {
		if (lcore_params[i].port_id == port && lcore_params[i].queue_id > queue)
			queue = lcore_params[i].queue_id;
	}
	return (uint8_t)(++queue);
}

static int
init_lcore_rx_queues(void)
{
	uint16_t i, nb_rx_queue;
	uint8_t lcore;

	for (i = 0; i < nb_lcore_params; ++i) {
		lcore = lcore_params[i].lcore_id;
		nb_rx_queue = lcore_conf[lcore].n_rx_queue;

		if (nb_rx_queue >= MAX_RX_QUEUE_PER_LCORE) {
			printf("error: too many queues (%u) for lcore %u (max: %u)\n",
				(unsigned)nb_rx_queue + 1, (unsigned)lcore, MAX_RX_QUEUE_PER_LCORE);
			return -1;
		}

		lcore_conf[lcore].rx_queue_list[nb_rx_queue].port_id =
			lcore_params[i].port_id;
		lcore_conf[lcore].rx_queue_list[nb_rx_queue].queue_id =
			lcore_params[i].queue_id;
		lcore_conf[lcore].n_rx_queue++;
	}
	return 0;
}

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
	int opt, ret;
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
	return ret;
}

static void
print_ethaddr(const char *name, const struct ether_addr *eth_addr)
{
	char buf[ETHER_ADDR_FMT_SIZE];
	ether_format_addr(buf, ETHER_ADDR_FMT_SIZE, eth_addr);
	printf("%s%s", name, buf);
}

/* Check the link status of all ports in up to 9s, and print them finally */
static void
check_all_ports_link_status(uint8_t port_num, uint32_t port_mask)
{
#define CHECK_INTERVAL 100 /* 100ms */
#define MAX_CHECK_TIME 90 /* 9s (90 * 100ms) in total */
	uint8_t portid, count, all_ports_up, print_flag = 1;
	struct rte_eth_link link;

	printf("\nChecking link status \n");
	fflush(stdout);
	for (count = 0; count <= MAX_CHECK_TIME; count++) {
		all_ports_up = 1;
		for (portid = 0; portid < port_num; portid++) {
			if ((port_mask & (1 << portid)) == 0)
				continue;
			memset(&link, 0, sizeof(link));
			rte_eth_link_get_nowait(portid, &link);
			/* print link status if flag set */
			if (print_flag == 1) {
				if (link.link_status)
					printf("Port %d Link Up - speed %u "
						"Mbps - %s\n", (uint8_t)portid,
						(unsigned)link.link_speed,
				(link.link_duplex == ETH_LINK_FULL_DUPLEX) ?
					("full-duplex") : ("half-duplex\n"));
				else
					printf("Port %d Link Down\n",
						(uint8_t)portid);
				continue;
			}
			/* clear all_ports_up flag if any link down */
			if (link.link_status == 0) {
				all_ports_up = 0;
				break;
			}
		}
		/* after finally printing all link status, get out */
		if (print_flag == 1)
			break;

		if (all_ports_up == 0) {
			printf(".");
			fflush(stdout);
			rte_delay_ms(CHECK_INTERVAL);
		}

		/* set the print_flag if all ports up or timeout */
		if (all_ports_up == 1 || count == (MAX_CHECK_TIME - 1)) {
			print_flag = 1;
			printf("done\n");
		}
	}
}

static void
create_tables_on_socket(int socketid)
{
    if(table_config == NULL) return;
    printf("Initializing tables on socket %d\n", socketid);
    int i;
    for(i = 0; i < NB_TABLES; i++) {
        printf("Creating table %d on socket %d (%d copies)\n", i, socketid, NB_REPLICA);
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

static int
get_socketid(unsigned lcore_id)
{
	if (numa_on)
		return rte_lcore_to_socket_id(lcore_id);
	else
		return 0;
}

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

int init_lookup_tables()
{
    printf("Initializing tables...\n");
	struct lcore_conf *qconf;
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
		if (tables_on_sockets[socketid][0][0] == NULL)
            create_tables_on_socket(socketid);
		qconf = &lcore_conf[lcore_id];
        int i;
        for(i = 0; i < NB_TABLES; i++)
    		qconf->tables[i] = tables_on_sockets[socketid][i][0];
	}
    printf("Initializing tables Done.\n");
	return 0;
}

static void change_replica(int socketid, int tid, int replica) {
    struct lcore_conf *qconf;
    int core_socketid;
    unsigned lcore_id;
    for (lcore_id = 0; lcore_id < RTE_MAX_LCORE; lcore_id++) {
        if (rte_lcore_is_enabled(lcore_id) == 0) continue;
        core_socketid = rte_lcore_to_socket_id(lcore_id);
        if(core_socketid != socketid) continue;
        qconf = &lcore_conf[lcore_id];
   	    qconf->tables[tid] = tables_on_sockets[socketid][tid][replica]; // TODO should this be atomic?
        active_replica[socketid][tid] = replica;
        //printf("\n\n\nCHANGING REPLICA of TABLE %d: core %d on socket %d now uses replica %d\n\n\n", tid, lcore_id, socketid, replica);
    }
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


uint8_t
initialize(int argc, char **argv)
{
	struct lcore_queue_conf *qconf;
	int ret;
	uint8_t nb_ports;
	uint8_t nb_ports_available;
	uint8_t portid;
	unsigned rx_lcore_id;
	/* init EAL */
	ret = rte_eal_init(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid EAL arguments\n");
	argc -= ret;
	argv += ret;

	/* parse application arguments (after the EAL ones) */
	ret = parse_args(argc, argv);
	if (ret < 0)
		rte_exit(EXIT_FAILURE, "Invalid L2FWD arguments\n");

	int socketid = get_socketid(rx_lcore_id);

	/* create the mbuf pool */
	pktmbuf_pool[socketid] =
		rte_mempool_create("mbuf_pool", NB_MBUF,
				   MBUF_SIZE, 32,
				   sizeof(struct rte_pktmbuf_pool_private),
				   rte_pktmbuf_pool_init, NULL,
				   rte_pktmbuf_init, NULL,
				   rte_socket_id(), 0);
	if (pktmbuf_pool[socketid] == NULL)
		rte_exit(EXIT_FAILURE, "Cannot init mbuf pool\n");

	nb_ports = rte_eth_dev_count();
	//if (nb_ports == 0)
	//	rte_exit(EXIT_FAILURE, "No Ethernet ports - bye\n");

	if (nb_ports > RTE_MAX_ETHPORTS)
		nb_ports = RTE_MAX_ETHPORTS;

	rx_lcore_id = 0;
	qconf = NULL;

	/* Initialize the port/queue configuration of each logical core */
	for (portid = 0; portid < nb_ports; portid++) {
		/* skip ports that are not enabled */
		if ((enabled_port_mask & (1 << portid)) == 0)
			continue;

		/* get the lcore_id for this port */
		while (rte_lcore_is_enabled(rx_lcore_id) == 0 ||
		       lcore_queue_conf[rx_lcore_id].n_rx_port == rx_queue_per_lcore) {
			rx_lcore_id++;
			if (rx_lcore_id >= RTE_MAX_LCORE)
				rte_exit(EXIT_FAILURE, "Not enough cores\n");
		}

		if (qconf != &lcore_queue_conf[rx_lcore_id])
			/* Assigned a new logical core in the loop above. */
			qconf = &lcore_queue_conf[rx_lcore_id];

		qconf->rx_port_list[qconf->n_rx_port] = portid;
		qconf->n_rx_port++;
		printf("Lcore %u: RX port %u\n", rx_lcore_id, (unsigned) portid);
	}

	nb_ports_available = nb_ports;

	/* Initialise each port */
	for (portid = 0; portid < nb_ports; portid++) {
		/* skip ports that are not enabled */
		if ((enabled_port_mask & (1 << portid)) == 0) {
			printf("Skipping disabled port %u\n", (unsigned) portid);
			nb_ports_available--;
			continue;
		}
		/* init port */
		printf("Initializing port %u... ", (unsigned) portid);
		fflush(stdout);
		ret = rte_eth_dev_configure(portid, 1, 1, &port_conf);
		if (ret < 0)
			rte_exit(EXIT_FAILURE, "Cannot configure device: err=%d, port=%u\n",
				  ret, (unsigned) portid);

		rte_eth_macaddr_get(portid,&ports_eth_addr[portid]);

		/* init one RX queue */
		fflush(stdout);
		ret = rte_eth_rx_queue_setup(portid, 0, nb_rxd,
					     rte_eth_dev_socket_id(portid),
					     NULL,
					     pktmbuf_pool[socketid]);
		if (ret < 0)
			rte_exit(EXIT_FAILURE, "rte_eth_rx_queue_setup:err=%d, port=%u\n",
				  ret, (unsigned) portid);

		/* init one TX queue on each port */
		fflush(stdout);
		ret = rte_eth_tx_queue_setup(portid, 0, nb_txd,
				rte_eth_dev_socket_id(portid),
				NULL);
		if (ret < 0)
			rte_exit(EXIT_FAILURE, "rte_eth_tx_queue_setup:err=%d, port=%u\n",
				ret, (unsigned) portid);

		/* Start device */
		ret = rte_eth_dev_start(portid);
		if (ret < 0)
			rte_exit(EXIT_FAILURE, "rte_eth_dev_start:err=%d, port=%u\n",
				  ret, (unsigned) portid);

		printf("done: \n");

		rte_eth_promiscuous_enable(portid);

		printf("Port %u, MAC address: %02X:%02X:%02X:%02X:%02X:%02X\n\n",
				(unsigned) portid,
				ports_eth_addr[portid].addr_bytes[0],
				ports_eth_addr[portid].addr_bytes[1],
				ports_eth_addr[portid].addr_bytes[2],
				ports_eth_addr[portid].addr_bytes[3],
				ports_eth_addr[portid].addr_bytes[4],
				ports_eth_addr[portid].addr_bytes[5]);

		/* initialize port stats */
		memset(&port_statistics, 0, sizeof(port_statistics));
	}

	//if (!nb_ports_available) {
	//	rte_exit(EXIT_FAILURE,
	//		"All available ports are disabled. Please set portmask.\n");
	//}

	check_all_ports_link_status(nb_ports, enabled_port_mask);

    init_lookup_tables(); 

	return nb_ports;
}

