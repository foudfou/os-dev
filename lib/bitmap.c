#include <stddef.h>
#include "lib/bitmap.h"

/**
 * Traverses bitmap to find free slot, mark it as used. Returns the slot index
 * of the allocated slot, or `num_slots` if there is no free slot.
 */
uint64_t bitmap_alloc(bitmap *map, uint64_t sz) {
    // TODO acquire lock

    for (uint64_t i = 0; i < sz; ++i) {
        if (BITMAP_GET(map, i) != 0)
            continue;

        BITMAP_SET(map, i, 1);

        // TODO release lock
        return i;
    }

    // TODO release lock
    return sz;
}
