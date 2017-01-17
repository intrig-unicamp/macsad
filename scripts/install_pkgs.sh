#!/bin/bash

#Install packages necessary for MACSAD and its submodules
sudo apt-get install -y automake
sudo apt-get install -y autoconf
sudo apt-get install -y libtool
sudo apt-get install -y git

# Libraries for OpenSSL for ODP
#sudo apt-get install -y libssl-dev

#For p4-hlir
sudo apt-get install -y python-yaml
suod apt-get install -y graphviz
sudo apt-get install -y python-pip
