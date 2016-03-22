
#ifndef __CONTROL_PLANE_ACTIONS_H_
#define __CONTROL_PLANE_ACTIONS_H_

#include <p4_hw_tables.h>

// Currently the only sort of action that the control plane
// can send to the data plane.
struct control_plane_action_set_table_entry
{
	struct table_t* table;
	int entry_index;
	char* data;
};

#endif
