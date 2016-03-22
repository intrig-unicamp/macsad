from data_plane_analyser_utils import *


def create_field_list():
    addDecl("struct type_field_list{")
    addDecl("   int    fields_quantity;")
    addDecl("   int**  field_headers;")
    addDecl("   int*   field_offsets;")
    addDecl("   int*   field_widths;")
    addDecl("};\n")


def create_fields():
    field_defs = []
    field_inits = []

    for act_key in field_lists.keys():
        field_list = field_lists[act_key]
        name = field_list.name + "_field_list"
        quan_name = (name + "_quantity").upper()
        name2 = field_list.name + "_field"
        quan = str(len(field_list.fields))

        add("struct type_field_list " + name + ";\n")
        addDecl("#define %s %s\n" % (quan_name, quan))

        hds = []
        fds = []
        ws = []
        for field in field_list.fields:
            if isinstance(field, p4_field):
                hds += ["header[instance_idx_%s]" % (field.instance)]
                fds += ["FIELD_OFFSET__%s_%s" % (field.instance.header_type.name, field.name)]
                ws += [str(field.width)]
            else:
                addError("Unhandled parameter type in field_list: " + name + ", " + str(field))

        addDecl("extern int * %s_headers[%s];" % (name2, quan_name))
        addDecl("extern int %s_offsets[%s];" % (name2, quan_name))
        addDecl("extern int %s_widths[%s];" % (name2, quan_name))

        field_inits += name + ".fields_quantity = %s;\n" % (quan_name)
        field_inits += "%s.field_headers = %s_headers;\n" % (name, name2)
        field_inits += name + ".field_offsets = " + name2 + "_offsets;\n"
        field_inits += name + ".field_widths = " + name2 + "_widths;\n"

        field_defs += "int * %s_headers[%s];\n" % (name2, quan_name)
        field_defs += "int %s_offsets[%s];\n" % (name2, quan_name)
        field_defs += "int %s_widths[%s];\n" % (name2, quan_name)

        for idx, hd in enumerate(hds):
            # TODO is the & ok here?
            field_inits += "%s_headers[%d] = &%s;\n" % (name2, idx, hd)

        for idx, fd in enumerate(fds):
            field_inits += "%s_offsets[%s] = %s;\n" % (name2, idx, fd)

        for idx, w in enumerate(ws):
            field_inits += "%s_widths[%s] = %s;\n" % (name2, idx, w)

    addCodeSection("Field definitions")
    for field_def in field_defs:
        add(field_def)

    addCodeSection("Field initialisation")
    add("void init_fields() {\n")
    incTAB()
    for field_init in field_inits:
        add(field_init)
    addL("}\n")
    decTAB()


def create_modify_field(call, fun_params, fun):
    field_width = 0
    hasConstParam = 0
    hasSignParam = 0
    for p in call[1]:
        if isinstance(p, p4_field):
            # hd = str(p.instance).upper()
            # hd_t = str(p.instance.header_type.name).upper()
            # fd = str(p.name).upper()
            # actP = "__%s_%s" % (p.instance, p.name)
            # addMacro("%s_header header[instance_idx_%s]" % (actP, p.instance.header_type.name))
            # addMacro("%s_offset FIELD_OFFSET__%s_%s" % (actP, p.instance.header_type.name, p.name))
            fun_params += ["field_desc(field_instance_%s_%s)" % (p.instance, p.name)]
            if not field_width:
                field_width = p.width
        elif isinstance(p, p4_signature_ref):
            actP = "parameters->" + str(fun.signature[p.idx])
            actL = "FIELD_LENGTH__%s" % (str(fun.signature[p.idx]))
            fun_params += [actP, actL]
            hasSignParam = 1
        elif isinstance(p, int):
            fun_params += ["conv(%s)" % str(p) , "4"]
            hasConstParam = 1
        else:
            addError("Unhandled parameter type in modify_field: " + str(p))

    if hasConstParam or hasSignParam:
        addBlock(TAB() + "modify_field_to_const(")
        addBlock(fun_params)
        addBlockL(");")
    else:
        addBlock(TAB() + "modify_field_to_field(")
        addBlock(fun_params)
        addBlockL(");")


def create_generate_digest(call, fun_params, fun):
    # TODO make this proper
    fun_params += ["\"todo_meaningful_name\""]
    for p in call[1]:
        if isinstance(p, int):
            fun_params += [str(p)]
        elif isinstance(p, p4_field_list):
            fun_params += ["&"+p.name + "_field_list"]
        else:
            addError("Unhandled parameter type in generate_digest: " + str(p))
    addBlockL(TAB() + "generate_digest(%s);" % (",".join(fun_params)))


def create_drop(call, fun_params, fun):
    addBlockL(TAB() + "drop(%s);" % (",".join(fun_params)));

    
def create_user_actions():
    for act_key in useractions:
        fun = actions[act_key]
        incTAB()
        hasParam = fun.signature
        if hasParam:
            for i, sign in enumerate(fun.signature):
                width = fun.signature_widths[i]
                addDecl("#define FIELD_LENGTH__%s %d" % (sign, width))

            addDecl("struct params_type_" + act_key + " {")
            for i, sign in enumerate(fun.signature):
                addField(str(sign))
            addDecl("};\n")
        for call in fun.call_sequence:
            # NOTE: called functions require the packet as the first argument
            fun_params = ["pkt"]
            if call[0].name == "modify_field":
                create_modify_field(call, fun_params, fun)
            elif call[0].name == "generate_digest":
                create_generate_digest(call, fun_params, fun)
            elif call[0].name == "drop":
                create_drop(call, fun_params, fun)
        decTAB()
        addFun(act_key, hasParam)


def build():
    create_field_list()
    create_fields()
    create_user_actions()
