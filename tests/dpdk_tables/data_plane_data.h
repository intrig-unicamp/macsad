#ifndef __DATA_PLANE_MAIN_H__
#define __DATA_PLANE_MAIN_H__

#define NB_TABLES 3
#define TABLE1 0
#define TABLE2 1
#define TABLE3 2
#define HEADER_INSTANCE_COUNT 0

enum header_instance_e {a};
enum field_instance_e {b};

// ----------------------------------------------------------------------------

struct type_field_list {
  int fields_quantity;
  int **field_headers;
  int *field_offsets;
  int *field_widths;
};
struct params_type_forward {};
struct params_type_for_test {};
union table_action_params__dmac_t {};

#endif // __DATA_PLANE_MAIN_H__
