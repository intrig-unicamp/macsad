#!/usr/bin/env python

from hlir16.p4node import P4Node, deep_copy
from hlir16.hlir16 import get_fresh_node_id

def apply_annotations(postfix, extra_args, x):
    if (x.methodCall.method.node_type=="PathExpression") :
        new_decl = deep_copy(x.methodCall.method.ref)
        new_decl.name += "_" + postfix
        extra_args_types = [ P4Node({'id' : get_fresh_node_id(),
                                   'node_type' : 'Parameter',
                                   'type' : arg.type,
                                   'name' : postfix + "_extra_param",
                                   'direction' : 'in',
                                   'declid' : 0,         #TODO: is this important?
                                   'annotations' : []
                                   }) for arg in extra_args ]
        new_decl.type.parameters.parameters.vec += extra_args_types
        x.methodCall.method.ref = new_decl
        x.methodCall.method.path.name += "_" + postfix
        x.methodCall.arguments.vec += extra_args
    return x

def search_for_annotations(x):
    available_optimization_annotations = ['offload']

    if (x.node_type == "BlockStatement") :
        name_list = [annot.name for annot in x.annotations.annotations.vec if annot.name in available_optimization_annotations]
        arg_list = []
        for annot in x.annotations.annotations.vec :
            if annot.name in available_optimization_annotations : arg_list += annot.expr
        if ([] != name_list) :
	    x.components = map(lambda x : apply_annotations('_'.join(name_list), arg_list, x), x.components)
    return x

def transform_hlir16(hlir16):

    main = hlir16.declarations['Declaration_Instance'][0] # TODO what if there are more package instances?
    pipeline_elements = main.arguments

    for pe in pipeline_elements:
        c = hlir16.declarations.get(pe.type.name, 'P4Control')
        if c is not None:
            c.body.components = map(search_for_annotations, c.body.components)
