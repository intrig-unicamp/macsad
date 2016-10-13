# set default values for variables if they are not already defined
MAKE_CMD=${MAKE_CMD-make}
#LD_LIBRARY_PATH=/root/odp/lib/.libs:$LD_LIBRARY_PATH

# Compile test controller
cd src/hardware_dep/shared/ctrl_plane
make clean
make mac_controller
cd -

# Restart mac controller in background
sudo killall mac_controller
./src/hardware_dep/shared/ctrl_plane/mac_controller &

echo "Controller started... "

echo "Creating Datapath Logic from P4 source."
rm -rf build
python src/transpiler.py examples/p4_src/l2_switch_test.p4

# Compile C sources
make clean;${MAKE_CMD} -j16

# Start the switch
#echo "Runing the switch with veth1 and veth2"
#sudo -E LD_LIBRARY_PATH=/root/odp/lib/.libs ./mac_ad -i veth1,veth2

