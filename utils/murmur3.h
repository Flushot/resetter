#ifndef __MURMUR3_H__
#define __MURMUR3_H__
#include <stdlib.h>

uint32_t murmur3(const uint8_t* key, size_t len, uint32_t seed);

#endif
