import p4_hlir.hlir.p4 as p4
from header_info import hdr_prefix, fld_prefix

def int_to_byte_array(val): # CAUTION: big endian!
    """
    :param val: int
    :rtype:     (int, [int])
    """
    nbytes = 0
    res = []
    while val > 0 or nbytes < 4:
        nbytes += 1
        res.append(int(val % 256))
        val /= 256
    res.reverse()
    return nbytes, res

def fld_id(f):
    """
    :param f: p4_field
    :rtype:   str
    """
    return fld_prefix(f.instance.name + "_" + f.name)

def get_key_byte_width(branch_on):
    """
    :param branch_on: list of union(p4_field, tuple)
    :rtype:           int
    """
    key_width = 0
    for switch_ref in branch_on:
        if type(switch_ref) is p4.p4_field:
            key_width += (switch_ref.width+7)/8
        elif type(switch_ref) is tuple:
            key_width += max(4, (switch_ref[1] + 7) / 8)
    return max(4, key_width)

generated_code += " // TODO: insert the parser_exception calls into right places// sugar@38\n"
pe_dict = { "p4_pe_index_out_of_bounds" : None,
            "p4_pe_out_of_packet" : None,
            "p4_pe_header_too_long" : None,
            "p4_pe_header_too_short" : None,
            "p4_pe_unhandled_select" : None,
            "p4_pe_checksum" : None,
            "p4_pe_default" : None }


pe_default = p4.p4_parser_exception(None, None)
pe_default.name = "p4_pe_default"
pe_default.return_or_drop = p4.P4_PARSER_DROP

hlir_pes = hlir.p4_parser_exceptions
for pe_name, pe in pe_dict.items():
    if hlir_pes.has_key(pe_name):
        pe_dict[pe_name] = hlir_pes[pe_name]
    else:
        pe_dict[pe_name] = pe_default
for pe_name, pe in hlir_pes.items():
    if pe_dict.hask_key(pe_name) == False:
        pe_dict[pe_name] = pe



generated_code += " #include \"odp_lib.h\"// sugar@64\n"
generated_code += " void print_mac(uint8_t* v) { printf(\"%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX\\n\", v[0], v[1], v[2], v[3], v[4], v[5]); }// sugar@65\n"
generated_code += " void print_ip(uint8_t* v) { printf(\"%d.%d.%d.%d\\n\",v[0],v[1],v[2],v[3]); }// sugar@66\n"
generated_code += "\n"


for pe_name, pe in pe_dict.items():
    generated_code += " static inline void " + str(pe_name) + "(packet_descriptor_t *pd) {// sugar@71\n"
    if pe.return_or_drop == p4.P4_PARSER_DROP:
        generated_code += "     drop(pd->packet);// sugar@73\n"

    elif type(pe.return_or_drop) == p4.p4_conditional_node:
        generated_code += "     // TODO: the specification allow only control function// sugar@76\n"
        generated_code += "     // TODO: generate conditions and expressions// sugar@77\n"
    generated_code += " }// sugar@78\n"
    
generated_code += "\n"

for table in hlir.p4_tables.values():
    generated_code += " void apply_table_" + str(table.name) + "(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@83\n"
generated_code += " \n"
generated_code += " static void// sugar@85\n"
generated_code += " extract_header(uint8_t* buf, packet_descriptor_t* pd, header_instance_t h) {// sugar@86\n"
generated_code += "     pd->headers[h] =// sugar@87\n"
generated_code += "       (header_descriptor_t) {// sugar@88\n"
generated_code += "         .type = h,// sugar@89\n"
generated_code += "         .pointer = buf,// sugar@90\n"
generated_code += "         .length = header_instance_byte_width[h]// sugar@91\n"
generated_code += "       };// sugar@92\n"
generated_code += " }// sugar@93\n"
generated_code += " \n"

for state_name, parse_state in hlir.p4_parse_states.items():
    generated_code += " static void parse_state_" + str(state_name) + "(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables);// sugar@97\n"
generated_code += "\n"

for state_name, parse_state in hlir.p4_parse_states.items():
    branch_on = parse_state.branch_on
    if branch_on:
        generated_code += " static inline void build_key_" + str(state_name) + "(packet_descriptor_t *pd, uint8_t *buf, uint8_t *key) {// sugar@103\n"
        for switch_ref in branch_on:
            if type(switch_ref) is p4.p4_field:
                field_instance = switch_ref
                byte_width = (field_instance.width + 7) / 8
                if byte_width <= 4:
                    generated_code += " EXTRACT_INT32(pd, " + str(fld_id(field_instance)) + ", *(uint32_t*)key)// sugar@109\n"
                    generated_code += " key += sizeof(uint32_t);// sugar@110\n"
                else:
                    generated_code += " EXTRACT_BYTEBUF(pd, " + str(fld_id(field_instance)) + ", key)// sugar@112\n"
                    generated_code += " key += " + str(byte_width) + ";// sugar@113\n"
            elif type(switch_ref) is tuple:
                generated_code += "     uint8_t* ptr;// sugar@115\n"
                offset, width = switch_ref
                src_len = (width + 7) / 8
                generated_code += "     ptr = buf + " + str(offset / 8) + ";// sugar@118\n"
                generated_code += "     // TODO implement extract_*_from_stream in the backend// sugar@119\n"
                if width <= 32:
                    generated_code += "     *(uint32_t *) key = extract_intvalue_from_stream(ptr, " + str(offset % 8) + ", " + str(width) + ");// sugar@121\n"
                    generated_code += "     key += sizeof(uint32_t);// sugar@122\n"
                else:
                    generated_code += "     memcpy(key, extract_bytebuf_from_stream(ptr, " + str(offset % 8) + ", " + str(width) + "), " + str(src_len) + ");// sugar@124\n"
                    generated_code += "     key += " + str(src_len) + ";// sugar@125\n"
        generated_code += " }// sugar@126\n"

for state_name, parse_state in hlir.p4_parse_states.items():
    generated_code += " static void parse_state_" + str(state_name) + "(packet_descriptor_t* pd, uint8_t* buf, lookup_table_t** tables)// sugar@129\n"
    generated_code += " {// sugar@130\n"
    
    for call in parse_state.call_sequence:
        if call[0] == p4.parse_call.extract:
            header_instance_name = hdr_prefix(call[1].name)
            generated_code += "     extract_header(buf, pd, " + str(header_instance_name) + ");// sugar@135\n"
            generated_code += "     buf += header_instance_byte_width[" + str(header_instance_name) + "];// sugar@136\n"
        elif call[0] == p4.parse_call.set:
            dest_field, src = call[1], call[2]
            if type(src) is int or type(src) is long:
                generated_code += "     modify_field_to_const32(pd, field_desc(" + str(fld_id(dest_field)) + "), " + str(hex(src)) + ");// sugar@140\n"
            elif type(src) is p4.p4_field:
                generated_code += "     modify_field_to_field(pd, field_desc(" + str(fld_id(dest_field)) + "), field_desc(" + str(fld_id(src)) + "));// sugar@142\n"
            elif type(src) is tuple:
                value, mask = src
                fld = fld_id(dest_field)
                generated_code += "     modify_field_to_field(pd, field_desc(" + str(fld_id(dest_field)) + "), field_desc(" + str(fld_id(src)) + "));// sugar@146\n"
                generated_code += "     modify_field_with_mask(pd, field_desc(" + str(fld) + "), " + str(value) + ", " + str(mask) + ", (field_instance_bit_width[" + str(fld) + "+7])/8)// sugar@147\n"
#                #[     modify_field_to_field(pd, field_desc(${fld_id(dest_field)}), field_desc(${fld_id(src)}));
#                #[     modify_field_from_buffer(pd, field_desc(${fld_id(dest_field)}), ?, mask));
#void modify_field_to_field  (packet_descriptor_t* p, field_reference_t dstf, field_reference_t srcf);
#void modify_field_with_mask (packet* p, field f, void* value, void* mask, int modify_length);
                generated_code += "     // TODO setting field to current// sugar@152\n"
    branch_on = parse_state.branch_on
    if not branch_on:
        branch_case, next_state = parse_state.branch_to.items()[0]
        if isinstance(next_state, p4.p4_parse_state):
            generated_code += "     return parse_state_" + str(next_state.name) + "(pd, buf, tables);// sugar@157\n"
        else:
            generated_code += "     #if debug == 1// sugar@159\n"
            generated_code += "     printf(\"  :::: PACKET PARSED\\n\");// sugar@160\n"
            generated_code += "     printf(\"    :: ethernet srcaddr: \");// sugar@161\n"
            generated_code += "     print_mac(extract_bytebuf(pd, field_desc(field_instance_ethernet_srcAddr)));// sugar@162\n"
            generated_code += "     printf(\"    :: ethernet dstaddr: \");// sugar@163\n"
            generated_code += "     print_mac(extract_bytebuf(pd, field_desc(field_instance_ethernet_dstAddr)));// sugar@164\n"
            generated_code += "     //printf(\"    :: IP src: \"); // sugar@165\n"
            generated_code += "     //print_ip(extract_bytebuf(pd, field_desc(field_instance_ipv4_srcAddr))); // sugar@166\n"
            generated_code += "     //printf(\"    :: IP dst: \"); // sugar@167\n"
            generated_code += "     //print_ip(extract_bytebuf(pd, field_desc(field_instance_ipv4_dstAddr)));// sugar@168\n"
            generated_code += "     for (int i = 0; i < HEADER_INSTANCE_COUNT; ++i) {// sugar@169\n"
            generated_code += "         printf(\"    :: header %d (type=%d, len=%d) = \", i, pd->headers[i].type, pd->headers[i].length);// sugar@170\n"
            generated_code += "         for (int j = 0; j < pd->headers[i].length; ++j) {// sugar@171\n"
            generated_code += "         printf(\"%02x \", ((uint8_t*)(pd->headers[i].pointer))[j]);// sugar@172\n"
            generated_code += "         }// sugar@173\n"
            generated_code += "         printf(\"\\n\");// sugar@174\n"
            generated_code += "     }// sugar@175\n"
            generated_code += "\n"
            generated_code += "     printf(\"  :::: TURNING TO TABLE " + str(next_state.name) + "\\n\");// sugar@177\n"
            generated_code += "     #endif// sugar@178\n"
            generated_code += "     return apply_table_" + str(next_state.name) + "(pd, tables);// sugar@179\n"
    else:
        key_byte_width = get_key_byte_width(branch_on)
        generated_code += "     uint8_t key[" + str(key_byte_width) + "];// sugar@182\n"
        generated_code += "     build_key_" + str(state_name) + "(pd, buf, key);// sugar@183\n"
        has_default_case = False
        for case_num, case in enumerate(parse_state.branch_to.items()):
            branch_case, next_state = case
            mask_name  = "mask_value_%d" % case_num
            value_name  = "case_value_%d" % case_num
            if branch_case == p4.P4_DEFAULT: # TODO why should two if?
                has_default_case = True
                if isinstance(next_state, p4.p4_parse_state):
                    generated_code += "     return parse_state_" + str(next_state.name) + "(pd, buf, tables);// sugar@192\n"
                else:
                    generated_code += "     return apply_table_" + str(next_state.name) + "(pd, tables);// sugar@194\n"
                continue
            elif type(branch_case) is int:
                branch_case # TODO ?
            elif type(branch_case) is tuple:
                value = branch_case[0]
                mask = branch_case[1]
                generated_code += "     // TODO masked values not supported yet// sugar@201\n"
            elif type(branch_case) is p4.p4_parse_value_set:
                value_set = branch_case
                generated_code += "     // TODO value sets not supported yet// sugar@204\n"
                continue

            if type(branch_case) is int or type(branch_case) is tuple:
                value = branch_case
                value_len, l = int_to_byte_array(value)
                generated_code += "     uint8_t " + str(value_name) + "[" + str(value_len) + "] = {// sugar@210\n"
                for c in l:
                    generated_code += "         " + str(c) + ",// sugar@212\n"
                generated_code += "     };// sugar@213\n"
                generated_code += "     if ( memcmp(key, " + str(value_name) + ", " + str(value_len) + ") == 0)// sugar@214\n"
                if isinstance(next_state, p4.p4_parse_state):
                    generated_code += "     return parse_state_" + str(next_state.name) + "(pd, buf, tables);// sugar@216\n"
                else:
                    generated_code += "     return apply_table_" + str(next_state.name) + "(pd, tables);// sugar@218\n"
        if not has_default_case:
            generated_code += "     return NULL;// sugar@220\n"
    generated_code += " }// sugar@221\n"
    generated_code += " \n"



# TODO This initialisation shouldn't be done for each and every packet. Perhaps we need a generated packet initialisation function that is invoked on main initialisation, and the main_loop would only set the ingress_port metadata field for each packet.

generated_code += " void init_metadata_headers(packet_descriptor_t* packet_desc) {// sugar@228\n"
for hn,hi in hlir.p4_header_instances.items():
    if hi.metadata and not hn is 'standard_metadata':
        n = hdr_prefix(hn)
        generated_code += " packet_desc->headers[" + str(n) + "] = (header_descriptor_t) { .type = " + str(n) + ", .length = header_instance_byte_width[" + str(n) + "],// sugar@232\n"
        generated_code += "                               .pointer = calloc(header_instance_byte_width[" + str(n) + "], sizeof(uint8_t)) };// sugar@233\n"
generated_code += " }// sugar@234\n"
generated_code += "\n"

generated_code += " void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables) {// sugar@237\n"
generated_code += "     init_metadata_headers(pd);// sugar@238\n"
generated_code += "     parse_state_start(pd, pd->pointer, tables);// sugar@239\n"
generated_code += " }// sugar@240\n"
