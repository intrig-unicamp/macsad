
#ifndef __TABLES_H_
#define __TABLES_H_

#include "p4_hw_lib.h"

struct table_lookup_result_t
{
	int    type;
	void** fields;
};

struct table_t
{
	const char*const name;
	const int actions_count;
	const char*const*const action_names;

	const int max_entry_count;
	const int entry_size;

	int entry_count;

	// The code of the action to take when a lookup fails.
	int default_action_code;

	void* table;
};

#endif
