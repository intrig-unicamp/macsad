
from utils.misc import addError, addWarning 
from utils.codegen import format_statement_16
import math

#[ #include "odp_lib.h"
#[ #include "actions.h"
#[ #include <stdlib.h>
#[ #include <unistd.h>

#[ extern ctrl_plane_backend bg;

for ctl in hlir16.controls:
    for act in ctl.actions:
        fun_params = ["packet_descriptor_t* pd", "lookup_table_t** tables"]
        if act.parameters.parameters.vec != []:
            fun_params += ["struct action_{}_params parameters".format(act.name)]

        #{ void action_code_${act.name}(${', '.join(fun_params)}) {
        #[ uint32_t value32, res32, mask32;
        #[ (void)value32; (void)res32; (void)mask32;
        for stmt in act.body.components:
            global statement_buffer
            statement_buffer = ""
            code = format_statement_16(stmt)
            if statement_buffer != "":
                #= statement_buffer
                statement_buffer = ""
            #= code
        #} }
