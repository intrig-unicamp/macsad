#!/bin/bash

if [ -z "$RTE_SDK" ]; then echo "\$RTE_SDK has to be set"; exit 1; fi
if [ -z "$RTE_TARGET" ]; then echo "\$RTE_TARGET has to be set"; exit 1; fi


CURR_DIR=`pwd`

# Installing necessary OS packages
sudo apt-get install linux-headers-`uname -r` g++-multilib libvirt-dev python-setuptools clang-format-3.6 libpcap-dev pandoc tcpreplay wireshark git

# Building DPDK
cd $RTE_SDK
sudo make config -j16 T=$RTE_TARGET
sudo make -j16 T=$RTE_TARGET
sudo make install -j16 T=$RTE_TARGET



cd $CURR_DIR
