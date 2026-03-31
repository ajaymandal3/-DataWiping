#ifndef SECUREWIPE_METHODS_H
#define SECUREWIPE_METHODS_H

#include <stddef.h>

/* Method identifiers for pattern resolution */
typedef enum
{
    METHOD_SINGLE_PASS,
    METHOD_THREE_PASS,
    METHOD_SEVEN_PASS,
    METHOD_CRYPTO_ERASE,
    METHOD_ATA_SECURE_ERASE,
    METHOD_FLASH_OVERWRITE,
    METHOD_ENCRYPTION_AWARE,
    METHOD_RAID_SERVER_WIPE,
    METHOD_SECURE_FORMAT
} WipeMethodType;

/* Get pattern byte for a method and pass index */
unsigned char get_wipe_pattern(WipeMethodType method, unsigned int passIndex);

/* Get method type from string key */
WipeMethodType method_from_key(const char *key);

/* Get total passes for a method */
unsigned int get_method_passes(WipeMethodType method);

#endif /* SECUREWIPE_METHODS_H */
