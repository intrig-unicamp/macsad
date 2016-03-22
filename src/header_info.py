def hdr_prefix(name): return "header_instance_"+name
def fld_prefix(name): return "field_instance_"+name

def header_instance_ids(hlir):
    return map(hdr_prefix, hlir.p4_header_instances)

def field_instance_ids(hlir):
    names = [hn+"_"+fn for hn,hi in hlir.p4_header_instances.items() for fn,fw in hi.header_type.layout.items()]
    return map(fld_prefix, names)

def field_offsets(header_type):
    offsets = []
    o = 0
    for name,length in header_type.layout.items():
        offsets.append(o)
        o += length
    return offsets

def field_mask(bitoffset, bitwidth):
    if bitoffset+bitwidth > 32: return hex(0) # FIXME do something about this
    pos = bitoffset;
    mask = 0;
    while pos < bitoffset + bitwidth:
        mask |= (1 << pos)
        pos += 1
    return hex(mask)
