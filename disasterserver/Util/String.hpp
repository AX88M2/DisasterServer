#ifndef DISASTERSERVER_STRING_HPP
#define DISASTERSERVER_STRING_HPP

#include <cstdint>

typedef struct
{
    char		value[250];
    uint16_t	len;
} String;

#endif //DISASTERSERVER_STRING_HPP
