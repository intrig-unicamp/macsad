#!/bin/bash

if [ -z "$RTE_SDK" ]; then echo "\$RTE_SDK has to be set"; exit 1; fi
if [ -z "$RTE_TARGET" ]; then echo "\$RTE_TARGET has to be set"; exit 1; fi

# Run the program; put it into the background, and store its process id
sudo $RTE_SDK/examples/l2fwd/$RTE_TARGET/l2fwd -c 0x3 -n 2 -- -p 0x3 &
bg_pid=$!

# Make output directory for Wireshark
mkdir -p output

# Start wireshark, it will capture expected outputs for a second
# wireshark -a duration:1 -i ....... -w output

# Reconfigure data to our own MAC address
MAC1=`cat ~/P4_DPDK_eth1_address.txt`
tcprewrite --enet-dmac=$MAC1 --infile=data/http.cap --outfile=output/http_modified.cap


# Wait to make sure the bridge is up, then send some data to the bridge
sleep 3
sudo tcpreplay -i eth0 output/http_modified.cap

# Wait half a second more, then stop the program
sleep 0.5
sudo killall l2fwd

# Print an empty line to leave the user with a proper terminal
sleep 0.1
echo
