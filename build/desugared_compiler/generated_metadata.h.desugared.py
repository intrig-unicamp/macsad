
# TODO put it into a common library
def get_field_type(field_name, field_length):
    """Returns textually the C type generated for the field."""
    if (field_length > 32):
        # for now, we suppose that anything longer than 4 bytes
        # is stored as an array
        return "unsigned char*"
    else:
        # note: shorter fields would have shorter representations
        return "void*"



generated_code += " #ifndef __GENERATED_METADATA_H_// sugar@15\n"
generated_code += " #define __GENERATED_METADATA_H_// sugar@16\n"
generated_code += " \n"

list_names = []

for (list_name, field_list) in hlir.p4_field_lists.items():
    list_names.append(list_name)

    generated_code += " struct fieldlist_" + str(list_name) + "_t// sugar@24\n"
    generated_code += " {// sugar@25\n"
    for field in field_list.fields:
        if (field.instance.name.startswith("standard_metadata")):
            generated_code += "     // field " + str(field.name) + " is in standard metadata// sugar@28\n"
            continue

        # TODO set it properly
        field_type = get_field_type(field.instance.name, field.width)

        generated_code += "     " + str(field_type) + " " + str(field.instance.name) + "_" + str(field.name) + ";// sugar@34\n"
    generated_code += " };// sugar@35\n"
    generated_code += " \n"



generated_code += " \n"
generated_code += " union any_fieldlist_t// sugar@41\n"
generated_code += " {// sugar@42\n"
for list_name in list_names:
    generated_code += "     struct fieldlist_" + str(list_name) + "_t fieldlist_" + str(list_name) + ";// sugar@44\n"
generated_code += " };// sugar@45\n"
generated_code += " \n"
generated_code += " #endif// sugar@47\n"