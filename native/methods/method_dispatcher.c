#include "methods.h"
#include <string.h>

/* Forward declarations for individual method pattern getters */
extern unsigned char get_standard_pattern(const char *method, unsigned int passIndex);
extern unsigned char get_crypto_pattern(const char *method);
extern unsigned char get_storage_pattern(const char *method, unsigned int passIndex);
extern unsigned char get_raid_pattern(unsigned int passIndex);

/* Get method type from string key */
WipeMethodType method_from_key(const char *key)
{
    if (!key)
        return METHOD_SINGLE_PASS;

    if (strcmp(key, "SinglePass") == 0 || strcmp(key, "singlepass") == 0)
    {
        return METHOD_SINGLE_PASS;
    }
    if (strcmp(key, "ThreePass") == 0 || strcmp(key, "threepass") == 0)
    {
        return METHOD_THREE_PASS;
    }
    if (strcmp(key, "SevenPass") == 0 || strcmp(key, "sevenpass") == 0)
    {
        return METHOD_SEVEN_PASS;
    }
    if (strcmp(key, "CryptoErase") == 0 || strcmp(key, "cryptoerase") == 0)
    {
        return METHOD_CRYPTO_ERASE;
    }
    if (strcmp(key, "AtaSecureErase") == 0 || strcmp(key, "atasecureerase") == 0)
    {
        return METHOD_ATA_SECURE_ERASE;
    }
    if (strcmp(key, "FlashOverwrite") == 0 || strcmp(key, "flashoverwrite") == 0)
    {
        return METHOD_FLASH_OVERWRITE;
    }
    if (strcmp(key, "EncryptionAware") == 0 || strcmp(key, "encryptionaware") == 0)
    {
        return METHOD_ENCRYPTION_AWARE;
    }
    if (strcmp(key, "RaidServerWipe") == 0 || strcmp(key, "raidserverwipe") == 0)
    {
        return METHOD_RAID_SERVER_WIPE;
    }
    if (strcmp(key, "SecureFormat") == 0 || strcmp(key, "secureformat") == 0)
    {
        return METHOD_SECURE_FORMAT;
    }

    return METHOD_SINGLE_PASS;
}

/* Get total passes for a method */
unsigned int get_method_passes(WipeMethodType method)
{
    switch (method)
    {
    case METHOD_SINGLE_PASS:
    case METHOD_CRYPTO_ERASE:
    case METHOD_ATA_SECURE_ERASE:
    case METHOD_ENCRYPTION_AWARE:
    case METHOD_SECURE_FORMAT:
        return 1U;

    case METHOD_FLASH_OVERWRITE:
        return 2U;

    case METHOD_THREE_PASS:
    case METHOD_RAID_SERVER_WIPE:
        return 3U;

    case METHOD_SEVEN_PASS:
        return 7U;

    default:
        return 1U;
    }
}

/* Get pattern byte for a method and pass index */
unsigned char get_wipe_pattern(WipeMethodType method, unsigned int passIndex)
{
    switch (method)
    {
    case METHOD_SINGLE_PASS:
    case METHOD_THREE_PASS:
    case METHOD_SEVEN_PASS:
        return get_standard_pattern(
            method == METHOD_SINGLE_PASS ? "SinglePass" : method == METHOD_THREE_PASS ? "ThreePass"
                                                                                      : "SevenPass",
            passIndex);

    case METHOD_CRYPTO_ERASE:
    case METHOD_ENCRYPTION_AWARE:
        return get_crypto_pattern("CryptoErase");

    case METHOD_ATA_SECURE_ERASE:
        return get_crypto_pattern("AtaSecureErase");

    case METHOD_FLASH_OVERWRITE:
        return get_storage_pattern("FlashOverwrite", passIndex);

    case METHOD_SECURE_FORMAT:
        return get_storage_pattern("SecureFormat", passIndex);

    case METHOD_RAID_SERVER_WIPE:
        return get_raid_pattern(passIndex);

    default:
        return 0x00;
    }
}
