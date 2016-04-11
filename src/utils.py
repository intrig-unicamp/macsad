
import sys
import os

import pprint
from p4_hlir.hlir.p4_tables import p4_match_type

errors = []

warnings = []

def addError(where, msg):
    global errors
    errors += ["ERROR: " + msg + " (While " + where + ").\n"]

def addWarning(where, msg):
    global warnings
    warnings += ["WARNING: " + msg + " (While " + where + ").\n"]

def showErrors():
   global errors
   for e in errors: print e

def showWarnings():
   global warnings
   for w in warnings: print w


disable_hlir_messages = True

simplePrimitives = set(["drop", "no_op"])

# TODO: better condition...
def notPrimitive(action):
    return ((action.signature_flags == {}) and (not (action.name in simplePrimitives)))

def userActions(actions):
    useractions = []
    for key in actions.keys():
        val = actions[key]
        if notPrimitive(val):
            useractions += [key]
    return useractions

def getTypeAndLength(table) : 
   key_length = 0
   lpm = 0
   ternary = 0
   for field, typ, mx in table.match_fields:
       if typ == p4_match_type.P4_MATCH_TERNARY:
           ternary = 1
       elif typ == p4_match_type.P4_MATCH_LPM:
           lpm += 1          
       key_length += field.width
   if (ternary) or (lpm > 1):
      table_type = "LOOKUP_TERNARY"
   elif lpm:
      table_type = "LOOKUP_LPM"
   else:
      table_type = "LOOKUP_EXACT"
   return (table_type, (key_length+7)/8)



def build_hlir(hlir):
    """Builds the P4 internal representation, optionally disabling its output messages."""
    if disable_hlir_messages:
        old_stdout = sys.stdout
        old_stderr = sys.stderr
        sys.stdout = open(os.devnull, 'w')
        sys.stderr = open(os.devnull, 'w')

    hlir.build()

    if disable_hlir_messages:
        sys.stdout = old_stdout
        sys.stderr = old_stderr


def pretty_print(orddict, dictname):
    """Prints a HLIR constructs."""
    print(dictname)
    pprint.pprint(dict(orddict.items()), indent=4)
    print


def pretty_print_all(hlir):
    """Prints all HLIR constructs."""
    pretty_print(hlir.p4_headers, "p4_headers")
    pretty_print(hlir.p4_ingress_ptr, "p4_ingress_ptr")

    pretty_print(hlir.p4_actions, "p4_actions")
    pretty_print(hlir.p4_control_flows, "p4_control_flows")
    pretty_print(hlir.p4_headers, "p4_headers")
    pretty_print(hlir.p4_header_instances, "p4_header_instances")
    pretty_print(hlir.p4_fields, "p4_fields")
    pretty_print(hlir.p4_field_lists, "p4_field_lists")
    pretty_print(hlir.p4_field_list_calculations, "p4_field_list_calculations")
    pretty_print(hlir.p4_parser_exceptions, "p4_parser_exceptions")
    pretty_print(hlir.p4_parse_value_sets, "p4_parse_value_sets")
    pretty_print(hlir.p4_parse_states, "p4_parse_states")
    pretty_print(hlir.p4_counters, "p4_counters")
    pretty_print(hlir.p4_meters, "p4_meters")
    pretty_print(hlir.p4_registers, "p4_registers")
    pretty_print(hlir.p4_nodes, "p4_nodes")
    pretty_print(hlir.p4_tables, "p4_tables")
    pretty_print(hlir.p4_action_profiles, "p4_action_profiles")
    pretty_print(hlir.p4_action_selectors, "p4_action_selectors")
    pretty_print(hlir.p4_conditional_nodes, "p4_conditional_nodes")


def dbg(str):
    print(str)
