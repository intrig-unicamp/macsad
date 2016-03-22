#include "dpdk_lib.h"

extern uint8_t initialize(int argc, char **argv);
extern void init_control_plane();
extern int launch_dpdk();

int main(int argc, char** argv)
{
    initialize(argc, argv);
    init_control_plane();
    int retval = launch_dpdk();
    printf("Exiting program.\n");
    return retval;
}
