# set default values for variables if they are not already defined
MAKE_CMD=${MAKE_CMD-make}
#LD_LIBRARY_PATH=/root/odp/lib/.libs:$LD_LIBRARY_PATH

# Compile Controller
cd src/hardware_dep/shared/ctrl_plane
make clean
make mac_controller
ERROR_CODE=$?
if [ "$ERROR_CODE" -ne 0 ]; then
    echo Controller compilation failed with error code $ERROR_CODE
    exit 1
fi
cd -

# Restart mac controller in background
sudo killall mac_controller
./src/hardware_dep/shared/ctrl_plane/mac_controller &

echo "Controller started... "

echo "Creating Datapath Logic from P4 source."
rm -rf build
python src/transpiler.py examples/p4_src/l2_switch_test.p4
ERROR_CODE=$?
if [ "$ERROR_CODE" -ne 0 ]; then
    echo Transpiler failed with error code $ERROR_CODE
    exit 1
fi


# Compile C sources
make clean;${MAKE_CMD} -j16

# Start the switch
#echo "Runing the switch with veth1 and veth2"
#sudo -E LD_LIBRARY_PATH=/root/odp/lib/.libs ./mac_ad -i veth1,veth2

