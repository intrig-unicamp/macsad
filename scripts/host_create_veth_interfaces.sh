#!/bin/bash

nodeCount=2

sudo echo 1 > /proc/sys/net/ipv4/ip_forward
sudo iptables -t nat -A POSTROUTING -j MASQUERADE

for (( i = 1 ; i <= ${nodeCount} ; i++ )); do
  sudo ip link add dev veth${i}.0 type veth peer name veth${i}.1
  sudo ip link set dev veth${i}.0 up
done

for (( i = 1 ; i <= ${nodeCount} ; i++ )); do
  sudo ip addr add 10.0.0.${i}/24 dev veth${i}.1
  sudo ip link set dev veth${i}.1 up
done

