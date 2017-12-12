mkdir /mnt/huge
umount -a -t hugetlbfs
umount /mnt/huge
echo "Mounting hugetlbfs"
mount -t hugetlbfs nodev /mnt/huge
sh -c 'echo 5120 > /sys/devices/system/node/node0/hugepages/hugepages-2048kB/nr_hugepages'
cat /proc/meminfo |grep uge
grep -R "" /sys/kernel/mm/hugepages/ /proc/sys/vm/*huge*
