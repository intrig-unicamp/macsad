#!/bin/bash

echo "Script started... "
# set default values for variables if they are not already defined
MAKE_CMD=${MAKE_CMD-make}

# Compile Controller
pwd=`pwd`
echo $pwd
cd ..
cd src/hardware_dep/shared/ctrl_plane
#gcc -Wall -pthread -std=c99  handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c mac_l3_controller_ipv6.c -o $pwd/mac_l3_controller_ipv6
gcc -Wall -pthread -std=c99  handlers.c controller.c messages.c sock_helpers.c threadpool.c fifo.c mac_controller.c -o $pwd/mac_controller
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
pkill -f mac_controller
pkill -f mac_l2_l3_controller
pkill -f mac_l3_controller
pkill -f mac_l3_nhg_controller
pkill -f mac_l3_controller_ipv6

#./ctrl/mac_controller &

./old_mk/mac_controller &
#./src/hardware_dep/shared/ctrl_plane/mac_l2_l3_controller &
#./src/hardware_dep/shared/ctrl_plane/mac_l3_controller traces/trace_trPR_100_l3.txt &
#./src/hardware_dep/shared/ctrl_plane/mac_l3_controller&
#./src/hardware_dep/shared/ctrl_plane/mac_l3_nhg_controller &
#./src/hardware_dep/shared/ctrl_plane/mac_l3_controller_ipv6 &
#./src/hardware_dep/shared/ctrl_plane/mac_l3_controller_ipv6 traces/trace_trL3_ipv6_10_random.txt &
#./old_mk/mac_l3_controller_ipv6 traces/trace_trL3_ipv6_10_random.txt &

echo "Controller started... "

echo "Creating Datapath Logic from P4 source."
rm -rf build
python src/transpiler.py examples/p4_src/l2_fwd.p4
#python src/transpiler.py examples/p4_src/l2_l3.p4
#python src/transpiler.py examples/p4_src/l3_routing_test.p4
#python src/transpiler.py examples/p4_src/l3_routing_nhg.p4
#python src/transpiler.py examples/p4_src/l3_routing_ipv6.p4
ERROR_CODE=$?
if [ "$ERROR_CODE" -ne 0 ]; then
    echo Transpiler failed with error code $ERROR_CODE
    exit 1
fi

cd $pwd
# Compile C sources
make clean;${MAKE_CMD} -j4

rm -rf /tmp/odp*
