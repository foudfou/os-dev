#ifndef SCREEN_H
#define SCREEN_H

#define VIDEO_ADDRESS 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80
// Attribute for our default colour scheme
#define WHITE_ON_BLACK 0x0f

// Screen device I/O ports
#define REG_SCREEN_CTRL 0x3D4
#define REG_SCREEN_DATA 0x3D5


#define printi(msg, i) print(msg); print_hex(i); print("\n");

void clear_screen();
void print_char(const char character, int col, int row, char attribute_byte);
void print_at(const char* message, int col, int row);
void print_hex(unsigned int number);
void print_dump(const unsigned char* pointer, unsigned int length);
void print(const char* message);


#endif /* SCREEN_H */
