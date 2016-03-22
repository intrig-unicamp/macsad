
#include <stdio.h>
#include <stdlib.h>

#include <pcap.h>

#include <p4_hw_tables.h>


extern void dbg_printf(const int verbosity_level, const char *fmt, ...);

extern const int table_count;
extern const char* table_names[];
extern const char** table_action_names[];
extern const int table_action_names_count[];
extern const unsigned long tables_total_size;

extern void data_plane_learn_mac(unsigned char* mac, int ingress_port);

extern void init_tables();
extern struct table_t* get_table(char* table_name);
extern int action_name_to_id(struct table_t* table, const char* action_name);

extern void add_table_entry(int line_number, struct table_t* table, int action_id, char* line_data);


void process_config_file_line(int line_number, char* line, ssize_t* read)
{
    char  table_name[256];
    char  table_action[256];
    char  line_data[256];

    int parsed_arg_count = sscanf(line, "%s %s %[^\n]", table_name, table_action, line_data);

    if (parsed_arg_count != 3) {
        fprintf(stderr, "Error in config file on line %d\n", line_number);
        exit(1);
    }

    struct table_t* table = get_table(table_name);
    int action_id = action_name_to_id(table, table_action);

    add_table_entry(line_number, table, action_id, line_data);
}

void process_config_file_lines(FILE* fp)
{
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    int line_number = 1;
    while ((read = getline(&line, &len, fp)) != -1) {
        process_config_file_line(line_number, line, &read);
        ++line_number;
    }
}


void init_config_tables(const char* config_file_filename)
{
    FILE* fp = fopen(config_file_filename, "r");

    if (!fp) {
        fprintf(stderr, "Config file not found: %s\n", config_file_filename);
        exit(1);
    }

    process_config_file_lines(fp);

    fclose(fp);
}

void init_control_plane(const char* config_file_filename)
{
    init_tables();
    init_config_tables(config_file_filename);
}
