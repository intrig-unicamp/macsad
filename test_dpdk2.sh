
# set default values for variables if they are not already defined
ARCH=${ARCH-dpdk}
P4DPDK_OPTS=${P4DPDK_OPTS--c 0x3 -n 3 -- -p 0x3 --config \"\"}
MAKE_CMD=${MAKE_CMD-make}

# Compile test controller
cd src/hardware_dep/dpdk/ctrl_plane
make test_controller
cd -

# Restart test controller in background
if pgrep "test_controller" > /dev/null 2>&1
then
    killall "test_controller"
fi
./src/hardware_dep/dpdk/ctrl_plane/test_controller &


# Complete rebuild starting...
rm -rf build

python src/compiler.py examples/p4_src/test.p4

# Compile C sources
${MAKE_CMD} -j16

# Should the program produce any output, it will be put here
mkdir -p build/output

# Start the program
sudo ./build/example_dpdk1 ${P4DPDK_OPTS}
