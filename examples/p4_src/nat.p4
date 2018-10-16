/* Copyright 2018 INTRIG/FEEC/UNICAMP (University of Campinas), Brazi      */
/*                                                                         */
/*Licensed under the Apache License, Version 2.0 (the "License");          */
/*you may not use this file except in compliance with the License.         */
/*You may obtain a copy of the License at                                  */
/*                                                                         */
/*    http://www.apache.org/licenses/LICENSE-2.0                           */
/*                                                                         */
/*Unless required by applicable law or agreed to in writing, software      */
/*distributed under the License is distributed on an "AS IS" BASIS,        */
/*WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/*See the License for the specific language governing permissions and      */
/*limitations under the License.                                           */

/***********************  C O N S T A N T S  *****************************/
#define ETHERTYPE_ARP  0x0806
#define ETHERTYPE_IPV4 0x0800
#define MAC_LEARN_RECEIVER 1024
#define IP_PROT_TCP 0x06
#define IP_PROT_UDP 0x11

header_type ethernet_t {
    fields {
        dstAddr : 48;
        srcAddr : 48;
        etherType : 16;
    }
}

header_type ipv4_t {
    fields {
        version : 4;
        ihl : 4;
        diffserv : 8;
        totalLen : 16;
        identification : 16;
        flags : 3;
        fragOffset : 13;
        ttl : 8;
        protocol : 8;
        hdrChecksum : 16;
        srcAddr : 32;
        dstAddr: 32;
    }
}

header_type tcp_t {
    fields {
        srcPort : 16;
        dstPort : 16;
        seqNo : 32;
        ackNo : 32;
        dataOffset : 4;
        res : 4;
        flags : 8;
        window : 16;
        checksum : 16;
        urgentPtr : 16;
    }
}

header ethernet_t ethernet;
header ipv4_t ipv4;
header tcp_t tcp;

/***********************  M E T A D A T A  *******************************/
header_type routing_metadata_t {
    fields {
        is_int_if : 8;
    }
}
metadata routing_metadata_t routing_metadata;

/***********************  P A R S E R  ***********************************/

parser start {
    return parse_ethernet;
}

parser parse_ethernet {
    extract(ethernet);
    return select(latest.etherType) {
        ETHERTYPE_IPV4 : parse_ipv4;
        default: ingress;
    }
}

parser parse_ipv4 {
    extract(ipv4);
    return select(ipv4.protocol) {
        IP_PROT_TCP : parse_tcp;
        default: ingress;
    }
}

parser parse_tcp {
    extract(tcp);
    return ingress;
}

field_list mac_learn_digest {
    ethernet.srcAddr;
    standard_metadata.ingress_port;
}

field_list natTcp_learn_digest {
    ipv4.srcAddr;
    tcp.srcPort;
}

/**************  I N G R E S S   P R O C E S S I N G   ******************/

    /***************************** Drop  **************************/
    action _drop() {
        drop();
    }
    action _nop() {
    }
    /***************************** set IF info and others  **************************/
    action set_if_info(is_int) {
        modify_field(routing_metadata.is_int_if, is_int);
    }

    table if_info {
        reads {
            standard_metadata.ingress_port : exact;
        }
        actions {set_if_info; _drop;}
        size : 512;
    }
    
    /***************************** process mac learn  *****************************/
   
    action mac_learn() {
        generate_digest(MAC_LEARN_RECEIVER, mac_learn_digest);
    }

    table smac {
        reads {
            ethernet.srcAddr : exact;
        }
        actions {mac_learn; _nop;}
        size : 512;
    }
    
    /***************************** Nat control *****************************************/

    action natTcp_learn() {
        generate_digest(MAC_LEARN_RECEIVER, natTcp_learn_digest);
    }

    action nat_hit_int_to_ext(srcAddr) {
        modify_field(ipv4.srcAddr, srcAddr);
    }

    table nat_up {
        reads {
            ipv4.srcAddr: lpm;
        }
        actions {
            nat_hit_int_to_ext;
            natTcp_learn;
        }
        size: 512;
    }

    action nat_hit_ext_to_int(dstAddr) {
        modify_field(ipv4.dstAddr, dstAddr);
    }

    table nat_dw {
        reads {
            tcp.dstPort : exact;
        }
        actions {
            nat_hit_ext_to_int;
            _drop;
        }
        size: 512;
    }

    /************** forwarding ipv4 ******************/        
    
    action set_nhop(port, dstAddr) {
        modify_field(standard_metadata.egress_port, port);
        modify_field(ethernet.dstAddr, dstAddr);
    }

    table ipv4_lpm {
        reads {
            ipv4.dstAddr : lpm;
        }
        actions {
            set_nhop;
            _drop;
        }
        size: 512;
    }

    action rewrite_src_mac(srcAddr) {
        modify_field(ethernet.srcAddr, srcAddr);
    }

    table sendout {
        reads {
            standard_metadata.egress_port: exact;
        }
        actions {
            rewrite_src_mac;
            _drop;
        }
        size: 512;
    }

    /************** APPLY ******************/

control ingress {
    apply(if_info);
    apply(smac);
    if (routing_metadata.is_int_if == 1) {
        apply(nat_up);
    } else {
        apply(nat_dw);
    }
    apply(ipv4_lpm);
    apply(sendout);
}

control egress {
}
