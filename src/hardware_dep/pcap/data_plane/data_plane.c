
#include <stdlib.h>

#include <pcap.h>

#include "standard_metadata.h"


extern void process_packet(u_char* pkt_ptr, struct pcap_pkthdr* pkt_header_ptr);

extern int packet_count_limit;

extern void dbg_printf(const int verbosity_level, const char *fmt, ...);


pcap_t*         ingress_pcap_handle;

int             egress_port_count;
int*            ports;
pcap_dumper_t** egress_pcap_handles;

struct standard_metadata_t* standard_metadata;


// returns the pcap_dumper_t* that corresponds to the port
// or NULL is the port is not found
pcap_dumper_t* find_pcap_handle_by_port(int port)
{
    for (int i = 0; i < egress_port_count; ++i) {
        dbg_printf(1, "dumper %d is for port %d\n", i, ports[i]);

        if (ports[i] == port) {
            return egress_pcap_handles[i];
        }
    }

    return 0;
}

void control_egress_on_port(int port, pcap_dumper_t* dumper, u_char* pkt_ptr, struct pcap_pkthdr* pkt_header_ptr)
{
    pcap_dump((u_char*)dumper, pkt_header_ptr, (const u_char*)pkt_ptr);
}

// Egresses a packet: writes it to the appropriate pcap file(s).
void control_egress(u_char* pkt_ptr, struct pcap_pkthdr* pkt_header_ptr)
{
    dbg_printf(1, "preparing egress for port %d\n", standard_metadata->egress_spec);

    pcap_dumper_t* dumper = find_pcap_handle_by_port(standard_metadata->egress_spec);

    if (dumper) {
        dbg_printf(0, "egress port found, sending packet to port %d\n", standard_metadata->egress_spec);
        int port = standard_metadata->egress_spec;
        control_egress_on_port(port, dumper, pkt_ptr, pkt_header_ptr);
    } else {
        dbg_printf(0, "egress: port %d not found, broadcasting packet...\n", standard_metadata->egress_spec);

        for (int i = 0; i < egress_port_count; ++i) {
            int bcast_port = ports[i];
            pcap_dumper_t* bcast_dumper = egress_pcap_handles[i];
            control_egress_on_port(bcast_port, bcast_dumper, pkt_ptr, pkt_header_ptr);
        }
    }

    dbg_printf(1, "egress done\n");
    dbg_printf(0, "\n");
}

// Since we use pcap for ingress, there are no real ports.
// This method to imitates them.
void ingress_set_metadata_for_packet(u_char* pkt_ptr, int pkt_number)
{
    dbg_printf(1, "setting standard_metadata->ingress_port %d\n", 1000 + pkt_number % 3);
    standard_metadata->ingress_port = 1000 + pkt_number % 3;
}

void run_data_plane_pipeline()
{
    int pkt_number = 0;

    const u_char* packet;
    struct pcap_pkthdr pkt_header;

    while ((packet = pcap_next(ingress_pcap_handle, &pkt_header))) {
        u_char* pkt_ptr = (u_char*)packet;
        ingress_set_metadata_for_packet(pkt_ptr, pkt_number);
        process_packet(pkt_ptr, &pkt_header);

        ++pkt_number;

        if (packet_count_limit != -1 && pkt_number == packet_count_limit) break;
    }

    for (int i = 0; i < egress_port_count; ++i) {
        pcap_dump_flush(egress_pcap_handles[i]);
    }
}


void init_ingress(char* ingress_pcap_filename)
{
    standard_metadata = malloc(sizeof(struct standard_metadata_t));

    char errbuf[PCAP_ERRBUF_SIZE];
    ingress_pcap_handle = pcap_open_offline(ingress_pcap_filename, errbuf);

    if (!ingress_pcap_handle) {
          fprintf(stderr,"Couldn't open ingress pcap file %s: %s\n", ingress_pcap_filename, errbuf);
          exit(1);
    }
}


void init_egress_pcap(int idx, char* egress_pcap_filename)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    egress_pcap_handles[idx] = pcap_dump_open(ingress_pcap_handle, egress_pcap_filename);

    if (!egress_pcap_handles[idx]) {
          fprintf(stderr,"Couldn't open egress pcap file %s: %s\n", egress_pcap_filename, errbuf);
          exit(1);
    }
}

// a pcap_dumper_t* is created for all ports
void init_egress(int egress_port_count_par, char** egress_ports, char* cwd)
{
    egress_port_count = egress_port_count_par;

    egress_pcap_handles = malloc(egress_port_count * sizeof(struct pcap_dumper_t*));
    ports = malloc(egress_port_count * sizeof(int));

    for (int i = 0; i < egress_port_count; ++i) {
        ports[i] = atoi(egress_ports[i]);

        char filename[256];
        sprintf(filename, "%s/output/egress%d.pcap", cwd, ports[i]);

        init_egress_pcap(i, filename);
    }
}

void init_data_plane(char* ingress_pcap_filename, int egress_port_count_par, char** egress_ports, char* cwd)
{
    init_ingress(ingress_pcap_filename);
    init_egress(egress_port_count_par, egress_ports, cwd);
}
