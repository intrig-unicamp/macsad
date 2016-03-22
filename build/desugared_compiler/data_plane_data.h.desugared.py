from utils import *

tables = hlir.p4_tables

generated_code += " #ifndef __DATA_PLANE_DATA_H__// sugar@5\n"
generated_code += " #define __DATA_PLANE_DATA_H__// sugar@6\n"
generated_code += "\n"
generated_code += " #include \"parser.h\"// sugar@8\n"
generated_code += "\n"
generated_code += " #define NB_TABLES " + str(len(tables)) + "// sugar@10\n"
generated_code += "\n"
for i, table in enumerate(tables.values()):
    generated_code += " #define TABLE_" + str(table.name) + " " + str(i) + "// sugar@13\n"
generated_code += " \n"
generated_code += " #endif // __DATA_PLANE_DATA_H__// sugar@15\n"