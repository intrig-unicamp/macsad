#!/bin/bash

#Uncomment the following line and install sudo, if you do not already have it
#apt install -y sudo

#Install packages necessary for MACSAD and its submodules
sudo apt-get install -y make automake autoconf libtool git libcunit1-dev

#Libraries for OpenSSL for ODP
sudo apt-get install -y libssl-dev

#For p4-hlir
sudo apt-get install -y python-yaml graphviz python-pip

#For dpdk
sudo apt-get install -y libpcap-dev

#Various other developement tools
apt install -y gdb exuberant-ctags ethtool vim cscope

#Misc tools
#In case an error of "killall command not found",
#run the following command:
#sudo apt-get install -y psmisc

#Install scapy:
#sudo apt install -y scapy

#Scapy implementation for VxLAN and GRE usage:
#git clone https://github.com/p4lang/scapy-vxlan.git
#cd scapy-vxlan; python setup.py install
