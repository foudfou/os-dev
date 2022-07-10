/*
 * Copyright (c) 2019 Foudil Brétel.  All rights reserved.
 *
 * Copied from https://github.com/foudfou/ptp/blob/master/src/utils/bitfield.h
 */
#ifndef BITMAP_H
#define BITMAP_H

/**
 * An efficient bit field.
 *
 * Bit fields are slightly different to byte arrays in that they focus on bits.
 *
 * Directly copied from https://stackoverflow.com/a/24753227/421846.
 */
#include <stdint.h>

typedef uint32_t bitmap;

#define BITMAP_RESERVE_BITS(n) (((n)+0x1f)>>5)
#define BITMAP_DWORD_IDX(x) ((x)>>5)
#define BITMAP_BIT_IDX(x) ((x)&0x1f) // ((1<<DW_INDEX_BITS)-1)
#define BITMAP_GET(array, index) (((array)[BITMAP_DWORD_IDX(index)]>>BITMAP_BIT_IDX(index))&1)
#define BITMAP_SET(array, index, bit)                                 \
    (((bit)&1) ? ((array)[BITMAP_DWORD_IDX(index)] |= UINT32_C(1)<<BITMAP_BIT_IDX(index)) \
     : ((array)[BITMAP_DWORD_IDX(index)] &= ~(UINT32_C(1)<<BITMAP_BIT_IDX(index))) \
     , (void)0                                                          \
        )

uint64_t bitmap_alloc(bitmap *map, uint64_t sz);

#endif /* BITMAP_H */
