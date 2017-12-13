#!/bin/bash

sudo apt-get install -y automake autoconf libtool libcunit1-dev sudo

#Install packages necessary for MACSAD and its submodules
sudo apt-get install -y make automake autoconf libtool git

# Libraries for OpenSSL for ODP
sudo apt-get install -y libssl-dev

#For p4-hlir
sudo apt-get install -y python-yaml graphviz python-pip

#For dpdk
sudo apt-get install -y libpcap-dev

#Various other developement tools
apt install -y gdb exuberant-ctags ethtool vim cscope

#Misc tools
#"killall command not found" 
#sudo apt-get install -y psmisc

#Install scapy:
#sudo apt install -y scapy

#Scapy implementation for VxLAN and GRE usage:
#git clone https://github.com/p4lang/scapy-vxlan.git
#cd scapy-vxlan; python setup.py install
