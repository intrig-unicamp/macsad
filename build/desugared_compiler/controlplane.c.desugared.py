from utils import getTypeAndLength
import p4_hlir.hlir.p4 as p4
from header_info import fld_prefix

def fld_id(f): return fld_prefix(f.instance.name + "_" + f.name)

def match_type_order(t):
    if t is p4.p4_match_type.P4_MATCH_EXACT:   return 0
    if t is p4.p4_match_type.P4_MATCH_LPM:     return 1
    if t is p4.p4_match_type.P4_MATCH_TERNARY: return 2

generated_code += " #include \"odp_lib.h\"// sugar@12\n"
generated_code += " #include \"actions.h\"// sugar@13\n"
generated_code += " \n"
generated_code += " extern void table_setdefault_promote  (int tableid, uint8_t* value);// sugar@15\n"
generated_code += " extern void exact_add_promote  (int tableid, uint8_t* key, uint8_t* value);// sugar@16\n"
generated_code += " extern void lpm_add_promote    (int tableid, uint8_t* key, uint8_t depth, uint8_t* value);// sugar@17\n"
generated_code += " extern void ternary_add_promote(int tableid, uint8_t* key, uint8_t* mask, uint8_t* value);// sugar@18\n"
generated_code += "\n"
for table in hlir.p4_tables.values():
    generated_code += " extern void table_" + str(table.name) + "_key(packet_descriptor_t* pd, uint8_t* key); // defined in dataplane.c// sugar@21\n"
generated_code += " \n"

for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    generated_code += " void// sugar@26\n"
    generated_code += " " + str(table.name) + "_add(// sugar@27\n"
    for match_field, match_type, match_mask in table.match_fields:
        byte_width = (match_field.width+7)/8
        generated_code += " uint8_t " + str(fld_id(match_field)) + "[" + str(byte_width) + "],// sugar@30\n"
        ###if match_type is p4_match_type.P4_MATCH_EXACT:
        if match_type is p4.p4_match_type.P4_MATCH_TERNARY:
            generated_code += " uint8_t " + str(fld_id(match_field)) + "_mask[" + str(byte_width) + "],// sugar@33\n"
        if match_type is p4.p4_match_type.P4_MATCH_LPM:
            generated_code += " uint8_t " + str(fld_id(match_field)) + "_prefix_length,// sugar@35\n"
    generated_code += " struct " + str(table.name) + "_action action)// sugar@36\n"
    generated_code += " {// sugar@37\n"
    generated_code += "     uint8_t key[" + str(key_length) + "];// sugar@38\n"
    sortedfields = sorted(table.match_fields, key=lambda field: match_type_order(field[1]))
    k = 0
    for match_field, match_type, match_mask in sortedfields:
        byte_width = (match_field.width+7)/8
        generated_code += " memcpy(key+" + str(k) + ", " + str(fld_id(match_field)) + ", " + str(byte_width) + ");// sugar@43\n"
        k += byte_width;
    if table_type == "LOOKUP_LPM":
        generated_code += " uint8_t prefix_length = 0;// sugar@46\n"
        for match_field, match_type, match_mask in table.match_fields:
            byte_width = (match_field.width+7)/8
            if match_type is p4.p4_match_type.P4_MATCH_EXACT:
                generated_code += " prefix_length += " + str(match_field.width) + ";// sugar@50\n"
            if match_type is p4.p4_match_type.P4_MATCH_LPM:
                generated_code += " prefix_length += " + str(fld_id(match_field)) + "_prefix_length;// sugar@52\n"
        generated_code += " lpm_add_promote(TABLE_" + str(table.name) + ", (uint8_t*)key, prefix_length, (uint8_t*)&action);// sugar@53\n"
    if table_type == "LOOKUP_EXACT":
        for match_field, match_type, match_mask in table.match_fields:
            byte_width = (match_field.width+7)/8
        generated_code += " exact_add_promote(TABLE_" + str(table.name) + ", (uint8_t*)key, (uint8_t*)&action);// sugar@57\n"
    generated_code += " }// sugar@58\n"
    generated_code += "\n"
    generated_code += " void// sugar@60\n"
    generated_code += " " + str(table.name) + "_setdefault(struct " + str(table.name) + "_action action)// sugar@61\n"
    generated_code += " {// sugar@62\n"
    generated_code += "     table_setdefault_promote(TABLE_" + str(table.name) + ", (uint8_t*)&action);// sugar@63\n"
    generated_code += " }// sugar@64\n"


for table in hlir.p4_tables.values():
    generated_code += " void// sugar@68\n"
    generated_code += " " + str(table.name) + "_add_table_entry(struct p4_ctrl_msg* ctrl_m) {// sugar@69\n"
    for i, m in enumerate(table.match_fields):
        match_field, match_type, match_mask = m
        if match_type is p4.p4_match_type.P4_MATCH_EXACT:
            generated_code += " uint8_t* " + str(fld_id(match_field)) + " = (uint8_t*)(((struct p4_field_match_exact*)ctrl_m->field_matches[" + str(i) + "])->bitmap);// sugar@73\n"
        if match_type is p4.p4_match_type.P4_MATCH_LPM:
            generated_code += " uint8_t* " + str(fld_id(match_field)) + " = (uint8_t*)(((struct p4_field_match_lpm*)ctrl_m->field_matches[" + str(i) + "])->bitmap);// sugar@75\n"
            generated_code += " uint16_t " + str(fld_id(match_field)) + "_prefix_length = ((struct p4_field_match_lpm*)ctrl_m->field_matches[" + str(i) + "])->prefix_length;// sugar@76\n"
    for action in table.actions:
        generated_code += " if(strcmp(\"" + str(action.name) + "\", ctrl_m->action_name)==0) {// sugar@78\n"
        generated_code += "     struct " + str(table.name) + "_action action;// sugar@79\n"
        generated_code += "     action.action_id = action_" + str(action.name) + ";// sugar@80\n"
        if action.signature:
            generated_code += " action." + str(action.name) + "_params;// sugar@82\n"
        for j, (name, length) in enumerate(zip(action.signature, action.signature_widths)):
            generated_code += " uint8_t* " + str(name) + " = (uint8_t*)((struct p4_action_parameter*)ctrl_m->action_params[" + str(j) + "])->bitmap;// sugar@84\n"
            generated_code += " memcpy(action." + str(action.name) + "_params." + str(name) + ", " + str(name) + ", " + str((length+7)/8) + ");// sugar@85\n"
        generated_code += "     printf(\"Reply from the control plane arrived.\\n\");// sugar@86\n"
        generated_code += "     printf(\"Addig new entry to " + str(table.name) + " with action " + str(action.name) + "\\n\");// sugar@87\n"
        generated_code += "     " + str(table.name) + "_add(// sugar@88\n"
        for m in table.match_fields:
            match_field, match_type, match_mask = m
            generated_code += " " + str(fld_id(match_field)) + ",// sugar@91\n"
            if match_type is p4.p4_match_type.P4_MATCH_LPM:
                generated_code += " " + str(fld_id(match_field)) + "_prefix_length,// sugar@93\n"
        generated_code += "     action);// sugar@94\n"
        generated_code += " }// sugar@95\n"
    generated_code += " }// sugar@96\n"

generated_code += " void recv_from_controller(struct p4_ctrl_msg* ctrl_m) {// sugar@98\n"
generated_code += "     if (ctrl_m->type == P4T_ADD_TABLE_ENTRY) {// sugar@99\n"
for table in hlir.p4_tables.values():
    generated_code += " if (strcmp(\"" + str(table.name) + "\", ctrl_m->table_name) == 0)// sugar@101\n"
    generated_code += "     " + str(table.name) + "_add_table_entry(ctrl_m);// sugar@102\n"
generated_code += "     }// sugar@103\n"
generated_code += " }// sugar@104\n"



generated_code += " backend bg;// sugar@108\n"
generated_code += " void init_control_plane()// sugar@109\n"
generated_code += " {// sugar@110\n"
generated_code += "     printf(\"Creating control plane connection...\\n\");// sugar@111\n"
generated_code += "     bg = create_backend(3, 1000, \"localhost\", 11111, recv_from_controller);// sugar@112\n"
generated_code += "     launch_backend(bg);// sugar@113\n"
if "smac" in hlir.p4_tables:
    generated_code += " struct smac_action action;// sugar@115\n"
    generated_code += " action.action_id = action_mac_learn;// sugar@116\n"
    generated_code += " smac_setdefault(action);// sugar@117\n"
    generated_code += " printf(\"smac setdefault\\n\");// sugar@118\n"
if "dmac" in hlir.p4_tables:
    generated_code += " struct dmac_action action2;// sugar@120\n"
    generated_code += " action2.action_id = action_bcast;// sugar@121\n"
    generated_code += " dmac_setdefault(action2);// sugar@122\n"
    generated_code += " printf(\"dmac setdefault\\n\");// sugar@123\n"
generated_code += " }// sugar@124\n"