#include "odp_lib.h"

//struct macs_conf gconf;
//struct macs_conf macs_conf[ODP_MAX_LCORE];

int main(int argc, char** argv)
{
#ifdef ODP_BK
    printf("Starting ODP-P4 DataPlane\n");
    odpc_initialize(argc, argv);
    printf("Exiting Program.\n");
#endif // ODP_BK
    return 0;
}
