
from data_plane_analyser_utils import *


def isActionParam(table):
    for action in table.actions:
        if action.signature:
            return True
    return False


def addStruct(tablename, actionname):
    addDecl("   struct params_type_%s %s_table_%s_params;" % (actionname, tablename, actionname))


def build():
    tables_quan = len(tables.keys())
    addDecl("#define NUMBER_OF_TABLES %d\n" % tables_quan)
    addDecl("extern void * tables[NUMBER_OF_TABLES];\n")
    addL("void * tables[NUMBER_OF_TABLES];\n")
    addL(TAB() + "void create_tables(){")
    incTAB()

    addDecl("extern const header_instance_t field_instance_header[FIELD_INSTANCE_COUNT+1];\n")
    addDecl("extern const int header_instance_is_metadata[HEADER_INSTANCE_COUNT + 1];\n")
    addDecl("extern const int field_instance_bit_width[FIELD_INSTANCE_COUNT + 1];\n")
    addDecl("extern const int field_instance_bit_offset[FIELD_INSTANCE_COUNT + 1];\n")
    addDecl("extern const int field_instance_byte_offset_hdr[FIELD_INSTANCE_COUNT + 1];\n")

    addDecl("#define TABLE_WITHOUT_PARAMETERS 0")

    for i, key in enumerate(tables.keys()):
        table = tables[key]
        key_length = 0
        for field, typ, mx in table.match_fields:
            if typ == p4_match_type.P4_MATCH_EXACT:
                key_length += field.width
            else:
                addError("Currently only exact table match is supported:" + table.name + ", " + field.name)
        addDecl("#define TABLE_IDX__%s %d" % (table.name, i))
        addDecl("#define TABLE_KEY_LEN__%s %d" % (table.name, key_length))
        addDecl("#define TABLE_MIN_SIZE__%s %d" % (table.name, table.min_size))
        addDecl("#define TABLE_MAX_SIZE__%s %d" % (table.name, table.max_size))
        add(TAB() + "tables[TABLE_IDX__%s] = create_table_exact(\"%s\", TABLE_KEY_LEN__%s, TABLE_MIN_SIZE__%s, TABLE_MAX_SIZE__%s" % (table.name, table.name, table.name, table.name, table.name))

        if isActionParam(table):
            type_name = "union table_action_params__" + table.name + "_t"
            addDecl(type_name + " {")
            for action in table.actions:
                if action.signature:
                    addStruct(table.name, action.name)
            addDecl("};\n")
            addL(", sizeof(" + type_name + "));")
        else:
            addL(", TABLE_WITHOUT_PARAMETERS);")
    decTAB()
    addL(TAB() + "}")
