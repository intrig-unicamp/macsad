#!/bin/bash

cd /root/
./mac/scripts/install_pkgs.sh
wget https://github.com/Linaro/odp/archive/v1.16.0.0.tar.gz
tar xzvf v1.16.0.0.tar.gz
mv odp-1.16.0.0/ odp/
cd odp
rm -rf v1.16.0.0.tar.gz
./bootstrap
./configure --disable-abi-compat --prefix=`pwd`/build
make
make install
echo `pwd`/build/lib | sudo tee /etc/ld.so.conf.d/odp.conf
sudo ldconfig
export PKG_CONFIG_PATH=`pwd`/build/lib/pkgconfig
echo "export PKG_CONFIG_PATH=`pwd`/build/lib/pkgconfig" >> ~/.bashrc
cd ..
git clone --recursive https://github.com/intrig-unicamp/mac.git
cd mac
cd p4-hlir
python setup.py install --user
cd ..
python src/transpiler.py examples/p4_src/l2_fwd.p4
./autogen.sh
./configure
make
sudo sysctl -w vm.nr_hugepages=512
sudo ./scripts/veth_create.sh

