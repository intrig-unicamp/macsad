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

action _nop() {
}

table smac {
    reads {
        ethernet.srcAddr : exact;
    }
    actions {_nop;}
    size : 512;
}

control ingress {
    apply(smac);
}

control egress {
}
