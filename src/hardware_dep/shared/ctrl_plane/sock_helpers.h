#ifndef __SOCK_HELPERS_H__
#define __SOCK_HELPERS_H__ 1

int read_p4_msg(int sock, char* buffer, int length);
int write_p4_msg(int sock, char* buffer, int length);

#endif
