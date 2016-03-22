from utils import getTypeAndLength

generated_code += " #include \"dataplane.h\"// sugar@3\n"
generated_code += " #include \"actions.h\"// sugar@4\n"
generated_code += " #include \"data_plane_data.h\"// sugar@5\n"
generated_code += "\n"
generated_code += " lookup_table_t table_config[NB_TABLES] = {// sugar@7\n"
for table in hlir.p4_tables.values():
    table_type, key_length = getTypeAndLength(table)
    generated_code += " {// sugar@10\n"
    generated_code += "  .name= \"" + str(table.name) + "\",// sugar@11\n"
    generated_code += "  .type = " + str(table_type) + ",// sugar@12\n"
    generated_code += "  .key_size = " + str(key_length) + ",// sugar@13\n"
    generated_code += "  .val_size = sizeof(struct " + str(table.name) + "_action),// sugar@14\n"
    generated_code += "  .min_size = 0, //" + str(table.min_size) + ",// sugar@15\n"
    generated_code += "  .max_size = 255 //" + str(table.max_size) + "// sugar@16\n"
    generated_code += " },// sugar@17\n"
generated_code += " };// sugar@18\n"