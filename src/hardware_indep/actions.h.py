#[ #ifndef __ACTION_INFO_GENERATED_H__
#[ #define __ACTION_INFO_GENERATED_H__
#[ 
#[
#[ #define FIELD(name, length) uint8_t name[(length + 7) / 8];
#[

#[ enum actions {
a = []
for table in hlir.p4_tables.values():
    for action in table.actions:
        if a.count(action.name) == 0:
            a.append(action.name)
            #[ action_${action.name},
#[ };

for table in hlir.p4_tables.values():
    for action in table.actions:
        if action.signature:
            #[ struct action_${action.name}_params {
            for name, length in zip(action.signature, action.signature_widths):
                #[ FIELD(${name}, ${length});
            #[ };

for table in hlir.p4_tables.values():
    #[ struct ${table.name}_action {
    #[     int action_id;
    #[     union {
    for action in table.actions:
        if action.signature:
            #[ struct action_${action.name}_params ${action.name}_params;
    #[     };
    #[ };

for table in hlir.p4_tables.values():
    #[ void apply_table_${table.name}(packet_descriptor_t *pd, lookup_table_t** tables);
    for action in table.actions:
        if action.signature:
            #[ void action_code_${action.name}(packet_descriptor_t *pd, lookup_table_t **tables, struct action_${action.name}_params);
        else:
            #[ void action_code_${action.name}(packet_descriptor_t *pd, lookup_table_t **tables);

#[ #endif
