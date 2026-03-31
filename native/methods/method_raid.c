#include "methods.h"

/* Professional RAID/Server wipe method: RaidServerWipe */

unsigned char get_raid_pattern(unsigned int passIndex)
{
    /* RaidServerWipe: 3-pass professional (0x00, 0xFF, 0xAA) */
    static const unsigned char praid[3] = {0x00, 0xFF, 0xAA};
    return praid[passIndex % 3U];
}
