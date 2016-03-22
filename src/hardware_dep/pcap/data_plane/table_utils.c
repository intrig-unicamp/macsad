
#include <stdio.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <p4_hw_tables.h>

extern struct table_t** tables;

extern const int table_count;
extern const char* table_names[];
extern const char** table_action_names[];
extern const int table_action_names_count[];
extern const int table_actions_count[];

extern void dbg_printf(const int verbosity_level, const char *fmt, ...);
extern void dbg_print_hex(int verbosity_level, int length, const char*const data);


// Returns the table by its name, or NULL if not found.
struct table_t* get_table(char* table_name)
{
    for (int i = 0; i < table_count; ++i)
    {
        if (0 == strcmp(tables[i]->name, table_name)) {
            return tables[i];
        }
    }

    return 0;
}

// Returns the id corresponding to the action name, or -1 if not found.
int action_name_to_id(struct table_t* table, const char*const action_name)
{
    const char*const*const action_names  = table->action_names;
    const int              actions_count = table->actions_count;
    for (int i = 0; i < actions_count; ++i)
    {
        if (0 == strcmp(action_names[i], action_name)) {
            return i;
        }
    }

    return -1;
}

void dbg_print_table(int verbosity_level, struct table_t*const table)
{
    for (int i = 0; i < table->entry_count; ++i)
    {
        dbg_printf(verbosity_level, "table %p row %d ", table, i);
        dbg_print_hex(verbosity_level, table->entry_size, table->table + i * table->entry_size);
    }
}

// Returns whether the table already contains the given data.
bool table_contains_entry(struct table_t*const table, const char*const entry_data)
{
    dbg_printf(1, "Lookup in table %p: ", table);
    dbg_print_hex(1, table->entry_size, entry_data);
    dbg_print_table(1, table);


    for (int i = 0; i < table->entry_count; ++i)
    {
        if (0 == memcmp(table->table + i * table->entry_size, entry_data, table->entry_size)) {
            return true;
        }
    }
    return false;
}

// Writes an entry into the given table.
// Intended use: the control plane supplies all required information,
// and is responsible to respect the structure of the table.
void set_table_entry_data(struct table_t*const table, int entry_index, const char*const data)
{
    dbg_printf(1, "Setting entry %d in table %p\n", entry_index, table);
    memcpy(table->table + entry_index * table->entry_size, data, table->entry_size);
}
