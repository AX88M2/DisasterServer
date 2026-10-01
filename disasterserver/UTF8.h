#ifndef UTF8_H
#define UTF8_H
#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
typedef int utf8_char;

utf8_char utf8_tolower(utf8_char c);
utf8_char utf8_get(const char* str, int index);
size_t utf8_strlen(const char* str);

#endif