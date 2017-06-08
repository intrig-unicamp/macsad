#include "odp_lib.h"

int main(int argc, char** argv)
{
#ifdef ODP_BK
    info("Starting MACSAD DataPlane.\n");
    maco_initialize(argc, argv);
    info("Exiting MACSAD Program.\n");
    maco_terminate();
#endif // ODP_BK
    return 0;
}
