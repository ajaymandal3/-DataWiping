#include "methods.h"

/* Storage-specific methods: AtaSecureErase, FlashOverwrite, SecureFormat */

unsigned char get_storage_pattern(const char *method, unsigned int passIndex)
{
    if (!method)
    {
        return 0x00;
    }

    /* AtaSecureErase: SSD optimized (crypto-safe 0xA5) */
    if (method[0] == 'A' || method[0] == 'a')
    {
        return 0xA5;
    }

    /* FlashOverwrite: USB/SD cards (2-pass: 0x00, 0xFF) */
    if (method[0] == 'F' || method[0] == 'f')
    {
        static const unsigned char p2[2] = {0x00, 0xFF};
        return p2[passIndex % 2U];
    }

    /* SecureFormat: Zero + format (single pass zeros) */
    if (method[0] == 'S' || method[0] == 's')
    {
        return 0x00;
    }

    return 0x00;
}
