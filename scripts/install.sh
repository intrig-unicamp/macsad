#!/bin/bash

cd /root/
mkdir tools
mkdir tools/odp
./mac/install_pkgs.sh
wget https://github.com/Linaro/odp/archive/v1.16.0.0.tar.gz
tar xzvf v1.16.0.0.tar.gz
mv odp-1.16.0.0/ odp/
cd odp
./bootstrap
./configure --disable-abi-compat --prefix=/root/tools/odp
make
make install
ln -s /root/odp/helper /root/tools/odp
export ODP_SDK=/root/tools/odp
cd ../mac/
git submodule update --init --recursive
cd p4-hlir
python setup.py install
cd ..
python src/transpiler.py examples/p4_src/l2_fwd.p4
export LD_LIBRARY_PATH=$ODK_SDK:$LD_LIBRARY_PATH
make
sudo sysctl -w vm.nr_hugepages=512
./scripts/veth_create.sh
