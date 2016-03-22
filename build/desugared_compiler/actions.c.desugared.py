from p4_hlir.hlir.p4_headers import p4_field, p4_field_list
from p4_hlir.hlir.p4_imperatives import p4_signature_ref
from utils import *

from header_info import hdr_prefix, fld_prefix
def fld_id(f): return fld_prefix(f.instance.name + "_" + f.name)

generated_code += " #include \"odp_lib.h\"// sugar@8\n"
generated_code += " #include \"actions.h\"// sugar@9\n"
generated_code += " #include <unistd.h>// sugar@10\n"
generated_code += " #include <arpa/inet.h>// sugar@11\n"

actions = hlir.p4_actions
useractions = userActions(actions)
useraction_objs = [(actions[act_key]) for act_key in useractions ]

def modify_field(fun, call):
    generated_code = ""
    args = call[1]
    dst = args[0]
    src = args[1]
    # mask = args[2]
    if not isinstance(dst, p4_field):
        addError("generating modify_field", "We do not allow changing an R-REF yet")
    if isinstance(src, int):
        generated_code += " value32 = " + str(src) + ";// sugar@26\n"
        if dst.width <= 32:
            if not dst.instance.metadata:
                generated_code += " value32 = htonl(value32);// sugar@29\n"
            generated_code += " MODIFY_INT32_INT32(pkt, " + str(fld_id(dst)) + ", value32)// sugar@30\n"
        else:
            if dst.width % 8 == 0 and dst.offset % 8 == 0:
                generated_code += " MODIFY_BYTEBUF_INT32(pkt, " + str(fld_id(dst)) + ", value32)// sugar@33\n"
            else:
                addError("generating modify_field", "Improper bytebufs cannot be modified yet.")
    elif isinstance(src, p4_field):
        if dst.width <= 32 and src.length <= 32:
            generated_code += " EXTRACT_INT32(pkt, " + str(fld_id(src)) + ", value32)// sugar@38\n"
            if not src.instance.metadata:
                generated_code += " value32 = ntohl(value32);// sugar@40\n"
            generated_code += " MODIFY_INT32_INT32(pkt, " + str(fld_id(dst)) + ", value32)// sugar@41\n"
        elif src.width != dst.width:
            addError("generating modify_field", "bytebuf field-to-field of different widths is not supported yet")
        else:
            if dst.width % 8 == 0 and dst.offset % 8 == 0 and src.width % 8 == 0 and src.offset % 8 == 0 and src.instance.metadata == dst.instance.metadata:
                generated_code += " MODIFY_BYTEBUF_BYTEBUF(pkt, " + str(fld_id(dst)) + ", FIELD_BYTE_ADDR(pkt, field_desc(" + str(fld_id(src)) + ")), (field_desc(" + str(fld_id(dst)) + ")).bytewidth)// sugar@46\n"
            else:
                addError("generating modify_field", "Improper bytebufs cannot be modified yet.")
    elif isinstance(src, p4_signature_ref):
        p = "parameters.%s" % str(fun.signature[src.idx])
        l = fun.signature_widths[src.idx]
        if dst.width <= 32 and l <= 32:
            if not dst.instance.metadata:
                generated_code += " value32 = htonl(value32);// sugar@54\n"
            generated_code += " MODIFY_INT32_BYTEBUF(pkt, " + str(fld_id(dst)) + ", " + str(p) + ", " + str((l+7)/8) + ")// sugar@55\n"
        else:
            if dst.width % 8 == 0 and dst.offset % 8 == 0 and l % 8 == 0: #and dst.instance.metadata:
                generated_code += " MODIFY_BYTEBUF_BYTEBUF(pkt, " + str(fld_id(dst)) + ", " + str(p) + ", (field_desc(" + str(fld_id(dst)) + ")).bytewidth)// sugar@58\n"
            else:
                addError("generating modify_field", "Improper bytebufs cannot be modified yet.")        
    return generated_code

def add_to_field(fun, call):
    generated_code = ""
    args = call[1]
    dst = args[0]
    val = args[1]
    if not isinstance(dst, p4_field):
        addError("generating add_to_field", "We do not allow changing an R-REF yet")
    if isinstance(val, int):
        generated_code += " value32 = " + str(val) + ";// sugar@71\n"
        if dst.width <= 32:
            generated_code += " EXTRACT_INT32(pkt, " + str(fld_id(dst)) + ", res32)// sugar@73\n"
            if not dst.instance.metadata:
                generated_code += " res32 = ntohl(res32);// sugar@75\n"
            generated_code += " value32 += res32;// sugar@76\n"
            if not dst.instance.metadata:
                generated_code += " value32 = htonl(value32);// sugar@78\n"
            generated_code += " MODIFY_INT32_INT32(pkt, " + str(fld_id(dst)) + ", value32)// sugar@79\n"
        else:
            addError("generating modify_field", "Bytebufs cannot be modified yet.")
    elif isinstance(val, p4_field):
        if dst.width <= 32 and val.length <= 32:
            generated_code += " EXTRACT_INT32(pkt, " + str(fld_id(val)) + ", value32)// sugar@84\n"
            generated_code += " EXTRACT_INT32(pkt, " + str(fld_id(dst)) + ", res32)// sugar@85\n"
            generated_code += " value32 += res32;// sugar@86\n"
            if not val.instance.metadata:
                generated_code += " value32 = ntohl(value32);// sugar@88\n"
            generated_code += " MODIFY_INT32_INT32(pkt, " + str(fld_id(dst)) + ", value32)// sugar@89\n"
        else:
            addError("generating add_to_field", "bytebufs cannot be modified yet.")
    elif isinstance(val, p4_signature_ref):
        p = "parameters.%s" % str(fun.signature[val.idx])
        l = fun.signature_widths[val.idx]
        if dst.width <= 32 and l <= 32:
            if not dst.instance.metadata:
                generated_code += " value32 = htonl(value32);// sugar@97\n"
            generated_code += " EXTRACT_INT32(pkt, " + str(fld_id(dst)) + ", res32)// sugar@98\n"
            generated_code += " TODO// sugar@99\n"
        else:
            addError("generating add_to_field", "bytebufs cannot be modified yet.")
    return generated_code

def create_generate_digest(call, fun_params, fun):
    generated_code = ""
    
    ## TODO make this proper
    fun_params += ["\"mac_learn_digest\""]
    for p in call[1]:
        if isinstance(p, int):
            fun_params += "0" #[str(p)]
        elif isinstance(p, p4_field_list):
            field_list = p
            fun_params += ["&fields"]
        else:
            addError("generating actions.c", "Unhandled parameter type in generate_digest: " + str(p))
 
    generated_code += "  struct type_field_list fields;// sugar@118\n"
    quan = str(len(field_list.fields))
    generated_code += "    fields.fields_quantity = " + str(quan) + ";// sugar@120\n"
    generated_code += "    fields.field_offsets = malloc(sizeof(uint8_t*)*fields.fields_quantity);// sugar@121\n"
    generated_code += "    fields.field_widths = malloc(sizeof(uint8_t*)*fields.fields_quantity);// sugar@122\n"
    for i,field in enumerate(field_list.fields):
        j = str(i)
        if isinstance(field, p4_field):
            generated_code += "    fields.field_offsets[" + str(j) + "] = (uint8_t*) (pkt->headers[header_instance_" + str(field.instance) + "].pointer + field_instance_byte_offset_hdr[field_instance_" + str(field.instance) + "_" + str(field.name) + "]);// sugar@126\n"
            generated_code += "    fields.field_widths[" + str(j) + "] = field_instance_bit_width[field_instance_" + str(field.instance) + "_" + str(field.name) + "]*8;// sugar@127\n"
        else:
            addError("generating actions.c", "Unhandled parameter type in field_list: " + name + ", " + str(field))

    params = ",".join(fun_params)
    generated_code += "\n"
    generated_code += "    generate_digest(" + str(params) + "); sleep(1);// sugar@133\n"
    return generated_code

def create_drop(call, fun_params, fun):
    params = ",".join(fun_params)
    val = "drop(%s);" % params
    return val

generated_code += " extern backend bg;// sugar@141\n"
generated_code += "\n"


for fun in useraction_objs:
    hasParam = fun.signature
    modifiers = ""
    ret_val_type = "void"
    name = fun.name
    params = ", struct action_%s_params parameters" % (name) if hasParam else ""
    generated_code += " " + str(modifiers) + " " + str(ret_val_type) + " action_code_" + str(name) + "(packet_descriptor_t* pkt, lookup_table_t** tables " + str(params) + ") {// sugar@151\n"
    generated_code += "     uint32_t value32, res32;// sugar@152\n"
    generated_code += "     (void)value32;// sugar@153\n"
    generated_code += "     (void)res32;// sugar@154\n"
    for i,call in enumerate(fun.call_sequence):
        # NOTE: called functions require the packet as the first argument
        fun_params = ["pkt"]
        if call[0].name == "modify_field":
            generated_code += " " + str(modify_field(fun, call)) + "// sugar@159\n"
        elif call[0].name == "add_to_field":
            generated_code += " " + str(add_to_field(fun, call)) + "// sugar@161\n"
        elif call[0].name == "generate_digest":
            fun_params = ["bg"]
            generated_code += create_generate_digest(call, fun_params, fun);
        elif call[0].name == "drop":
            code = create_drop(call, fun_params, fun)
            generated_code += " " + str(code) + "// sugar@167\n"
        else:
            addWarning("generating actions.c", "Unhandled primitive function: " +  call[0].name)     
            
    generated_code += " }// sugar@171\n"
    generated_code += "\n"
