
#[ #ifndef __ACTION_INFO_GENERATED_H__
#[ #define __ACTION_INFO_GENERATED_H__

#[ #define FIELD(name, length) uint8_t name[(length + 7) / 8];

def unique_stable(items):
    """Returns only the first occurrence of the items in a list.
    Equivalent to unique_everseen from Python 3."""
    from collections import OrderedDict
    return list(OrderedDict.fromkeys(items))

# NOTE: hlir16 version
#{ enum actions {
a = {}
for table in hlir16.tables:
    for action in unique_stable(table.actions):
        #[ action_${action.action_object.name},
#} };

for table in hlir16.tables:
    for action in table.actions:
        # if len(action.action_object.parameters) == 0:
        #     continue

        #{ struct action_${action.action_object.name}_params {
        for param in action.action_object.parameters.parameters:
            #[ FIELD(${param.name}, ${param.type.size});
        #} };

for table in hlir16.tables:
    #[ struct ${table.name}_action {
    #[     int action_id;
    #[     union {
    for action in table.actions:
        # TODO what if the action is not a method call?
        # TODO what if there are more actions?
        action_method_name = action.expression.method.ref.name
        #[         struct action_${action.action_object.name}_params ${action_method_name}_params;
    #[     };
    #[ };



for table in hlir16.tables:
    #[ void apply_table_${table.name}(packet_descriptor_t *pd, lookup_table_t** tables);
    for action in table.actions:
        aname = action.action_object.name
        mname = action.expression.method.ref.name

        if len(action.action_object.parameters.parameters) == 0:
            #[ void action_code_$aname(packet_descriptor_t *pd, lookup_table_t **tables);
        else:
            #[ void action_code_$aname(packet_descriptor_t *pd, lookup_table_t **tables, struct action_${mname}_params);


#[ #endif
