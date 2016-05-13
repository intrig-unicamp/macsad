#!/bin/bash

vethCount=4                                                                      
if [ $# -eq 1 ]; then                                                            
   vethCount=$1                                                                  
fi                                                                               
                                                                                 
let "vethpairs=$vethCount/2"                                                     
for (( i = 0 ; i < ${vethpairs} ; i++ )); do                                     
  intf0="veth$(($i*2))"                                                          
  intf1="veth$(($i*2+1))"                                                        
  sysctl net.ipv6.conf.$intf0.disable_ipv6=1
  sysctl net.ipv6.conf.$intf1.disable_ipv6=1
done

                                                                                 
