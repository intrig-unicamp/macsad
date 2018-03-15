#!/bin/bash

vethCount=2                                                                      
if [ $# -eq 1 ]; then                                                            
   vethCount=$1                                                                  
fi                                                                               
echo "$vethCount veths will be deleted." 

for (( i = 0 ; i < ${vethCount} ; i++ )); do
    intf="veth$(($i*2))"                                                       
    if ip link show $intf &> /dev/null; then          
        ip link delete $intf type veth
#		echo "dele success $intf"		
    fi
done                                                                             
echo "$vethCount veths are successfully deleted." 
#while [ $idx -lt $vethCount ]
#do 
#done                                                                             
