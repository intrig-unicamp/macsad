#define MAC_LEARN 1024

header_type ethernet_t {
    fields {
        dstAddr : 48;
        srcAddr : 48;
        etherType : 16;
    }
}

header ethernet_t ethernet;

parser start {
    return parse_ethernet;
}

parser parse_ethernet {
    extract(ethernet);
    return ingress;
}

action _drop() {
    drop();
}

action _nop() {
}


field_list mac_learn_digest {
    ethernet.srcAddr;
    standard_metadata.ingress_port;
}

action mac_learn() {
    generate_digest(MAC_LEARN, mac_learn_digest);
}

table smac {
    reads {
        ethernet.srcAddr : exact;
    }
    actions {mac_learn; _nop;}
    size : 512;
}

action forward(port) {
    modify_field(standard_metadata.egress_port, port);
}

action bcast() {
    modify_field(standard_metadata.egress_port, 100);

}

table dmac {
    reads {
        ethernet.dstAddr : exact;
    }
    actions {forward; bcast;}
    size : 512;
}

control ingress {
    apply(smac);
    apply(dmac);
}

control egress {
}
