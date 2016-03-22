import p4_hlir.hlir.p4 as p4
from utils import getTypeAndLength
from header_info import fld_prefix

def fld_id(f): return fld_prefix(f.instance.name + "_" + f.name)

generated_code += " #include <stdlib.h>// sugar@7\n"
generated_code += " #include <string.h>// sugar@8\n"
generated_code += " #include \"odp_lib.h\"// sugar@9\n"
generated_code += " #include \"actions.h\"// sugar@10\n"
generated_code += " \n"
# #[ #define FIELD_BYTE_ADDR(p, f) (((uint8_t*)(p)->headers[f.header].pointer)+f.byteoffset)
# #[ #define EXTRACT_INT32(pd, field, dst) f = field_desc(field); dst = (*(uint32_t *)(FIELD_BYTE_ADDR(pd, f)) & f.mask) >> f.bitoffset;
# #[ #define EXTRACT_BYTEBUF(pd, field, dst) f = field_desc(field); memcpy(dst, FIELD_BYTE_ADDR(pd, f), f.bytewidth);
generated_code += " \n"
generated_code += " extern void parse_packet(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@16\n"
generated_code += "\n"
for table in hlir.p4_tables.values():
    generated_code += " void apply_table_" + str(table.name) + "(packet_descriptor_t* pd, lookup_table_t** tables);// sugar@19\n"
generated_code += "\n"

def match_type_order(t):
    if t is p4.p4_match_type.P4_MATCH_EXACT:   return 0
    if t is p4.p4_match_type.P4_MATCH_LPM:     return 1
    if t is p4.p4_match_type.P4_MATCH_TERNARY: return 2

for table in hlir.p4_tables.values():
    generated_code += " void table_" + str(table.name) + "_key(packet_descriptor_t* pd, uint8_t* key) {// sugar@28\n"
    generated_code += "     field_reference_t f;// sugar@29\n"
    generated_code += "     (void)f;// sugar@30\n"
    sortedfields = sorted(table.match_fields, key=lambda field: match_type_order(field[1]))
    for match_field, match_type, match_mask in sortedfields:
        if match_field.width <= 32:
            generated_code += " EXTRACT_INT32(pd, " + str(fld_id(match_field)) + ", *(uint32_t*)key)// sugar@34\n"
            generated_code += " key += sizeof(uint32_t);// sugar@35\n"
        elif match_field.width > 32 and match_field.width % 8 == 0:
            byte_width = (match_field.width+7)/8
            generated_code += " EXTRACT_BYTEBUF(pd, " + str(fld_id(match_field)) + ", key)// sugar@38\n"
            generated_code += " key += " + str(byte_width) + ";// sugar@39\n"
        else:
            print "Unsupported field %s ignored in key calculation." % fld_id(match_field)
    generated_code += " }// sugar@42\n"
    generated_code += "\n"

for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    lookupfun = {'LOOKUP_LPM':'lpm_lookup', 'LOOKUP_EXACT':'exact_lookup', 'LOOKUP_TERNARY':'ternary_lookup'}
    generated_code += " void apply_table_" + str(table.name) + "(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@48\n"
    generated_code += " {// sugar@49\n"
    generated_code += "     #if debug == 1// sugar@50\n"
    generated_code += "     printf(\"  :::: EXECUTING TABLE " + str(table.name) + "\\n\");// sugar@51\n"
    generated_code += "     #endif// sugar@52\n"
    generated_code += "     uint8_t* key[" + str(key_length) + "];// sugar@53\n"
    generated_code += "     table_" + str(table.name) + "_key(pd, (uint8_t*)key);// sugar@54\n"
    generated_code += "     struct " + str(table.name) + "_action* res = (struct " + str(table.name) + "_action*)" + str(lookupfun[table_type]) + "(tables[TABLE_" + str(table.name) + "], (uint8_t*)key);// sugar@55\n"
    generated_code += "     if(res == NULL) {// sugar@56\n"
    generated_code += "         #if debug == 1// sugar@57\n"
    generated_code += "         printf(\"    :: NO RESULT, NO DEFAULT ACTION, IGNORING PACKET... key:0x%08x\\n\", (unsigned)*key);// sugar@58\n"
    generated_code += "         #endif// sugar@59\n"
    generated_code += "         return;// sugar@60\n"
    generated_code += "     }// sugar@61\n"
    generated_code += "     switch (res->action_id) {// sugar@62\n"
    for action in table.actions:
        generated_code += " case action_" + str(action.name) + ":// sugar@64\n"
        generated_code += "   #if debug == 1// sugar@65\n"
        generated_code += "   printf(\"    :: EXECUTING ACTION " + str(action.name) + "...\\n\");// sugar@66\n"
        generated_code += "   #endif// sugar@67\n"
        if action.signature:
            generated_code += " action_code_" + str(action.name) + "(pd, tables, res->" + str(action.name) + "_params);// sugar@69\n"
        else:
            generated_code += " action_code_" + str(action.name) + "(pd, tables);// sugar@71\n"
        generated_code += "     break;// sugar@72\n"
    generated_code += "     }// sugar@73\n"
    if "hit" in table.next_:
        generated_code += " // apply_table_" + str(table.next_['hit'].name) + "(pd, tables);// sugar@75\n"
        generated_code += " // apply_table_" + str(table.next_['miss'].name) + "(pd, tables);// sugar@76\n"
    else:
        generated_code += " switch (res->action_id) {// sugar@78\n"
        for action, nexttable in table.next_.items():
            if nexttable != None:
                generated_code += " case action_" + str(action.name) + ":// sugar@81\n"
                generated_code += "     apply_table_" + str(nexttable.name) + "(pd, tables);// sugar@82\n"
                generated_code += "     break;// sugar@83\n"
        generated_code += " }// sugar@84\n"
    generated_code += " }// sugar@85\n"
    generated_code += "\n"

generated_code += " \n"
generated_code += " void handle_packet(packet_descriptor_t* pd, lookup_table_t** tables)// sugar@89\n"
generated_code += " {// sugar@90\n"
generated_code += "     #if debug == 1// sugar@91\n"
generated_code += "     printf(\"### HANDLING PACKET ARRIVING AT PORT %\" PRIu32 \"...\\n\", extract_intvalue(pd, field_desc(field_instance_standard_metadata_ingress_port)));// sugar@92\n"
generated_code += "     #endif// sugar@93\n"
generated_code += "     parse_packet(pd, tables);// sugar@94\n"
generated_code += " }// sugar@95\n"