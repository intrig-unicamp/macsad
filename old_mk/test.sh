#!/bin/bash

echo "Script started... "
# set default values for variables if they are not already defined
MAKE_CMD=${MAKE_CMD-make}

# Compile Controller
pwd=`pwd`
echo $pwd
cd ..
cd src/hardware_dep/shared/ctrl_plane
gcc -Wall -pthread -std=c99  handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c mac_controller.c -o $pwd/mac_controller
#gcc -Wall -pthread -std=c99  handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c mac_l3_controller.c -o $pwd/mac_l3_controller
#gcc -Wall -pthread -std=c99  handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c mac_l3_controller_ipv6.c -o $pwd/mac_l3_controller_ipv6
#gcc -Wall -pthread -std=c99  handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c mac_nat_dw.c -o $pwd/mac_nat_dw
#gcc -Wall -pthread -std=c99  handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c mac_nat_up.c -o $pwd/mac_nat_up
cd $pwd/..
echo $(pwd)
#make clean
#make all
#ERROR_CODE=$?
#if [ "$ERROR_CODE" -ne 0 ]; then
#    echo Controller compilation failed with error code $ERROR_CODE
#    exit 1
#fi
#cd -

# Restart mac controller in background
killall mac_controller
killall mac_l2_l3_controller
killall mac_l3_controller
killall mac_l3_nhg_controller
killall mac_l3_controller_ipv6
killall mac_nat_dw
killall mac_nat_up

pkill -f mac_controller
pkill -f mac_l2_l3_controller
pkill -f mac_l3_controller
pkill -f mac_l3_nhg_controller
pkill -f mac_l3_controller_ipv6
pkill -f mac_nat_dw
pkill -f mac_nat_up

./old_mk/mac_controller &
#./old_mk/mac_controller traces/trace_trPR_100_l2.txt &
#./old_mk/mac_l3_controller traces/trace_trPR_100_l3.txt &
#./src/hardware_dep/shared/ctrl_plane/mac_l2_l3_controller &
#./src/hardware_dep/shared/ctrl_plane/mac_l3_controller&
#./src/hardware_dep/shared/ctrl_plane/mac_l3_nhg_controller &
#./old_mk/mac_l3_controller_ipv6 traces/trace_trL3_ipv6_10_random.txt &
#./old_mk/mac_nat_dw traces/trace_trPR_100_random_nat_dw.txt &
#./old_mk/mac_nat_up traces/trace_trPR_100_random_nat_up.txt &

echo "Controller started... "

echo "Creating Datapath Logic from P4 source."
rm -rf build
python src/transpiler.py examples/p4_src/l2_fwd.p4
#python src/transpiler.py examples/p4_src/l2_l3.p4
#python src/transpiler.py examples/p4_src/l3_routing_test.p4
#python src/transpiler.py examples/p4_src/l3_routing_nhg.p4
#python src/transpiler.py examples/p4_src/l3_routing_ipv6.p4
#python src/transpiler.py examples/p4_src/nat.p4

ERROR_CODE=$?
if [ "$ERROR_CODE" -ne 0 ]; then
    echo Transpiler failed with error code $ERROR_CODE
    exit 1
fi

cd $pwd
# Compile C sources
make clean;${MAKE_CMD} -j4

# Remove all temporary files (including unreleased hugepages) created by ODP & macsad
rm -rf /tmp/odp*
#rm /mnt/huge/rte*
#rm /mnt/huge/0/odp*
#rm /dev/hugepages/rte*
#rm /dev/hugepages/0/odp*
