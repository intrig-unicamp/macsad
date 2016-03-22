#ifndef __DATA_PLANE_DATA_H__
#define __DATA_PLANE_DATA_H__

struct type_field_list {
  int fields_quantity;
  int **field_headers;
  int *field_offsets;
  int *field_widths;
};
struct params_type_forward {};
struct params_type_for_test {};
union table_action_params__dmac_t {};

#include "header_info.h"

#endif // __DATA_PLANE_DATA_H__
