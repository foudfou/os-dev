/**
 * Block-level I/O general request layer.
 */
#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

/** All block requests are of size 1024 bytes. */
#define BLOCK_SIZE 1024

// FIXME not sure in which header these should go
#define MAXOPBLOCKS  10  // max # of blocks any FS op writes
#define LOGSIZE      (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF         (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE       1000  // size of file system in blocks

#define BLOCK_VALID 1<<1  // buffer has been read from disk
#define BLOCK_DIRTY 1<<2  // buffer needs to be written to disk

struct block_req {
    int16_t               flags;
    uint16_t              dev;
    // FIXME use /home/foudil/src/c/ptp/src/utils/list.h
    struct block_request *next; /** Next in device queue. */
    uint32_t              block_no; /** Block index on disk. */
    uint8_t               data[BLOCK_SIZE];
};

#endif
