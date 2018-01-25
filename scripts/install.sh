#!/bin/bash

cd /root/
mkdir tools
mkdir tools/odp
./mac/scripts/install_pkgs.sh
git clone -b v1.16.0.0 https://github.com/Linaro/odp
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
