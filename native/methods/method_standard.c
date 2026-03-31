#include "methods.h"

/* Standard overwriting methods: SinglePass, ThreePass, SevenPass */

unsigned char get_standard_pattern(const char *method, unsigned int passIndex)
{
    if (!method)
    {
        return 0x00;
    }

    /* SinglePass: zeros only */
    if (method[0] == 'S' || method[0] == 's')
    {
        return 0x00;
    }

    /* ThreePass: DoD standard (0x00, 0xFF, 0xAA) */
    if (method[0] == 'T' || method[0] == 't')
    {
        static const unsigned char p3[3] = {0x00, 0xFF, 0xAA};
        return p3[passIndex % 3U];
    }

    /* SevenPass: Gutmann-lite */
    if (method[0] == 'S' && (method[1] == 'e' || method[1] == 'E'))
    {
        static const unsigned char p7[7] = {0xF6, 0x00, 0xFF, 0xAA, 0x55, 0x92, 0x49};
        return p7[passIndex % 7U];
    }

    return 0x00;
}
