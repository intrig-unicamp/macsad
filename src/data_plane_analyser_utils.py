
from p4_hlir.main import HLIR
from p4_hlir.hlir.p4_headers import p4_field, p4_field_list
from p4_hlir.hlir.p4_imperatives import p4_signature_ref
from p4_hlir.hlir.p4_tables import p4_match_type
from utils import build_hlir
from inspect import currentframe, getframeinfo, getouterframes
from os.path import basename

simplePrimitives = set(["drop", "no_op"])


def init_data_plane_analyser(act_file):
    """Sets up a huge amount of global variables.
       Some of them are filled by the compiler infrastructure,
       and some will be filled by the analysers.
       This method must not be called more than once."""
    global hlir, TABSIZE, TABIN, actions, headers, useractions, is_first_run, tables, field_lists

    TABSIZE = 3
    TABIN = ""

    hlir = HLIR(act_file)
    build_hlir(hlir)

    actions = hlir.p4_actions
    headers = hlir.p4_headers
    tables = hlir.p4_tables
    field_lists = hlir.p4_field_lists
    useractions = []

    initVars()

    for key in actions.keys():
        val = actions[key]
        if notPrimitive(val):
            useractions += [key]

def initVars():
    global decl, code, macros, error, block
   
    decl = ""
    code = ""
    macros = ""
    block = ""
    error = ""

def initDataPlane():
   addDecl([
        "#ifndef __DATA_PLANE_MAIN_H__",
        "#define __DATA_PLANE_MAIN_H__\n",
        "#include <stdint.h>",
        '#include "aliases.h"',
        '#include "parser.h"',
        '#include "dataplane.h"',
        ])
   addDecl("#define FIELD(name,length) uint8_t name[(length + 7) / 8];\n\n")
   add([
        '#include "data_plane_data.h"\n',
        '#include "backend.h"\n'
    ],"\n")
   
   # TODO put in real value in the place of 42
   add("int header[42];\n")
   # TODO should be more meaningful
   add("int conv(int val) { return val; }\n")

# TODO: better condition...
def notPrimitive(action):
    return ((action.signature_flags == {}) and (not (action.name in simplePrimitives)))


def incTAB():
    global TABIN
    TABIN += TABSIZE * " "


def decTAB():
    global TABIN
    TABIN = TABIN[:-TABSIZE]

def TAB():
    global TABIN
    return TABIN


def generation_info():
    """Returns a C // comment that indicates where the function calling this one was called from."""
    frameinfo = getouterframes(currentframe())[2]
    filename = basename(frameinfo[1])
    lineno = frameinfo[2]
    return "// in %s:%d\n" % (filename, lineno)


def addDecl(decl_part):
    global decl
    decl += generation_info()
    if isinstance(decl_part, str):
        decl += decl_part + "\n"
    else:
        decl += ("\n").join(decl_part) + "\n"


def addField(field):
    addDecl("   FIELD(%s, FIELD_LENGTH__%s)" % (field, field))


def add(code_part, sep=", "):
    global code
    if isinstance(code_part, str):
      code += code_part
    else:
      code += sep.join(code_part)


def addL(code_part="", sep=", "):
    global code
    code += generation_info()
    add(code_part, sep)
    add("\n")


def addMacro(code_part):
    global macros
    macros += generation_info()
    if isinstance(code_part, str):
        macros += TAB() + "#define " + code_part + "\n"
    else:
        macros += TAB() + "#define " + ("\n" + TAB() + "#define ").join(code_part) + "\n"


def commitMacros():
    global code, macros
    code += macros
    macros = ""


def addBlock(code_part, sep = ", "):
    global block
    block += generation_info()
    if isinstance(code_part, str):
        block += code_part
    else:
        block += sep.join(code_part)


def addBlockL(code_part="", sep=", "):
    addBlock(code_part, sep)
    addBlock("\n")


def commitBlock():
    global code, block
    code += block
    block = ""


def addFun(name, hasParam):
    # "inline __attribute__((always_inline))"
    modifiers = ""
    ret_val_type = "void"
    params = ", struct params_type_%s *parameters" % (name) if hasParam else ""
    addL(TAB() + "%s %s action_code_%s(packet_descriptor_t* pkt %s) {" % (modifiers, ret_val_type, name, params))
    commitMacros()
    addL()
    commitBlock()
    addL("}")
    addL()


def endDataPlane():
    addDecl("#endif // __DATA_PLANE_MAIN_H__\n\n")

def result():
    global decl, code, error

    return decl, code, error


def addError(s):
    global error
    error += generation_info()
    error += s


def addCodeSection(name):
    add("\n///////////////////////////////////////////////////\n")
    add("//// " + name + "\n\n")
