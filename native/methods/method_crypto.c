#include "methods.h"

/* Cryptographic erase methods: CryptoErase, EncryptionAware */

unsigned char get_crypto_pattern(const char *method)
{
    if (!method)
    {
        return 0x00;
    }

    /* CryptoErase and EncryptionAware use crypto-safe pattern 0xA5 */
    return 0xA5;
}
