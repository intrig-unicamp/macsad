#!/bin/bash

#Install packages necessary for MACSAD and its submodules
sudo apt-get install -y automake
sudo apt-get install -y autoconf
sudo apt-get install -y libtool
sudo apt-get install -y git

# Libraries for OpenSSL for ODP
sudo apt-get install -y libssl-dev

#For p4-hlir
sudo apt-get install -y python-yaml
sudo apt-get install -y graphviz
sudo apt-get install -y python-pip

#For dpdk
sudo apt-get install -y libpcap-dev

#Various other developement tools
apt install -y gdb exuberant-ctags ethtool vim cscope

#Misc tools
#"killall command not found" 
#sudo apt-get install -y psmisc

#"sudo command not found"
#sudo apt install -y sudo 

#scapy
#git clone https://github.com/p4lang/scapy-vxlan.git
#python setup.py install
