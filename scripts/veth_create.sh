#!/bin/bash

vethCount=4
if [ $# -eq 1 ]; then 
   vethCount=$1
fi
echo "$vethCount veths will be created."

let "vethpairs=$vethCount/2"
for (( i = 0 ; i < ${vethpairs} ; i++ )); do
  intf0="veth$(($i*2))"
  intf1="veth$(($i*2+1))"

  if ! ip link show $intf0 &> /dev/null; then
    ip link add name $intf0 type veth peer name $intf1
    ip link set dev $intf0 up
    ip link set dev $intf1 up

    INTF_OPTIONS="rx tx sg tso ufo gso gro"
#   echo "ethtool options for $intf0 $intf1"
#	echo $INTF_OPTIONS

	for INTF_OPTION in $INTF_OPTIONS; do
#	  echo "ethtool option to unset $INTF_OPTION"
#	  echo "/sbin/ethtool --offload $intf0 $INTF_OPTION off"
	  /sbin/ethtool --offload $intf0 $INTF_OPTION off
#	  echo "/sbin/ethtool --offload $intf1 $INTF_OPTION off"
	  /sbin/ethtool --offload $intf1 $INTF_OPTION off
	done

	ip addr add 10.0.0.$(($i*2+1))/24 dev $intf1
    ip link set dev $intf1 up 
#   echo "create success for $intf0 $intf1"
  fi
done

#******************************

#rx  - 
#tx  - 
#sg  - scatter gather 
#tso - tcp segmentation offload
#ufo - udp fragmentation offload
#gso - generic segmentation offload
#gro - 
#lro - large recv offload
#rxvlan - 
#txvlan - 
#rxhash - 

#******************************
#while [ $idx -lt $vethCount ]                                                   
#do                                                                              
#done 
#  sudo ip link add dev veth${i}.0 type veth peer name veth${i}.1
#  sudo ip link set dev veth${i}.0 up
#sudo echo 1 > /proc/sys/net/ipv4/ip_forward
#sudo iptables -t nat -A POSTROUTING -j MASQUERADE
#INTF_OPTIONS="rx tx sg tso ufo gso gro lro rxvlan txvlan rxhash"
