
# set default values for variables if they are not already defined
MAKE_CMD=${MAKE_CMD-make}

# Compile test controller
cd src/hardware_dep/shared/ctrl_plane
make clean
make mac_controller
cd -

echo "Controller compilation complete."

# Restart test controller in background
sudo killall mac_controller
./src/hardware_dep/shared/ctrl_plane/mac_controller &

echo "Controller started... "

echo "Creating auto-generted files from p4 source...
rm -rf build
python src/compiler.py examples/p4_src/l2_switch_test.p4

# Compile C sources
#${MAKE_CMD} -j16

# Start the switch
echo "Run mac_ad with veth1.0 and veth2.1"
sudo ./mac_ad -i veth1.0,veth2.1

