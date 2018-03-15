
from hlir16.utils_hlir16 import *


#[ #ifndef __HEADER_INFO_H__
#[ #define __HEADER_INFO_H__

#[ #include <byteswap.h>

#[ // TODO add documentation
#[ #define MODIFIED 1


#{ typedef struct parsed_fields_s {

for hdr in hlir16.header_instances:
    for fld in hdr.type.type_ref.fields:
        if not fld.preparsed and fld.type.size <= 32:
            #[ uint32_t field_instance_${hdr.name}_${fld.name};
            #[ uint8_t attr_field_instance_${hdr.name}_${fld.name};
#} } parsed_fields_t;


#[ // Header stack infos
#[ // ------------------


# TODO make proper header stacks (types grouped by instances? order?)

#[ #define HEADER_STACK_COUNT ${len(hlir16.header_instances)}


#{ enum header_stack_e {
for hi in hlir16.header_instances:
    #[ header_stack_${hi.name},
#[ header_stack_, // dummy to prevent warning for empty stack
#} };



#[ // Header instance infos
#[ // ---------------------

#[ #define HEADER_INSTANCE_COUNT ${len(hlir16.header_instances)}


#[ enum header_instance_e {
for hdr in hlir16.header_instances:
    #[   header_instance_${hdr.name},
#[ };


#[ static const int header_instance_byte_width[HEADER_INSTANCE_COUNT] = {
for hdr in hlir16.header_instances:
    #[   ${hdr.type.type_ref.byte_width}, // header_instance_${hdr.name}, ${hdr.type.type_ref.bit_width} bits, ${hdr.type.type_ref.bit_width/8.0} bytes
#[ };


#[ static const int header_instance_is_metadata[HEADER_INSTANCE_COUNT] = {
for hdr in hlir16.header_instances:
    #[   ${1 if hdr.type.type_ref.is_metadata else 0}, // header_instance_${hdr.name}
#[ };



# TODO move to hlir16.py/set_additional_attrs?
def all_field_instances():
    return [fld for hdr in hlir16.header_instances for fld in hdr.type.type_ref.fields]


#[ // Field instance infos
#[ // --------------------


#[ #define FIELD_INSTANCE_COUNT ${len(all_field_instances())}


#[ typedef enum header_instance_e header_instance_t;
#[ typedef enum field_instance_e field_instance_t;


#[ enum field_instance_e {
for hdr in hlir16.header_instances:
    for fld in hdr.type.type_ref.fields:
        #[   field_instance_${hdr.name}_${fld.name},
#[ };

#[ // Note: -1: is fixed-width field
#[ static const int header_instance_var_width_field[HEADER_INSTANCE_COUNT] = {
for hdr in hlir16.header_instances:
    field_id_pattern = 'field_instance_{}_{}'
    #[   ${reduce((lambda x, f: field_id_pattern.format(hdr.name, f.name) if f.is_vw else x), hdr.type.type_ref.fields, -1)}, // header_instance_${hdr.name}
#[ };


# TODO this should be available as a field
def get_real_type(typenode):
    return typenode.type_ref if hasattr(typenode, 'type_ref') else typenode

#[ static const int field_instance_bit_width[FIELD_INSTANCE_COUNT] = {
for hdr in hlir16.header_instances:
    for fld in hdr.type.type_ref.fields:
        fldtype = get_real_type(fld.type)
        #[   ${fldtype.size}, // field_instance_${hdr.name}_${fld.name}
#[ };


#[ static const int field_instance_bit_offset[FIELD_INSTANCE_COUNT] = {
for hdr in hlir16.header_instances:
    for fld in hdr.type.type_ref.fields:
        #[   (${fld.offset} % 8), // field_instance_${hdr.name}_${fld.name}
#[ };


# TODO why does this name have "_hdr" at the end, but field_instance_bit_offset doesn't?

#[ static const int field_instance_byte_offset_hdr[FIELD_INSTANCE_COUNT] = {
for hdr in hlir16.header_instances:
    for fld in hdr.type.type_ref.fields:
        #[   (${fld.offset} / 8), // field_instance_${hdr.name}_${fld.name}
#[ };



#[ // TODO documentation
#[ #define mod_top(n, bits) (((bits-(n%bits)) % bits))

#[ // Produces a 32 bit int that has n bits on at the top end.
#[ #define uint32_top_bits(n) (0xffffffff << mod_top(n, 32))

#[ static const int field_instance_mask[FIELD_INSTANCE_COUNT] = {
for hdr in hlir16.header_instances:
    for fld in hdr.type.type_ref.fields:
        fldtype = get_real_type(fld.type)
        #[  __bswap_constant_32(uint32_top_bits(${fldtype.size}) >> (${fld.offset}%8)), // field_instance_${hdr.name}_${fld.name},
#[ };


#[ static const header_instance_t field_instance_header[FIELD_INSTANCE_COUNT] = {
for hdr in hlir16.header_instances:
    for fld in hdr.type.type_ref.fields:
        #[   header_instance_${hdr.name}, // field_instance_${hdr.name}_${fld.name}
#[ };


#[ // TODO current stacks are exactly 1 element deep 
#{ static const header_instance_t header_stack_elements[HEADER_STACK_COUNT][10] = {
for hi in hlir16.header_instances:
    #[ // header_instance_${hi.name}
    #{ {
    for stack_elem in [hi.name]:
        #[ header_instance_${stack_elem},
    #} },
#} };

#{ static const unsigned header_stack_size[HEADER_STACK_COUNT] = {
for hi in hlir16.header_instances:
    #[ 1, // ${hi.name}
#} };

#[ typedef enum header_stack_e header_stack_t;

#[ /////////////////////////////////////////////////////////////////////////////
#[ // HEADER TYPE AND FIELD TYPE INFORMATION
#[ // TODO remove instance info when no code refers to it

#[ #define HEADER_COUNT ${len(hlir16.header_types)}

#[ enum header_e {
for hdr in hlir16.header_types:
    #[   header_${hdr.name},
#[ };

#[ static const int header_byte_width[HEADER_COUNT] = {
for hdr in hlir16.header_types:
    #[   ${hdr.byte_width}, // ${hdr.name}
#[ };

#[ static const int header_is_metadata[HEADER_COUNT] = {
for hdr in hlir16.header_types:
    #[   ${1 if hdr.is_metadata else 0}, // ${hdr.name}
#[ };


def all_fields():
    return [fld for hdr in hlir16.header_types for fld in hdr.fields]

#[ #define FIELD_COUNT ${len(all_fields())}

#[ typedef enum header_e header_t;
#[ typedef enum field_e field_t;

#[ enum field_e {
for hdr in hlir16.header_types:
    for fld in hdr.fields:
        #[   field_${hdr.name}_${fld.name},
#[ };

#[ static const int field_bit_width[FIELD_COUNT] = {
for hdr in hlir16.header_types:
    for fld in hdr.fields:
        fldtype = get_real_type(fld.type)
        #[ ${fldtype.size}, // field_${hdr.name}_${fld.name}
#[ };

#[ static const int field_bit_offset[FIELD_COUNT] = {
for hdr in hlir16.header_types:
    for fld in hdr.fields:
        #[   (${fld.offset} % 8), // field_${hdr.name}_${fld.name}
#[ };

#[ static const int field_byte_offset_hdr[FIELD_COUNT] = {
for hdr in hlir16.header_types:
    for fld in hdr.fields:
        #[   (${fld.offset} / 8), // field_${hdr.name}_${fld.name}
#[ };

#[ #define mod_top(n, bits) (((bits-(n%bits)) % bits))
#[ // Produces a 32 bit int that has n bits on at the top end.
#[ #define uint32_top_bits(n) (0xffffffff << mod_top(n, 32))

#[ static const int field_mask[FIELD_COUNT] = {
for hdr in hlir16.header_types:
    for fld in hdr.fields:
        fldtype = get_real_type(fld.type)
        #[  __bswap_constant_32(uint32_top_bits(${fldtype.size}) >> (${fld.offset}%8)), // field_${hdr.name}_${fld.name},
#[ };

#[ static const header_t field_header[FIELD_COUNT] = {
for hdr in hlir16.header_types:
    for fld in hdr.fields:
        #[   header_${hdr.name}, // field_${hdr.name}_${fld.name}
#[ };

#[ static const int header_var_width_field[HEADER_COUNT] = {
for hdr in hlir16.header_types:
    #[   ${reduce((lambda x, f: f.id if f.is_vw else x), hdr.fields, -1)}, // ${hdr.name}
#[ };


for enum in hlir16.declarations['Type_Enum']:
    #[ enum ${enum.c_name} { ${', '.join([m.c_name for m in enum.members])} };

for error in hlir16.declarations['Type_Error']:
    #[ enum ${error.c_name} { ${', '.join([m.c_name for m in error.members])} };

#[
#[ // HW optimization related infos
#[ // --------------------
#[ #define OFFLOAD_CHECKSUM ${1 if []!=[x for x in hlir16.sc_annotations if x.name=='offload'] else 0}


#[ #endif // __HEADER_INFO_H__
