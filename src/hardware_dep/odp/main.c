#include "odp_lib.h"

int main(int argc, char** argv)
{
    info("Starting MACSAD DataPlane.\n");
    maco_initialize(argc, argv);
    info("Exiting MACSAD Program.\n");
    maco_terminate();
    return 0;
}
