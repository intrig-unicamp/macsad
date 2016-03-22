
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern int DBG_VERBOSITY;


// If DBG_VERBOSITY is low enough, prints the message, which is formatted similar to printf.
void dbg_printf(int verbosity_level, const char *fmt, ...)
{
    if (DBG_VERBOSITY < verbosity_level)    return;

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}


// Converts a range of bits from an array to an int.
// The converted bits range between low_bit_idx..high_bit_idx, inclusive.
// The range should not be longer than sizeof(int).
int bit_range_to_int(unsigned char* bytes, int low_bit_idx, int high_bit_idx)
{
    const int low_byte_idx  = low_bit_idx / 8;
    const int high_byte_idx = high_bit_idx / 8;
    const int full_low_byte = (int)bytes[low_byte_idx];

    const int low_byte_mask = 0xFF >> (low_bit_idx % 8);
    int retval = (full_low_byte & low_byte_mask);

    for (int i = low_byte_idx + 1; i < high_byte_idx; ++i) {
        retval = (retval << 8) + (int)bytes[i];
    }

    if (low_byte_idx != high_byte_idx) {
        const int high_byte_mask   = 0xFF >> ((high_bit_idx+1) % 8);
        const int masked_high_byte = ((int)bytes[high_byte_idx]) % high_byte_mask;
        retval = (retval << 8) + masked_high_byte;
    }

    return retval;
}

// Similar to bit_range_to_int, only this one creates a long.
long bit_range_to_long(unsigned char* bytes, int low_bit_idx, int high_bit_idx)
{
    const int low_byte_idx  = low_bit_idx / 8;
    const int high_byte_idx = high_bit_idx / 8;
    const long full_low_byte = (long)bytes[low_byte_idx];

    const long low_byte_mask = 0xFF >> (low_bit_idx % 8);
    long retval = (full_low_byte & low_byte_mask);

    for (int i = low_byte_idx + 1; i < high_byte_idx; ++i) {
        retval = (retval << 8) + (long)bytes[i];
    }

    if (low_byte_idx != high_byte_idx) {
        const int high_byte_mask   = 0xFF >> ((high_bit_idx+1) % 8);
        const int masked_high_byte = ((int)bytes[high_byte_idx]) % high_byte_mask;
        retval = (retval << 8) + masked_high_byte;
    }

    return retval;
}

// Converts a textual MAC address into a long value.
unsigned char* mac_str_to_mac(const char* mac_str)
{
    unsigned char* mac = malloc(6);

    int filled_fields =
        sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

    if (filled_fields < 6)
    {
        fprintf(stderr, "Could not parse %s\n", mac_str);
        exit(1);
    }

    return mac;
}

long mac_to_long(const unsigned char*const mac)
{
    return ((long)mac[0] << (8*5)) + ((long)mac[1] << (8*4)) + ((long)mac[2] << (8*3)) + ((long)mac[3] << (8*2)) + ((long)mac[4] << (8*1)) +  ((long)mac[5] << (8*0));
}

void dbg_print_hex(int verbosity_level, int length, const char*const data)
{
    for (int i = 0; i < length; i++)
    {
        dbg_printf(verbosity_level, "%02X ", (unsigned char)data[i]);
    }
    dbg_printf(verbosity_level, "\n");
}
