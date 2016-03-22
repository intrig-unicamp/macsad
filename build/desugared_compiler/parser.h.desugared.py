import inspect
from header_info import *

def enum(es):
    return "enum %s {\n  %s\n};\n\n" % (inspect.stack()[1][3], ",\n  ".join(es))
def harray(t, es):
    es2 = [ a + " /* " + b + " */" for (a,b) in zip(es, header_instance_ids(hlir))]
    return "static const %s %s[HEADER_INSTANCE_COUNT] = {\n  %s\n};\n\n" % (t, inspect.stack()[1][3], ",\n  ".join(es2))
def farray(t, es):
    es2 = [ a + " /* " + b + " */" for (a,b) in zip(es, field_instance_ids(hlir))]
    return "static const %s %s[FIELD_INSTANCE_COUNT] = {\n  %s\n};\n\n"  % (t, inspect.stack()[1][3], ",\n  ".join(es2))

#==============================================================================

def header_instance_e():
    return enum(header_instance_ids(hlir))

def field_instance_e():
    return enum(field_instance_ids(hlir))

def header_instance_byte_width():
    ws = [str(sum([f[1] for f in hi.header_type.layout.items()])/8) for hn,hi in hlir.p4_header_instances.items()]
    return harray("int", ws)

def header_instance_is_metadata():
    ms = ["1" if hi.metadata else "0" for hn,hi in hlir.p4_header_instances.items()]
    return harray("int", ms)

def field_instance_bit_width():
    ws = [str(f[1]) for hn,hi in hlir.p4_header_instances.items() for f in hi.header_type.layout.items()]
    return farray("int", ws)

def field_instance_bit_offset():
    os = [str(item%8) for sublist in [field_offsets(hi.header_type) for hn,hi in hlir.p4_header_instances.items()] for item in sublist]
    return farray("int", os)

def field_instance_byte_offset_hdr():
    os = [str(item//8) for sublist in [field_offsets(hi.header_type) for hn,hi in hlir.p4_header_instances.items()] for item in sublist]
    return farray("int", os)

def field_instance_mask():
    ws = [f[1] for hn,hi in hlir.p4_header_instances.items() for f in hi.header_type.layout.items()]
    os = [item%8 for sublist in [field_offsets(hi.header_type) for hn,hi in hlir.p4_header_instances.items()] for item in sublist]
    ms = [field_mask(a, b) for (a,b) in zip(os, ws)]
    return farray("uint32_t", ms)

def field_instance_header():
    names = [hn for hn,hi in hlir.p4_header_instances.items() for fn,fw in hi.header_type.layout.items()]
    ids = map(hdr_prefix, names)
    return farray("header_instance_t", ids)

hc = len(header_instance_ids(hlir))
fc = len(field_instance_ids(hlir))

generated_code += " #ifndef __HEADER_INFO_H__// sugar@55\n"
generated_code += " #define __HEADER_INFO_H__// sugar@56\n"
generated_code += " \n"
generated_code += " #define HEADER_INSTANCE_COUNT " + str(hc) + "// sugar@58\n"
generated_code += " #define FIELD_INSTANCE_COUNT " + str(fc) + "// sugar@59\n"
generated_code += " \n"
generated_code += " " + str(header_instance_e() ) + "// sugar@61\n"
generated_code += " " + str(field_instance_e() ) + "// sugar@62\n"
generated_code += " typedef enum header_instance_e header_instance_t;// sugar@63\n"
generated_code += " typedef enum field_instance_e field_instance_t;// sugar@64\n"
generated_code += " " + str(header_instance_byte_width() ) + "// sugar@65\n"
generated_code += " " + str(header_instance_is_metadata() ) + "// sugar@66\n"
generated_code += " " + str(field_instance_bit_width() ) + "// sugar@67\n"
generated_code += " " + str(field_instance_bit_offset() ) + "// sugar@68\n"
generated_code += " " + str(field_instance_byte_offset_hdr() ) + "// sugar@69\n"
generated_code += " " + str(field_instance_mask() ) + "// sugar@70\n"
generated_code += " " + str(field_instance_header() ) + "// sugar@71\n"
generated_code += " \n"
generated_code += " #endif // __HEADER_INFO_H__// sugar@73\n"