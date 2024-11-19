/*
  __  __  ____ _______ ____  _____  _      _____ _   _ ______
 |  \/  |/ __ \__   __/ __ \|  __ \| |    |_   _| \ | |  ____|
 | \  / | |  | | | | | |  | | |__) | |      | | |  \| | |__
 | |\/| | |  | | | | | |  | |  _  /| |      | | | . ` |  __|
 | |  | | |__| | | | | |__| | | \ \| |____ _| |_| |\  | |____
 |_|  |_|\____/  |_|  \____/|_|  \_\______|_____|_| \_|______|

*/

#include "UDP_Codes.h"
#include "stdio.h"
#include "string.h"

uint8_t *parse_INT_To_STR(char *data, uint8_t size, char *output)
{
    uint64_t int_Data;
    int_Data = strtoull(data, NULL, 10);
    char *str_Data = (char *)malloc(size);
    /* char *t */ *output = str_Data;

    //////printf("\n%llu\n", int_Data);
    for (int i = size - 1; i >= 0; i--)
    {
        if (i >= sizeof(int_Data))
        {
            *output = 0;
        }
        else
        {
            *output = int_Data >> (8 * i);
        }

        //////printf("%X - %X\n", (uint8_t)*output, (uint8_t)(int_Data >> (8 * i)));
        output++;
    }
      
    //////printf("\n%s\n", str_Data);
    return 1;
}