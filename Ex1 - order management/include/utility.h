#ifndef UTILITY_H
#define UTILITY_H
#include <stdio.h>
#include <stddef.h>

// Hàm tiện ích
char* get_current_time(char* buffer, int buffersize);
int get_next_valid_line(char **line, size_t *len, FILE *fp);

#endif

