
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pcap.h>


// Configuration option. Debug messages above this verbosity level are not printed.
int DBG_VERBOSITY = -1;


extern void init_data_plane(char* ingress_pcap_filename, int egress_port_count_par, char** egress_ports, char* cwd);
extern void run_data_plane_pipeline();
extern void init_control_plane(const char* config_file_filename);


// Only this many packets are processed; -1 indicates no limit.
int packet_count_limit = -1;


int main(int argc, char** argv)
{
    char* DBG_VERBOSITY_ENV = getenv("DBG_VERBOSITY");
    if (DBG_VERBOSITY_ENV) {
        DBG_VERBOSITY = atoi(DBG_VERBOSITY_ENV);
    }

    if (argc <= 3) {
        fprintf(stderr, "Usage: %s <config file> <in pcap> <out (port pcap)s>\n", argv[0]);
        exit(1);
    }

    char*  config_file_filename  = argv[1];
    char*  ingress_pcap_filename = argv[2];

    int    egress_port_count = argc - 3;
    char** egress_ports = argv + 3;

    char   cwdbuf[256];
    char* cwd = getcwd(cwdbuf, sizeof(cwdbuf));

    init_data_plane(ingress_pcap_filename, egress_port_count, egress_ports, cwd);
    init_control_plane(config_file_filename);

    run_data_plane_pipeline();

    return 0;
}
