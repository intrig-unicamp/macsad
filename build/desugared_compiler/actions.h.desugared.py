from utils import *

generated_code += " #ifndef __ACTION_INFO_GENERATED_H__// sugar@3\n"
generated_code += " #define __ACTION_INFO_GENERATED_H__// sugar@4\n"
generated_code += " \n"
generated_code += "\n"
generated_code += " " + str(fieldMacro()) + "// sugar@7\n"
generated_code += "\n"

generated_code += " enum actions {// sugar@10\n"
a = []
for table in hlir.p4_tables.values():
    for action in table.actions:
        if a.count(action.name) == 0:
            a.append(action.name)
            generated_code += " action_" + str(action.name) + ",// sugar@16\n"
generated_code += " };// sugar@17\n"

for table in hlir.p4_tables.values():
    for action in table.actions:
        if action.signature:
            generated_code += " struct action_" + str(action.name) + "_params {// sugar@22\n"
            for name, length in zip(action.signature, action.signature_widths):
                generated_code += " FIELD(" + str(name) + ", " + str(length) + ");// sugar@24\n"
            generated_code += " };// sugar@25\n"

for table in hlir.p4_tables.values():
    generated_code += " struct " + str(table.name) + "_action {// sugar@28\n"
    generated_code += "     int action_id;// sugar@29\n"
    generated_code += "     union {// sugar@30\n"
    for action in table.actions:
        if action.signature:
            generated_code += " struct action_" + str(action.name) + "_params " + str(action.name) + "_params;// sugar@33\n"
    generated_code += "     };// sugar@34\n"
    generated_code += " };// sugar@35\n"

for table in hlir.p4_tables.values():
    for action in table.actions:
        if action.signature:
            generated_code += " void action_code_" + str(action.name) + "(packet_descriptor_t *pd, lookup_table_t **tables, struct action_" + str(action.name) + "_params);// sugar@40\n"
        else:
            generated_code += " void action_code_" + str(action.name) + "(packet_descriptor_t *pd, lookup_table_t **tables);// sugar@42\n"

generated_code += " #endif// sugar@44\n"