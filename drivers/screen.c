#include "kernel/low_level.h"
#include "lib/debug.h"
#include "lib/string.h"

#include "drivers/screen.h"


/**
 * Default to black background + light grey foreground.
 * Foreground color can be customized with '*_color' functions.
 */
const enum vga_color TERMINAL_DEFAULT_COLOR_BG = VGA_COLOR_BLUE;
const enum vga_color TERMINAL_DEFAULT_COLOR_FG = VGA_COLOR_LIGHT_GREY;

static const char WHITE_ON_BLACK = vga_attr(TERMINAL_DEFAULT_COLOR_BG,
                                            TERMINAL_DEFAULT_COLOR_FG);

static int get_screen_offset(int col, int row) { return (row * MAX_COLS + col) * 2; }

static int get_cursor() {
  // The device uses its control register as an index
  // to select its internal registers, of which we are
  // interested in:
  // reg 14: which is the high byte of the cursor's offset
  // reg 15: which is the low byte of the cursor's offset
  // Once the internal register has been selected, we may read or
  // write a byte on the data register.
  outb(REG_SCREEN_CTRL, 14);
  int offset = inb(REG_SCREEN_DATA) << 8;
  outb(REG_SCREEN_CTRL, 15);
  offset += inb(REG_SCREEN_DATA);
  // Since the cursor offset reported by the VGA hardware is the
  // number of characters, we multiply by two to convert it to
  // a character cell offset.
  return offset * 2;
}

static void set_cursor(int offset) {
  offset /= 2; // Convert from cell offset to char offset.
               // This is similar to get_cursor, only now we write
               // bytes to those internal device registers.
  outb(REG_SCREEN_CTRL, 14);
  outb(REG_SCREEN_DATA, (offset & 0xff00) >> 8);
  outb(REG_SCREEN_CTRL, 15);
  outb(REG_SCREEN_DATA, offset & 0xff);
}

/* Advance the text cursor, scrolling the video buffer if necessary. */
static int handle_scrolling(int cursor_offset) {

  // If the cursor is within the screen, return it unmodified.
  if (cursor_offset < MAX_ROWS * MAX_COLS * 2) {
    return cursor_offset;
  }

  /* Shuffle the rows back one. */
  int i;
  for (i = 1; i < MAX_ROWS; i++) {
    memcpy((void*)(VIDEO_ADDRESS + get_screen_offset(0, i - 1)),
           (const void*)(VIDEO_ADDRESS + get_screen_offset(0, i)),
           MAX_COLS * 2);
  }

  /* Blank the last line by setting all bytes to 0 */
  char* last_line = (char*)(VIDEO_ADDRESS + get_screen_offset(0, MAX_ROWS - 1));
  for (i = 0; i < MAX_COLS * 2; i++) {
    last_line[i] = 0;
  }

  // Move the offset back one row, such that it is now on the last
  // row, rather than off the edge of the screen.
  cursor_offset -= 2 * MAX_COLS;

  // Return the updated cursor position.
  return cursor_offset;
}

/* Print a char on the screen at col, row, or at cursor position */
static void print_char(const char character, int col, int row, char attribute_byte) {
  /* Create a byte (char) pointer to the start of video memory */
  unsigned char volatile *vidmem = (unsigned char*) VIDEO_ADDRESS;

  /* If attribute byte is zero, assume the default style. */
  if (!attribute_byte) {
    attribute_byte = WHITE_ON_BLACK;
  }

  /* Get the video memory offset for the screen location */
  int offset;
  /* If col and row are non - negative, use them for offset. */
  if (col >= 0 && row >= 0) {
    offset = get_screen_offset(col, row);
  /* Otherwise, use the current cursor position. */
  } else {
    offset = get_cursor();
  }

  // If we see a newline character, set offset to the end of
  // current row, so it will be advanced to the first col
  // of the next row.
  if (character == '\n') {
    int rows = offset / (2 * MAX_COLS);
    offset = get_screen_offset(79, rows);
  // Otherwise, write the character and its attribute byte to
  // video memory at our calculated offset.
  } else {
    vidmem[offset] = character;
    vidmem[offset + 1] = attribute_byte;
  }

  // Update the offset to the next character cell, which is
  // two bytes ahead of the current cell.
  offset += 2;
  // Make scrolling adjustment, for when we reach the bottom
  // of the screen.
  offset = handle_scrolling(offset);
  // Update the cursor position on the screen device.
  set_cursor(offset);
}

static void print_at(const char* message, int col, int row) {
  // Update the cursor if col and row not negative.
  if (col >= 0 && row >= 0) {
    set_cursor(get_screen_offset(col, row));
  }
  // Loop through each char of the message and print it.
  int i = 0;
  while (message[i] != 0) {
    print_char(message[i++], col, row, WHITE_ON_BLACK);
  }
}

void print(const char* message) { print_at(message, -1, -1); }


void clear_screen() {
  int row = 0;
  int col = 0;
  /* Loop through video memory and write blank characters. */
  for (row = 0; row < MAX_ROWS; row++) {
    for (col = 0; col < MAX_COLS; col++) {
      print_char(' ', col, row, WHITE_ON_BLACK);
    }
  }
  // Move the cursor back to the top left.
  set_cursor(get_screen_offset(0, 0));
}



void print_color(const char *data, size_t size, enum vga_color fg) {
    for (size_t i = 0; i < size; ++i)
        print_char(data[i], -1, -1, vga_attr(TERMINAL_DEFAULT_COLOR_BG, fg));
}


// ----------- Lifted from xv6 -----------------

void
consputc(int c)
{
    print_char(c, -1, -1, vga_attr(TERMINAL_DEFAULT_COLOR_BG, TERMINAL_DEFAULT_COLOR_FG));
}

static char digits[] = "0123456789abcdef";

static void
printint(int xx, int base, int sign)
{
  char buf[16];
  int i;
  unsigned int x;

  if(sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do{
    buf[i++] = digits[x % base];
  }while((x /= base) != 0);

  if(sign)
    buf[i++] = '-';

  while(--i >= 0)
    consputc(buf[i]);
}

static void
printptr(uint64_t x)
{
  int i;
  consputc('0');
  consputc('x');
  for (i = 0; i < (sizeof(uint64_t) * 2); i++, x <<= 4)
    consputc(digits[x >> (sizeof(uint64_t) * 8 - 4)]);
}

// Print to the console. only understands %d, %x, %p, %s.
void
cprintf(char *fmt, ...)
{
  int i, c;
  unsigned int *argp;
  char *s;

  // TODO acquire lock

  if (fmt == 0)
    panic("null fmt");

  // The beauty is that authors don't use va_list (stdarg.h) but just iterate
  // on the function's arguments on the stack.
  argp = (unsigned int*)(void*)(&fmt + 1); // va_start
  for(i = 0; (c = fmt[i] & 0xff) != 0; i++){
    if(c != '%'){
      consputc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if(c == 0)
      break;
    switch(c){
    case 'd':
      printint(*argp++, 10, 1); // va_arg
      break;
    case 'x':
    case 'p':
      printint(*argp++, 16, 0);
      break;
    case 'l': // shortcut for %lx
      printptr(*(uint64_t*)argp);
      argp += sizeof(uint64_t) / sizeof(argp);
      break;
    case 's':
      if((s = (char*)*argp++) == 0)
        s = "(null)";
      for(; *s; s++)
        consputc(*s);
      break;
    case '%':
      consputc('%');
      break;
    default:
      // Print unknown % sequence to draw attention.
      consputc('%');
      consputc(c);
      break;
    }
  }

  // TODO release lock
}
