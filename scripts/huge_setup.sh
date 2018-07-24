# Copyright 2018 INTRIG/FEEC/UNICAMP (University of Campinas), Brazil
#
#Licensed under the Apache License, Version 2.0 (the "License");
#you may not use this file except in compliance with the License.
#You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
#Unless required by applicable law or agreed to in writing, software
#distributed under the License is distributed on an "AS IS" BASIS,
#WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#See the License for the specific language governing permissions and
#limitations under the License.

#!/bin/bash

# Setting the hugepage with 2MB page size and user provided page numbers
# across all the numa nodes.

DEF_HUGEPAGE_NOS=512

# Create directory to mount hugepages
mkdir -p /mnt/huge
umount -a -t hugetlbfs
umount /mnt/huge
echo "Mounting Hugetlbfs for hugepages"
(mount | grep hugetlbfs) > /dev/null || mount -t hugetlbfs nodev /mnt/huge

# Read hugepage size (number of hugepages) from arguments
HUGEPAGE_NOS=${1:-$DEF_HUGEPAGE_NOS}
echo "Will set ${HUGEPAGE_NOS} hugepages"

# Set the hugepages for all numa nodes
for i in {0..7}
do
        if [[ -e "/sys/devices/system/node/node$i" ]]
        then
                echo $HUGEPAGE_NOS > /sys/devices/system/node/node$i/hugepages/hugepages-2048kB/nr_hugepages
        fi
done
echo "Hugepage setup done"

# Show the hugepage details
echo "cat /proc/meminfo |grep uge"
cat /proc/meminfo |grep uge

#echo "rep -R "" /sys/kernel/mm/hugepages/ /proc/sys/vm/*huge*"
#grep -R "" /sys/kernel/mm/hugepages/ /proc/sys/vm/*huge*

# To release the hugepage memory used by applications
#rm -rf /dev/hugepages/*
