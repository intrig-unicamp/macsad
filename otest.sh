
# set default values for variables if they are not already defined
ARCH=${ARCH-dpdk}
#P4DPDK_OPTS=${P4DPDK_OPTS--c 0x3 -n 2 -- -P -p 0x3 --config "(0,0,0),(1,0,0)"}
P4DPDK_OPTS=${P4DPDK_OPTS--c 3 -n 2 -- -P -p 0x3 --config "(0,0,0),(1,0,0)"}
MAKE_CMD=${MAKE_CMD-make}

# Compile test controller
:'
cd src/hardware_dep/shared/ctrl_plane
make clean
make dpdk_controller
cd -

echo "dpdk ctrl make done"

# Restart test controller in background
if pgrep "dpdk_controller" > /dev/null 2>&1
then
    killall "dpdk_controller"
fi
./src/hardware_dep/shared/ctrl_plane/dpdk_controller &

'
echo "dpdk ctrl start "
# Complete rebuild starting...
rm -rf build

python src/compiler.py examples/p4_src/l2_switch_test.p4
# Compile C sources
${MAKE_CMD} -j16

# Should the program produce any output, it will be put here
mkdir -p build/output

# Start the program
echo "run build/example_dpdk1"
#sudo ./build/example_dpdk1 ${P4DPDK_OPTS}
