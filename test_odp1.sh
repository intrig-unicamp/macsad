
# set default values for variables if they are not already defined
#ARCH=${ARCH-odp}
#P4DPDK_OPTS=${P4DPDK_OPTS--c 0x3 -n 2 -- -P -p 0x3 --config "(0,0,0),(1,0,0)"}
#P4DPDK_OPTS=${P4DPDK_OPTS--c 3 -n 2 -- -P -p 0x3 --config "(0,0,0),(1,0,0)"}
MAKE_CMD=${MAKE_CMD-make}

# Compile test controller
cd src/hardware_dep/shared/ctrl_plane
make clean
make mac_controller
cd -

echo "odp ctrl make done"

# Restart test controller in background
sudo killall mac_controller
sudo killall mac_l3_controller
sudo killall mac_l3-full_controller
./src/hardware_dep/shared/ctrl_plane/mac_controller &


echo "odp ctrl start "
# Complete rebuild starting...
#rm -rf build

#python src/compiler.py examples/p4_src/l2_switch_test.p4
# Compile C sources
#${MAKE_CMD} -j16

# Should the program produce any output, it will be put here
mkdir -p build/output

# Start the program
#echo "run build/example_odp1"
#sudo ./build/example_odp1 ${P4DPDK_OPTS}

