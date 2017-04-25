#[ #ifndef __DATA_PLANE_DATA_H__
#[ #define __DATA_PLANE_DATA_H__
#[
#[ #include "parser.h"
#[
#[ #define NB_TABLES ${len(hlir.p4_tables)}
#[
#[ enum table_names {
for table in hlir.p4_tables.values():
    #[ TABLE_${table.name},
#[ TABLE_
#[ };
#[
#[ #define NB_COUNTERS ${len(hlir.p4_counters)}
#[
#[ enum counter_names {
for counter in hlir.p4_counters.values():
    #[ COUNTER_${counter.name},
#[ COUNTER_
#[ };
#[
#[ #define NB_REGISTERS ${len(hlir.p4_registers)}
#[
#[ enum register_names {
for register in hlir.p4_registers.values():
    #[ REGISTER_${register.name},
#[ REGISTER_
#[ };
#[
#[ #endif // __DATA_PLANE_DATA_H__
