#include "lib/debug.h"
#include "lib/string.h"
#include "idt.h"
#include "low_level.h"
#include "spinlock.h"

#include "drivers/ide.h"

#define LBA_SELECT(drive, sector) (IDE_SEL|IDE_SEL_LBA \
    | ((drive&1)<<4) \
    | ((sector>>24)&0x0f))

static struct spinlock ide_lock;
static struct block_req *ide_queue_head;
static struct block_req *ide_queue_tail; /* FIXME not needed */

static uint16_t ide_identify_data[256];

static void ide_interrupt_handler(struct interrupt_state *state) {
    (void) state;   /** Unused. */

    print("__ide_int\n");
}

/**
 * Wait for IDE disk on primary bus to become ready. Returns false on errors
 * or device faults, otherwise true.
 *
 * Some codes simply repeat `inb(IDE_ALT_STATUS)` 4 times.
 */
static bool
ide_wait_ready(void)
{
    uint8_t status;
    int i = IDE_WAIT_CYCLES;
    for (; i > 0; i--) {
        /** Read from alternative status so it won't affect interrupts. */
        status = inb(IDE_ALT_STATUS);
        if ((status & (IDE_STATUS_BSY | IDE_STATUS_RDY)) == IDE_STATUS_RDY) {
            break;
        }
    }

    if (i == 0 || (status & (IDE_STATUS_DF | IDE_STATUS_ERR)) != 0)
        return false;

    return true;
}

bool ide_soft_reset() {
    outb(IDE_CONTROL, 1<<2); // SRST
    // wait
    inb(IDE_ALT_STATUS);
    inb(IDE_ALT_STATUS);
    inb(IDE_ALT_STATUS);
    inb(IDE_ALT_STATUS);
    return ide_wait_ready();
}

static void ide_start(struct block_req *b);

/**
 * Initialize a single IDE disk 0 on the default primary bus. Registers the
 * IDE request interrupt ISR handler.
 */
void ide_init(void) {
    initlock(&ide_lock, "ide_lock");

//    ioapicenable(IRQ_IRQ_IDE1, ncpu - 1);

    isr_register(IDT_IRQ_BASE + IDT_IRQ_IDE1, &ide_interrupt_handler);

    // Our filesystem will be on Device-1.
    outb(IDE_SELECT, LBA_SELECT(1, 0));
    ide_wait_ready();

    outb(IDE_CONTROL, 0);    /** Ensure interrupts on. */

    /**
     * Detect that disk 0 on the primary ATA bus is there and is a PATA (IDE)
     * device. Utilizes the IDENTIFY command.
     * https://wiki.osdev.org/ATA_PIO_Mode#IDENTIFY_command
     */
    outb(IDE_SECTORS, 0);
    outb(IDE_LBA_LO,  0);
    outb(IDE_LBA_MI, 0);
    outb(IDE_LBA_HI,  0);
    outb(IDE_COMMAND, IDE_CMD_IDFY);

    uint8_t status = inb(IDE_ALT_STATUS);
    if (status == 0)
        error("ide_init: drive does not exist on primary bus");
    do {
        status = inb(IDE_ALT_STATUS);
        if (inb(IDE_LBA_MI) != 0 || inb(IDE_LBA_HI) != 0)
            error("ide_init: drive on primary bus is not PATA");
    } while ((status & (IDE_STATUS_BSY)) != 0
             && (status & (IDE_STATUS_DRQ | IDE_STATUS_ERR)) != 0);

    if ((status & (IDE_STATUS_ERR)) != 0)
        error("ide_init: error returned from the IDENTIFY command");

    /** Must be a stream in 32-bit dwords. */
    memset(ide_identify_data, 0, 256 * sizeof(uint16_t));
    insl(IDE_DATA, ide_identify_data, 256 * sizeof(uint16_t) / sizeof(uint32_t));

    uint8_t model[40];   // model in string.
    uint8_t *ptr = (uint8_t*)(ide_identify_data + ATA_IDENT_MODEL);
    for (int i = 0; i < 40; i+=2) {
        model[i] = ptr[i+1];
        model[i+1] = ptr[i];
    }
    model[39] = 0; // Terminate String.
    cprintf("ide_init: found ATA drive model: %s\n", model);

    struct block_req b = {0};
    strncpy(b.data, "FOUDIL WAS HERE", 20);
    b.flags = BLOCK_DIRTY;
    b.dev = 1;
    ide_start(&b);
}

static void
ide_start(struct block_req *b)
{
  if(b == 0)
    panic("ide_start");
  if(b->block_no >= FSSIZE)
    panic("incorrect blockno");
  int32_t sector = b->block_no * SECTORS_PER_BLOCK;
  int32_t read_cmd  = (SECTORS_PER_BLOCK == 1) ? IDE_CMD_READ :  IDE_CMD_RDMUL;
  int32_t write_cmd = (SECTORS_PER_BLOCK == 1) ? IDE_CMD_WRITE : IDE_CMD_WRMUL;

  if (SECTORS_PER_BLOCK > 7) panic("ide_start");

  ide_wait_ready();
  outb(IDE_CONTROL, 0);  // make sure to generate interrupt
  outb(IDE_SECTORS, SECTORS_PER_BLOCK);  // number of sectors
  outb(IDE_LBA_LO, sector & 0xff);
  outb(IDE_LBA_MI, (sector >> 8) & 0xff);
  outb(IDE_LBA_HI, (sector >> 16) & 0xff);
  outb(IDE_SELECT, LBA_SELECT(b->dev, sector));
  if(b->flags & BLOCK_DIRTY){
    outb(IDE_COMMAND, write_cmd);
    /** Must be a stream in 32-bit dwords, can't be in 8-bit bytes. */
    // FIXME bochs ">>PANIC<< IO write(0x01f0): current command is 00h". Not
    // sure why the command (controller->current_command) is not 0xc5 at this
    // point. Works well with IDE_CMD_WRITE.
    outsl(IDE_DATA, b->data, BLOCK_SIZE / sizeof(uint32_t));
    /* outb(IDE_COMMAND, IDE_CMD_FLUSH); */
  } else {
    outb(IDE_COMMAND, read_cmd);
  }
}
