#!/bin/bash

if [ -z "RTE_SDK" ]; then echo "\$RTE_SDK has to be set"; exit 1; fi
if [ -z "RTE_TARGET" ]; then echo "\$RTE_TARGET has to be set"; exit 1; fi


# Unbinding Linux host network interfaces to the DPDK driver
sudo $RTE_SDK/tools/dpdk_nic_bind.py -u eth1
sudo $RTE_SDK/tools/dpdk_nic_bind.py -u eth2
# Unbinding Windows host network interfaces to the DPDK driver
sudo $RTE_SDK/tools/dpdk_nic_bind.py -u 0000:00:08.0
sudo $RTE_SDK/tools/dpdk_nic_bind.py -u 0000:00:09.0

# Unsetting hugepages
echo 0 | sudo tee /sys/kernel/mm/hugepages/hugepages-2048kB/nr_hugepages
sudo umount /mnt/huge


# Unloading the kernel module
sudo modprobe -r uio
sudo rmmod $RTE_SDK/$RTE_TARGET/kmod/igb_uio.ko
