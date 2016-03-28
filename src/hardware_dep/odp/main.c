#include "odp_lib.h"

extern uint8_t odp_initialize(int argc, char **argv);
struct macs_conf gconf;


int main(int argc, char** argv)
{
#ifdef ODP_BK
    printf("Starting ODP-P4 DP\n");
    odp_initialize(argc, argv);
    printf("Exiting program.\n");
#endif // ODP_BK
    return 0;
}
