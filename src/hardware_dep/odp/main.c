#include "odp_lib.h"

int main(int argc, char** argv)
{
#ifdef ODP_BK
    info("Starting MACSAD DataPlane.\n");
    odpc_initialize(argc, argv);
    info("Exiting MACSAD Program.\n");
#endif // ODP_BK
    return 0;
}
