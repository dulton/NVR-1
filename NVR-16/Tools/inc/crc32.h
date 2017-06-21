#ifndef CRC32_H
#define CRC32_H

#include "common_basetypes.h"
typedef u32 __u32;
typedef u8 __u8;

/* Return a 32-bit CRC of the contents of the buffer. */
__u32 crc32(__u32 val, const void *ss, int len);

#endif
