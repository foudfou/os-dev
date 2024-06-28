/**
 * Parallel ATA (IDE) hard disk driver.
 * Assumes only port I/O (PIO) mode without DMA for now.
 */
#ifndef IDE_H
#define IDE_H


#include <stdint.h>
#include <stdbool.h>

#include "fs/block.h"

#define IDE_SECTOR_SIZE 512
#define SECTORS_PER_BLOCK (BLOCK_SIZE/IDE_SECTOR_SIZE)

// Need to wait at least 400ns before sending ATA commands. IO port read takes
// at least 30ns.
#define IDE_WAIT_CYCLES 15

/**
 * Default I/O ports that are mapped to device registers of the IDE disk
 * on the primary bus (with I/O base = 0x1F0).
 * See https://wiki.osdev.org/ATA_PIO_Mode#Registers.
 */
#define IDE_IO_BASE      0x1F0
#define IDE_DATA         (IDE_IO_BASE + 0)
#define IDE_ERROR        (IDE_IO_BASE + 1)
#define IDE_FEATURES     (IDE_IO_BASE + 1)
#define IDE_SECTORS      (IDE_IO_BASE + 2)
#define IDE_LBA_LO       (IDE_IO_BASE + 3)
#define IDE_LBA_MI       (IDE_IO_BASE + 4)
#define IDE_LBA_HI       (IDE_IO_BASE + 5)
#define IDE_SELECT       (IDE_IO_BASE + 6)
#define IDE_STATUS       (IDE_IO_BASE + 7)
#define IDE_COMMAND      (IDE_IO_BASE + 7)

#define IDE_CTRL_BASE    0x3F6
#define IDE_ALT_STATUS   (IDE_CTRL_BASE + 0)
#define IDE_CONTROL      (IDE_CTRL_BASE + 0)
#define IDE_DRIVE_ADDR   (IDE_CTRL_BASE + 1)

#define IDE_STATUS_ERR (1 << 0)
#define IDE_STATUS_DRQ (1 << 3)
#define IDE_STATUS_SRV (1 << 4)
#define IDE_STATUS_DF  (1 << 5) // Drive Fault Error
#define IDE_STATUS_RDY (1 << 6)
#define IDE_STATUS_BSY (1 << 7)

#define IDE_CMD_READ  0x20
#define IDE_CMD_WRITE 0x30
#define IDE_CMD_RDMUL 0XC4
#define IDE_CMD_WRMUL 0xC5
#define IDE_CMD_IDFY  0xEC
#define IDE_CMD_FLUSH 0xE7

#define IDE_SEL     0xA0
#define IDE_SEL_DRV 0x10        // drive 0 otherwise
#define IDE_SEL_LBA 0x40        // CHS otherwise

#define ATA_IDENT_DEVICETYPE   0
#define ATA_IDENT_CYLINDERS    1
#define ATA_IDENT_HEADS        3
#define ATA_IDENT_SECTORS      6
#define ATA_IDENT_SERIAL       10
#define ATA_IDENT_MODEL        27
#define ATA_IDENT_CAPABILITIES 49
#define ATA_IDENT_FIELDVALID   53
#define ATA_IDENT_MAX_LBA      60
#define ATA_IDENT_COMMANDSETS  82
#define ATA_IDENT_MAX_LBA_EXT  100

void ide_init();


#endif
