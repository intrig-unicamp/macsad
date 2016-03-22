#!/bin/bash

#
# Removes all reserved hugepages.
# From $DPDK/setup/tools.sh
#
clear_huge_pages()
{
	echo > .echo_tmp
	for d in /sys/devices/system/node/node? ; do
		echo "echo 0 > $d/hugepages/hugepages-2048kB/nr_hugepages" >> .echo_tmp
	done
	sudo sh .echo_tmp
	rm -f .echo_tmp
}


if [ -z "RTE_SDK" ]; then echo "\$RTE_SDK has to be set"; exit 1; fi
if [ -z "RTE_TARGET" ]; then echo "\$RTE_TARGET has to be set"; exit 1; fi


# Saving MAC address information to temp files in user home
if [ -e /sys/class/net/eth1 ]; then
	cat /sys/class/net/eth1/address > ~/P4_DPDK_eth1_address.txt
fi

if [ -e /sys/class/net/eth2 ]; then
	cat /sys/class/net/eth2/address > ~/P4_DPDK_eth2_address.txt
fi

# Loading the kernel module
sudo modprobe uio
sudo insmod $RTE_SDK/$RTE_TARGET/kmod/igb_uio.ko

case "$P4DPDK_OPTS" in
	*--no-huge* )
		echo No hugepages are needed
		;;
	* )
		# Removing previous hugepages, if any
		clear_huge_pages

		cat /proc/meminfo | grep -e "Huge[Pp]age"

		echo -en "Setting hugepages... "
		echo 1024 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
		sudo mkdir -p /mnt/huge
		sudo mount -t hugetlbfs nodev /mnt/huge

		cat /proc/meminfo | grep -e "Huge[Pp]age"
		;;
esac

# Binding Linux host network interfaces to the DPDK driver
sudo $RTE_SDK/tools/dpdk_nic_bind.py --bind=igb_uio eth1
sudo $RTE_SDK/tools/dpdk_nic_bind.py --bind=igb_uio eth2
# Binding Windows host network interfaces to the DPDK driver
sudo $RTE_SDK/tools/dpdk_nic_bind.py --bind=igb_uio 0000:00:08.0
sudo $RTE_SDK/tools/dpdk_nic_bind.py --bind=igb_uio 0000:00:09.0
