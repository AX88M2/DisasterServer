#include <UTF8.h>

utf8_char utf8_tolower(utf8_char c)
{
    if(c >= 0x0410 && c <= 0x042F)
        return c + 0x20;
    else if(c <= 0x7F)
        return tolower((int)c);
    else
        return c;
}

utf8_char utf8_get(const char* str, int index)
{
    utf8_char result = 0;
    int byte_index = 0;
    int char_index = 0;

    while (str[byte_index] != '\0' && char_index <= index) {
        const uint8_t c = str[byte_index];
        if (c <= 0x7F) 
        {
            if (char_index == index)
                result = c;

            byte_index += 1;
        } 
        else if (c >= 0xC0 && c <= 0xDF) 
        {
            if (char_index == index)
                result = ((str[byte_index] & 0x1F) << 6) | (str[byte_index + 1] & 0x3F);

            byte_index += 2;
        } 
        else if (c >= 0xE0 && c <= 0xEF) 
        {
            if (char_index == index)
                result = ((str[byte_index] & 0x0F) << 12) | ((str[byte_index + 1] & 0x3F) << 6) | (str[byte_index + 2] & 0x3F);

            byte_index += 3;
        } 
        else if (c >= 0xF0 && c <= 0xF7) 
        {
            if (char_index == index) 
                result = ((str[byte_index] & 0x07) << 18) | ((str[byte_index + 1] & 0x3F) << 12) | ((str[byte_index + 2] & 0x3F) << 6) | (str[byte_index + 3] & 0x3F);

            byte_index += 4;
        }

        char_index++;
    }

    return result;
}

size_t utf8_strlen(const char* str)
{
    size_t len = 0;
    while (*str != '\0') 
    {
        const uint8_t c = (uint8_t)*str;

        if (c < 128)
            str += 1;
        else if (c < 224)
            str += 2;
        else if (c < 240)
            str += 3;
        else
            str += 4;

        len++;
    }

    return len;
}