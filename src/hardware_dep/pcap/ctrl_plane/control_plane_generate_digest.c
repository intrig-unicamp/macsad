
#include "standard_metadata.h"
#include "generated_metadata.h"

#include <stdlib.h>

extern struct standard_metadata_t* standard_metadata;
extern long mac_to_long(unsigned char* mac);

const int MAC_LEARN_RECEIVER = 1024;

extern void send_msg_to_control_plane(unsigned char* mac, int ingress_port);
extern void recv_msg_from_control_plane();

extern void dbg_printf(const int verbosity_level, const char *fmt, ...);


static void data_plane_learn_mac(unsigned char* mac, int ingress_port)
{
    send_msg_to_control_plane(mac, ingress_port);
    recv_msg_from_control_plane();
}

static void generate_mac_learn_digest(int receiver, union any_fieldlist_t* any_fieldlist)
{
	if (receiver == MAC_LEARN_RECEIVER) {
	    const struct fieldlist_mac_learn_digest_t* fieldlist_mac_learn_digest = &any_fieldlist->fieldlist_mac_learn_digest;

	    dbg_printf(1, "Digesting dmac %012lX -> port %d\n", mac_to_long(fieldlist_mac_learn_digest->ethernet_srcAddr), standard_metadata->ingress_port);

	    data_plane_learn_mac(fieldlist_mac_learn_digest->ethernet_srcAddr, standard_metadata->ingress_port);
	}
}

// NOTE: standard_metadata does not need to be explicitly passed to this function, as it is readily available
void generate_digest(int receiver, union any_fieldlist_t* any_fieldlist)
{
    if (receiver == MAC_LEARN_RECEIVER) {
        generate_mac_learn_digest(receiver, any_fieldlist);
    }
}
