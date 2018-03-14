
#[ #ifndef __DATA_PLANE_DATA_H__
#[ #define __DATA_PLANE_DATA_H__
#[
#[ #include "parser.h"
#[
#[ #define NB_TABLES ${len(hlir16.tables)}
#[
#[ enum table_names {
for table in hlir16.tables:
    #[ TABLE_${table.name},
#[ TABLE_
#[ };
#[
#[ // TODO feature temporarily not supported (hlir16)
#[ #define NB_COUNTERS 0
#[
#[ enum counter_names {
#[ COUNTER_
#[ };
#[
#[ // TODO feature temporarily not supported (hlir16)
#[ #define NB_REGISTERS 0
#[
#[ enum register_names {
#[ REGISTER_
#[ };
#[
#[ struct uint8_buffer_t {
#[     int      buffer_size;
#[     uint8_t* buffer;
#[ };
#[
#[ #endif // __DATA_PLANE_DATA_H__
