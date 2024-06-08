#ifndef SCREEN_H
#define SCREEN_H

#include <stddef.h>
#include "drivers/vga.h"

#define MAX_ROWS 25
#define MAX_COLS 80

// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5

// FIXME can't we use KEY_BACKSPACE?
#define BACKSPACE 0x100

/**
 * Default to black background + light grey foreground.
 * Foreground color can be customized with '*_color' functions.
 */
extern const enum vga_color TERMINAL_DEFAULT_COLOR_BG;
extern const enum vga_color TERMINAL_DEFAULT_COLOR_FG;

void clear_screen();

void print(const char* message);
void print_color(const char *data, size_t size, enum vga_color fg);

void cprintf(char *fmt, ...);
void panic(char *s);

void consoleinit(void);


#endif /* SCREEN_H */
