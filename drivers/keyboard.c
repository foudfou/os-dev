#include <string.h>
#include "drivers/screen.h"
#include "kernel/idt.h"
#include "kernel/low_level.h"
#include "kernel/pic.h"

#include <stdbool.h>

#define IO_PORT_KBD_DATA 0x60

/** A partial set of special keys on US QWERTY keyboard. */
enum keyboard_meta_key {
    KEY_NULL,   // Dummy placeholder for empty key

    KEY_ESC,    // Escape
    KEY_BACK,   // Backspace
    KEY_TAB,    // Tab
    KEY_ENTER,  // Enter
    KEY_CTRL,   // Both ctrls
    KEY_SHIFT,  // Both shifts
    KEY_ALT,    // Both alts
    KEY_CAPS,   // Capslock

    KEY_HOME,   // Home
    KEY_END,    // End
    KEY_UP,     // Cursor up
    KEY_DOWN,   // Cursor down
    KEY_LEFT,   // Cursor left
    KEY_RIGHT,  // Cursor right
    KEY_PGUP,   // Page up
    KEY_PGDN,   // Page down
    KEY_INS,    // Insert
    KEY_DEL,    // Delete
};

/** Holds info for a keyboard key. */
struct keyboard_key_info {
    enum keyboard_meta_key meta;  /** Special meta key. */
    char codel; /** ASCII byte code - lower case. */
    char codeu; /** ASCII byte code - upper case. */
};

/** Struct for a keyboard event. */
struct keyboard_key_event {
    bool press;     /** False if is a release event. */
    bool ascii;     /** True if is ASCII character, otherwise special. */
    struct keyboard_key_info info;
};

/**
 * Hardcode scancode -> key event mapping.
 *
 * Check out https://wiki.osdev.org/Keyboard#Scan_Code_Set_1
 * for a complete list of mappings.
 *
 * We will only code a partial set of mappings - only those most
 * useful events.
 */
#define NO_KEY { .press = false, .ascii = false, .info = { .meta = KEY_NULL } }

static const struct keyboard_key_event scancode_event_map[0xE0] = {
    NO_KEY,                                                                      // 0x00
    { .press = true , .ascii = false, .info = { .meta = KEY_ESC   } },           // 0x01
    { .press = true , .ascii = true , .info = { .codel = '1' , .codeu = '!' } }, // 0x02
    { .press = true , .ascii = true , .info = { .codel = '2' , .codeu = '@' } }, // 0x03
    { .press = true , .ascii = true , .info = { .codel = '3' , .codeu = '#' } }, // 0x04
    { .press = true , .ascii = true , .info = { .codel = '4' , .codeu = '$' } }, // 0x05
    { .press = true , .ascii = true , .info = { .codel = '5' , .codeu = '%' } }, // 0x06
    { .press = true , .ascii = true , .info = { .codel = '6' , .codeu = '^' } }, // 0x07
    { .press = true , .ascii = true , .info = { .codel = '7' , .codeu = '&' } }, // 0x08
    { .press = true , .ascii = true , .info = { .codel = '8' , .codeu = '*' } }, // 0x09
    { .press = true , .ascii = true , .info = { .codel = '9' , .codeu = '(' } }, // 0x0A
    { .press = true , .ascii = true , .info = { .codel = '0' , .codeu = ')' } }, // 0x0B
    { .press = true , .ascii = true , .info = { .codel = '-' , .codeu = '_' } }, // 0x0C
    { .press = true , .ascii = true , .info = { .codel = '=' , .codeu = '+' } }, // 0x0D
    { .press = true , .ascii = false, .info = { .meta = KEY_BACK  } },           // 0x0E
    { .press = true , .ascii = false, .info = { .meta = KEY_TAB   } },           // 0x0F
    { .press = true , .ascii = true , .info = { .codel = 'q' , .codeu = 'Q' } }, // 0x10
    { .press = true , .ascii = true , .info = { .codel = 'w' , .codeu = 'W' } }, // 0x11
    { .press = true , .ascii = true , .info = { .codel = 'e' , .codeu = 'E' } }, // 0x12
    { .press = true , .ascii = true , .info = { .codel = 'r' , .codeu = 'R' } }, // 0x13
    { .press = true , .ascii = true , .info = { .codel = 't' , .codeu = 'T' } }, // 0x14
    { .press = true , .ascii = true , .info = { .codel = 'y' , .codeu = 'Y' } }, // 0x15
    { .press = true , .ascii = true , .info = { .codel = 'u' , .codeu = 'U' } }, // 0x16
    { .press = true , .ascii = true , .info = { .codel = 'i' , .codeu = 'I' } }, // 0x17
    { .press = true , .ascii = true , .info = { .codel = 'o' , .codeu = 'O' } }, // 0x18
    { .press = true , .ascii = true , .info = { .codel = 'p' , .codeu = 'P' } }, // 0x19
    { .press = true , .ascii = true , .info = { .codel = '[' , .codeu = '{' } }, // 0x1A
    { .press = true , .ascii = true , .info = { .codel = ']' , .codeu = '}' } }, // 0x1B
    { .press = true , .ascii = false, .info = { .meta = KEY_ENTER } },           // 0x1C
    { .press = true , .ascii = false, .info = { .meta = KEY_CTRL  } },           // 0x1D
    { .press = true , .ascii = true , .info = { .codel = 'a' , .codeu = 'A' } }, // 0x1E
    { .press = true , .ascii = true , .info = { .codel = 's' , .codeu = 'S' } }, // 0x1F
    { .press = true , .ascii = true , .info = { .codel = 'd' , .codeu = 'D' } }, // 0x20
    { .press = true , .ascii = true , .info = { .codel = 'f' , .codeu = 'F' } }, // 0x21
    { .press = true , .ascii = true , .info = { .codel = 'g' , .codeu = 'G' } }, // 0x22
    { .press = true , .ascii = true , .info = { .codel = 'h' , .codeu = 'H' } }, // 0x23
    { .press = true , .ascii = true , .info = { .codel = 'j' , .codeu = 'J' } }, // 0x24
    { .press = true , .ascii = true , .info = { .codel = 'k' , .codeu = 'K' } }, // 0x25
    { .press = true , .ascii = true , .info = { .codel = 'l' , .codeu = 'L' } }, // 0x26
    { .press = true , .ascii = true , .info = { .codel = ';' , .codeu = ':' } }, // 0x27
    { .press = true , .ascii = true , .info = { .codel = '\'', .codeu = '"' } }, // 0x28
    { .press = true , .ascii = true , .info = { .codel = '`' , .codeu = '~' } }, // 0x29
    { .press = true , .ascii = false, .info = { .meta = KEY_SHIFT } },           // 0x2A
    { .press = true , .ascii = true , .info = { .codel = '\\', .codeu = '|' } }, // 0x2B
    { .press = true , .ascii = true , .info = { .codel = 'z' , .codeu = 'Z' } }, // 0x2C
    { .press = true , .ascii = true , .info = { .codel = 'x' , .codeu = 'X' } }, // 0x2D
    { .press = true , .ascii = true , .info = { .codel = 'c' , .codeu = 'C' } }, // 0x2E
    { .press = true , .ascii = true , .info = { .codel = 'v' , .codeu = 'V' } }, // 0x2F
    { .press = true , .ascii = true , .info = { .codel = 'b' , .codeu = 'B' } }, // 0x30
    { .press = true , .ascii = true , .info = { .codel = 'n' , .codeu = 'N' } }, // 0x31
    { .press = true , .ascii = true , .info = { .codel = 'm' , .codeu = 'M' } }, // 0x32
    { .press = true , .ascii = true , .info = { .codel = ',' , .codeu = '<' } }, // 0x33
    { .press = true , .ascii = true , .info = { .codel = '.' , .codeu = '>' } }, // 0x34
    { .press = true , .ascii = true , .info = { .codel = '/' , .codeu = '?' } }, // 0x35
    { .press = true , .ascii = false, .info = { .meta = KEY_SHIFT } },           // 0x36
    NO_KEY,                                                                      // 0x37
    { .press = true , .ascii = false, .info = { .meta = KEY_ALT   } },           // 0x38
    { .press = true , .ascii = true , .info = { .codel = ' ' , .codeu = ' ' } }, // 0x39
    { .press = true , .ascii = false, .info = { .meta = KEY_CAPS  } },           // 0x3A
    NO_KEY,                                                                      // 0x3B
    NO_KEY,                                                                      // 0x3C
    NO_KEY,                                                                      // 0x3D
    NO_KEY,                                                                      // 0x3E
    NO_KEY,                                                                      // 0x3F
    NO_KEY,                                                                      // 0x40
    NO_KEY,                                                                      // 0x41
    NO_KEY,                                                                      // 0x42
    NO_KEY,                                                                      // 0x43
    NO_KEY,                                                                      // 0x44
    NO_KEY,                                                                      // 0x45
    NO_KEY,                                                                      // 0x46
    NO_KEY,                                                                      // 0x47
    NO_KEY,                                                                      // 0x48
    NO_KEY,                                                                      // 0x49
    NO_KEY,                                                                      // 0x4A
    NO_KEY,                                                                      // 0x4B
    NO_KEY,                                                                      // 0x4C
    NO_KEY,                                                                      // 0x4D
    NO_KEY,                                                                      // 0x4E
    NO_KEY,                                                                      // 0x4F
    NO_KEY,                                                                      // 0x50
    NO_KEY,                                                                      // 0x51
    NO_KEY,                                                                      // 0x52
    NO_KEY,                                                                      // 0x53
    NO_KEY,                                                                      // 0x54
    NO_KEY,                                                                      // 0x55
    NO_KEY,                                                                      // 0x56
    NO_KEY,                                                                      // 0x57
    NO_KEY,                                                                      // 0x58
    NO_KEY,                                                                      // 0x59
    NO_KEY,                                                                      // 0x5A
    NO_KEY,                                                                      // 0x5B
    NO_KEY,                                                                      // 0x5C
    NO_KEY,                                                                      // 0x5D
    NO_KEY,                                                                      // 0x5E
    NO_KEY,                                                                      // 0x5F
    NO_KEY,                                                                      // 0x60
    NO_KEY,                                                                      // 0x61
    NO_KEY,                                                                      // 0x62
    NO_KEY,                                                                      // 0x63
    NO_KEY,                                                                      // 0x64
    NO_KEY,                                                                      // 0x65
    NO_KEY,                                                                      // 0x66
    NO_KEY,                                                                      // 0x67
    NO_KEY,                                                                      // 0x68
    NO_KEY,                                                                      // 0x69
    NO_KEY,                                                                      // 0x6A
    NO_KEY,                                                                      // 0x6B
    NO_KEY,                                                                      // 0x6C
    NO_KEY,                                                                      // 0x6D
    NO_KEY,                                                                      // 0x6E
    NO_KEY,                                                                      // 0x6F
    NO_KEY,                                                                      // 0x70
    NO_KEY,                                                                      // 0x71
    NO_KEY,                                                                      // 0x72
    NO_KEY,                                                                      // 0x73
    NO_KEY,                                                                      // 0x74
    NO_KEY,                                                                      // 0x75
    NO_KEY,                                                                      // 0x76
    NO_KEY,                                                                      // 0x77
    NO_KEY,                                                                      // 0x78
    NO_KEY,                                                                      // 0x79
    NO_KEY,                                                                      // 0x7A
    NO_KEY,                                                                      // 0x7B
    NO_KEY,                                                                      // 0x7C
    NO_KEY,                                                                      // 0x7D
    NO_KEY,                                                                      // 0x7E
    NO_KEY,                                                                      // 0x7F
    NO_KEY,                                                                      // 0x80
    { .press = false, .ascii = false, .info = { .meta = KEY_ESC   } },           // 0x81
    { .press = false, .ascii = true , .info = { .codel = '1' , .codeu = '!' } }, // 0x82
    { .press = false, .ascii = true , .info = { .codel = '2' , .codeu = '@' } }, // 0x83
    { .press = false, .ascii = true , .info = { .codel = '3' , .codeu = '#' } }, // 0x84
    { .press = false, .ascii = true , .info = { .codel = '4' , .codeu = '$' } }, // 0x85
    { .press = false, .ascii = true , .info = { .codel = '5' , .codeu = '%' } }, // 0x86
    { .press = false, .ascii = true , .info = { .codel = '6' , .codeu = '^' } }, // 0x87
    { .press = false, .ascii = true , .info = { .codel = '7' , .codeu = '&' } }, // 0x88
    { .press = false, .ascii = true , .info = { .codel = '8' , .codeu = '*' } }, // 0x89
    { .press = false, .ascii = true , .info = { .codel = '9' , .codeu = '(' } }, // 0x8A
    { .press = false, .ascii = true , .info = { .codel = '0' , .codeu = ')' } }, // 0x8B
    { .press = false, .ascii = true , .info = { .codel = '-' , .codeu = '_' } }, // 0x8C
    { .press = false, .ascii = true , .info = { .codel = '=' , .codeu = '+' } }, // 0x8D
    { .press = false, .ascii = false, .info = { .meta = KEY_BACK  } },           // 0x8E
    { .press = false, .ascii = false, .info = { .meta = KEY_TAB   } },           // 0x8F
    { .press = false, .ascii = true , .info = { .codel = 'q' , .codeu = 'Q' } }, // 0x90
    { .press = false, .ascii = true , .info = { .codel = 'w' , .codeu = 'W' } }, // 0x91
    { .press = false, .ascii = true , .info = { .codel = 'e' , .codeu = 'E' } }, // 0x92
    { .press = false, .ascii = true , .info = { .codel = 'r' , .codeu = 'R' } }, // 0x93
    { .press = false, .ascii = true , .info = { .codel = 't' , .codeu = 'T' } }, // 0x94
    { .press = false, .ascii = true , .info = { .codel = 'y' , .codeu = 'Y' } }, // 0x95
    { .press = false, .ascii = true , .info = { .codel = 'u' , .codeu = 'U' } }, // 0x96
    { .press = false, .ascii = true , .info = { .codel = 'i' , .codeu = 'I' } }, // 0x97
    { .press = false, .ascii = true , .info = { .codel = 'o' , .codeu = 'O' } }, // 0x98
    { .press = false, .ascii = true , .info = { .codel = 'p' , .codeu = 'P' } }, // 0x99
    { .press = false, .ascii = true , .info = { .codel = '[' , .codeu = '{' } }, // 0x9A
    { .press = false, .ascii = true , .info = { .codel = ']' , .codeu = '}' } }, // 0x9B
    { .press = false, .ascii = false, .info = { .meta = KEY_ENTER } },           // 0x9C
    { .press = false, .ascii = false, .info = { .meta = KEY_CTRL  } },           // 0x9D
    { .press = false, .ascii = true , .info = { .codel = 'a' , .codeu = 'A' } }, // 0x9E
    { .press = false, .ascii = true , .info = { .codel = 's' , .codeu = 'S' } }, // 0x9F
    { .press = false, .ascii = true , .info = { .codel = 'd' , .codeu = 'D' } }, // 0xA0
    { .press = false, .ascii = true , .info = { .codel = 'f' , .codeu = 'F' } }, // 0xA1
    { .press = false, .ascii = true , .info = { .codel = 'g' , .codeu = 'G' } }, // 0xA2
    { .press = false, .ascii = true , .info = { .codel = 'h' , .codeu = 'H' } }, // 0xA3
    { .press = false, .ascii = true , .info = { .codel = 'j' , .codeu = 'J' } }, // 0xA4
    { .press = false, .ascii = true , .info = { .codel = 'k' , .codeu = 'K' } }, // 0xA5
    { .press = false, .ascii = true , .info = { .codel = 'l' , .codeu = 'L' } }, // 0xA6
    { .press = false, .ascii = true , .info = { .codel = ';' , .codeu = ':' } }, // 0xA7
    { .press = false, .ascii = true , .info = { .codel = '\'', .codeu = '"' } }, // 0xA8
    { .press = false, .ascii = true , .info = { .codel = '`' , .codeu = '~' } }, // 0xA9
    { .press = false, .ascii = false, .info = { .meta = KEY_SHIFT } },           // 0xAA
    { .press = false, .ascii = true , .info = { .codel = '\\', .codeu = '|' } }, // 0xAB
    { .press = false, .ascii = true , .info = { .codel = 'z' , .codeu = 'Z' } }, // 0xAC
    { .press = false, .ascii = true , .info = { .codel = 'x' , .codeu = 'X' } }, // 0xAD
    { .press = false, .ascii = true , .info = { .codel = 'c' , .codeu = 'C' } }, // 0xAE
    { .press = false, .ascii = true , .info = { .codel = 'v' , .codeu = 'V' } }, // 0xAF
    { .press = false, .ascii = true , .info = { .codel = 'b' , .codeu = 'B' } }, // 0xB0
    { .press = false, .ascii = true , .info = { .codel = 'n' , .codeu = 'N' } }, // 0xB1
    { .press = false, .ascii = true , .info = { .codel = 'm' , .codeu = 'M' } }, // 0xB2
    { .press = false, .ascii = true , .info = { .codel = ',' , .codeu = '<' } }, // 0xB3
    { .press = false, .ascii = true , .info = { .codel = '.' , .codeu = '>' } }, // 0xB4
    { .press = false, .ascii = true , .info = { .codel = '/' , .codeu = '?' } }, // 0xB5
    { .press = false, .ascii = false, .info = { .meta = KEY_SHIFT } },           // 0xB6
    NO_KEY,                                                                      // 0xB7
    { .press = false, .ascii = false, .info = { .meta = KEY_ALT   } },           // 0xB8
    { .press = false, .ascii = true , .info = { .codel = ' ' , .codeu = ' ' } }, // 0xB9
    { .press = false, .ascii = false, .info = { .meta = KEY_CAPS  } },           // 0xBA
    NO_KEY,                                                                      // 0xBB
    NO_KEY,                                                                      // 0xBC
    NO_KEY,                                                                      // 0xBD
    NO_KEY,                                                                      // 0xBE
    NO_KEY,                                                                      // 0xBF
    NO_KEY,                                                                      // 0xC0
    NO_KEY,                                                                      // 0xC1
    NO_KEY,                                                                      // 0xC2
    NO_KEY,                                                                      // 0xC3
    NO_KEY,                                                                      // 0xC4
    NO_KEY,                                                                      // 0xC5
    NO_KEY,                                                                      // 0xC6
    NO_KEY,                                                                      // 0xC7
    NO_KEY,                                                                      // 0xC8
    NO_KEY,                                                                      // 0xC9
    NO_KEY,                                                                      // 0xCA
    NO_KEY,                                                                      // 0xCB
    NO_KEY,                                                                      // 0xCC
    NO_KEY,                                                                      // 0xCD
    NO_KEY,                                                                      // 0xCE
    NO_KEY,                                                                      // 0xCF
    NO_KEY,                                                                      // 0xD0
    NO_KEY,                                                                      // 0xD1
    NO_KEY,                                                                      // 0xD2
    NO_KEY,                                                                      // 0xD3
    NO_KEY,                                                                      // 0xD4
    NO_KEY,                                                                      // 0xD5
    NO_KEY,                                                                      // 0xD6
    NO_KEY,                                                                      // 0xD7
    NO_KEY,                                                                      // 0xD8
    NO_KEY,                                                                      // 0xD9
    NO_KEY,                                                                      // 0xDA
    NO_KEY,                                                                      // 0xDB
    NO_KEY,                                                                      // 0xDC
    NO_KEY,                                                                      // 0xDD
    NO_KEY,                                                                      // 0xDE
    NO_KEY,                                                                      // 0xDF
};

static const struct keyboard_key_event extendcode_event_map[0xE0] = {
    NO_KEY,                                                            // 0x00
    NO_KEY,                                                            // 0x01
    NO_KEY,                                                            // 0x02
    NO_KEY,                                                            // 0x03
    NO_KEY,                                                            // 0x04
    NO_KEY,                                                            // 0x05
    NO_KEY,                                                            // 0x06
    NO_KEY,                                                            // 0x07
    NO_KEY,                                                            // 0x08
    NO_KEY,                                                            // 0x09
    NO_KEY,                                                            // 0x0A
    NO_KEY,                                                            // 0x0B
    NO_KEY,                                                            // 0x0C
    NO_KEY,                                                            // 0x0D
    NO_KEY,                                                            // 0x0E
    NO_KEY,                                                            // 0x0F
    NO_KEY,                                                            // 0x10
    NO_KEY,                                                            // 0x11
    NO_KEY,                                                            // 0x12
    NO_KEY,                                                            // 0x13
    NO_KEY,                                                            // 0x14
    NO_KEY,                                                            // 0x15
    NO_KEY,                                                            // 0x16
    NO_KEY,                                                            // 0x17
    NO_KEY,                                                            // 0x18
    NO_KEY,                                                            // 0x19
    NO_KEY,                                                            // 0x1A
    NO_KEY,                                                            // 0x1B
    NO_KEY,                                                            // 0x1C
    { .press = true , .ascii = false, .info = { .meta = KEY_CTRL  } }, // 0x1D
    NO_KEY,                                                            // 0x1E
    NO_KEY,                                                            // 0x1F
    NO_KEY,                                                            // 0x20
    NO_KEY,                                                            // 0x21
    NO_KEY,                                                            // 0x22
    NO_KEY,                                                            // 0x23
    NO_KEY,                                                            // 0x24
    NO_KEY,                                                            // 0x25
    NO_KEY,                                                            // 0x26
    NO_KEY,                                                            // 0x27
    NO_KEY,                                                            // 0x28
    NO_KEY,                                                            // 0x29
    NO_KEY,                                                            // 0x2A
    NO_KEY,                                                            // 0x2B
    NO_KEY,                                                            // 0x2C
    NO_KEY,                                                            // 0x2D
    NO_KEY,                                                            // 0x2E
    NO_KEY,                                                            // 0x2F
    NO_KEY,                                                            // 0x30
    NO_KEY,                                                            // 0x31
    NO_KEY,                                                            // 0x32
    NO_KEY,                                                            // 0x33
    NO_KEY,                                                            // 0x34
    NO_KEY,                                                            // 0x35
    NO_KEY,                                                            // 0x36
    NO_KEY,                                                            // 0x37
    { .press = true , .ascii = false, .info = { .meta = KEY_ALT   } }, // 0x38
    NO_KEY,                                                            // 0x39
    NO_KEY,                                                            // 0x3A
    NO_KEY,                                                            // 0x3B
    NO_KEY,                                                            // 0x3C
    NO_KEY,                                                            // 0x3D
    NO_KEY,                                                            // 0x3E
    NO_KEY,                                                            // 0x3F
    NO_KEY,                                                            // 0x40
    NO_KEY,                                                            // 0x41
    NO_KEY,                                                            // 0x42
    NO_KEY,                                                            // 0x43
    NO_KEY,                                                            // 0x44
    NO_KEY,                                                            // 0x45
    NO_KEY,                                                            // 0x46
    { .press = true , .ascii = false, .info = { .meta = KEY_HOME  } }, // 0x47
    { .press = true , .ascii = false, .info = { .meta = KEY_UP    } }, // 0x48
    { .press = true , .ascii = false, .info = { .meta = KEY_PGUP  } }, // 0x49
    NO_KEY,                                                            // 0x4A
    { .press = true , .ascii = false, .info = { .meta = KEY_LEFT  } }, // 0x4B
    NO_KEY,                                                            // 0x4C
    { .press = true , .ascii = false, .info = { .meta = KEY_RIGHT } }, // 0x4D
    NO_KEY,                                                            // 0x4E
    { .press = true , .ascii = false, .info = { .meta = KEY_END   } }, // 0x4F
    { .press = true , .ascii = false, .info = { .meta = KEY_DOWN  } }, // 0x50
    { .press = true , .ascii = false, .info = { .meta = KEY_PGDN  } }, // 0x51
    { .press = true , .ascii = false, .info = { .meta = KEY_INS   } }, // 0x52
    { .press = true , .ascii = false, .info = { .meta = KEY_DEL   } }, // 0x53
    NO_KEY,                                                            // 0x54
    NO_KEY,                                                            // 0x55
    NO_KEY,                                                            // 0x56
    NO_KEY,                                                            // 0x57
    NO_KEY,                                                            // 0x58
    NO_KEY,                                                            // 0x59
    NO_KEY,                                                            // 0x5A
    NO_KEY,                                                            // 0x5B
    NO_KEY,                                                            // 0x5C
    NO_KEY,                                                            // 0x5D
    NO_KEY,                                                            // 0x5E
    NO_KEY,                                                            // 0x5F
    NO_KEY,                                                            // 0x60
    NO_KEY,                                                            // 0x61
    NO_KEY,                                                            // 0x62
    NO_KEY,                                                            // 0x63
    NO_KEY,                                                            // 0x64
    NO_KEY,                                                            // 0x65
    NO_KEY,                                                            // 0x66
    NO_KEY,                                                            // 0x67
    NO_KEY,                                                            // 0x68
    NO_KEY,                                                            // 0x69
    NO_KEY,                                                            // 0x6A
    NO_KEY,                                                            // 0x6B
    NO_KEY,                                                            // 0x6C
    NO_KEY,                                                            // 0x6D
    NO_KEY,                                                            // 0x6E
    NO_KEY,                                                            // 0x6F
    NO_KEY,                                                            // 0x70
    NO_KEY,                                                            // 0x71
    NO_KEY,                                                            // 0x72
    NO_KEY,                                                            // 0x73
    NO_KEY,                                                            // 0x74
    NO_KEY,                                                            // 0x75
    NO_KEY,                                                            // 0x76
    NO_KEY,                                                            // 0x77
    NO_KEY,                                                            // 0x78
    NO_KEY,                                                            // 0x79
    NO_KEY,                                                            // 0x7A
    NO_KEY,                                                            // 0x7B
    NO_KEY,                                                            // 0x7C
    NO_KEY,                                                            // 0x7D
    NO_KEY,                                                            // 0x7E
    NO_KEY,                                                            // 0x7F
    NO_KEY,                                                            // 0x80
    NO_KEY,                                                            // 0x81
    NO_KEY,                                                            // 0x82
    NO_KEY,                                                            // 0x83
    NO_KEY,                                                            // 0x84
    NO_KEY,                                                            // 0x85
    NO_KEY,                                                            // 0x86
    NO_KEY,                                                            // 0x87
    NO_KEY,                                                            // 0x88
    NO_KEY,                                                            // 0x89
    NO_KEY,                                                            // 0x8A
    NO_KEY,                                                            // 0x8B
    NO_KEY,                                                            // 0x8C
    NO_KEY,                                                            // 0x8D
    NO_KEY,                                                            // 0x8E
    NO_KEY,                                                            // 0x8F
    NO_KEY,                                                            // 0x90
    NO_KEY,                                                            // 0x91
    NO_KEY,                                                            // 0x92
    NO_KEY,                                                            // 0x93
    NO_KEY,                                                            // 0x94
    NO_KEY,                                                            // 0x95
    NO_KEY,                                                            // 0x96
    NO_KEY,                                                            // 0x97
    NO_KEY,                                                            // 0x98
    NO_KEY,                                                            // 0x99
    NO_KEY,                                                            // 0x9A
    NO_KEY,                                                            // 0x9B
    NO_KEY,                                                            // 0x9C
    { .press = false, .ascii = false, .info = { .meta = KEY_CTRL  } }, // 0x9D
    NO_KEY,                                                            // 0x9E
    NO_KEY,                                                            // 0x9F
    NO_KEY,                                                            // 0xA0
    NO_KEY,                                                            // 0xA1
    NO_KEY,                                                            // 0xA2
    NO_KEY,                                                            // 0xA3
    NO_KEY,                                                            // 0xA4
    NO_KEY,                                                            // 0xA5
    NO_KEY,                                                            // 0xA6
    NO_KEY,                                                            // 0xA7
    NO_KEY,                                                            // 0xA8
    NO_KEY,                                                            // 0xA9
    NO_KEY,                                                            // 0xAA
    NO_KEY,                                                            // 0xAB
    NO_KEY,                                                            // 0xAC
    NO_KEY,                                                            // 0xAD
    NO_KEY,                                                            // 0xAE
    NO_KEY,                                                            // 0xAF
    NO_KEY,                                                            // 0xB0
    NO_KEY,                                                            // 0xB1
    NO_KEY,                                                            // 0xB2
    NO_KEY,                                                            // 0xB3
    NO_KEY,                                                            // 0xB4
    NO_KEY,                                                            // 0xB5
    NO_KEY,                                                            // 0xB6
    NO_KEY,                                                            // 0xB7
    { .press = false, .ascii = false, .info = { .meta = KEY_ALT   } }, // 0xB8
    NO_KEY,                                                            // 0xB9
    NO_KEY,                                                            // 0xBA
    NO_KEY,                                                            // 0xBB
    NO_KEY,                                                            // 0xBC
    NO_KEY,                                                            // 0xBD
    NO_KEY,                                                            // 0xBE
    NO_KEY,                                                            // 0xBF
    NO_KEY,                                                            // 0xC0
    NO_KEY,                                                            // 0xC1
    NO_KEY,                                                            // 0xC2
    NO_KEY,                                                            // 0xC3
    NO_KEY,                                                            // 0xC4
    NO_KEY,                                                            // 0xC5
    NO_KEY,                                                            // 0xC6
    { .press = false, .ascii = false, .info = { .meta = KEY_HOME  } }, // 0xC7
    { .press = false, .ascii = false, .info = { .meta = KEY_UP    } }, // 0xC8
    { .press = false, .ascii = false, .info = { .meta = KEY_PGUP  } }, // 0xC9
    NO_KEY,                                                            // 0xCA
    { .press = false, .ascii = false, .info = { .meta = KEY_LEFT  } }, // 0xCB
    NO_KEY,                                                            // 0xCC
    { .press = false, .ascii = false, .info = { .meta = KEY_RIGHT } }, // 0xCD
    NO_KEY,                                                            // 0xCE
    { .press = false, .ascii = false, .info = { .meta = KEY_END   } }, // 0xCF
    { .press = false, .ascii = false, .info = { .meta = KEY_DOWN  } }, // 0xD0
    { .press = false, .ascii = false, .info = { .meta = KEY_PGDN  } }, // 0xD1
    { .press = false, .ascii = false, .info = { .meta = KEY_INS   } }, // 0xD2
    { .press = false, .ascii = false, .info = { .meta = KEY_DEL   } }, // 0xD3
    NO_KEY,                                                            // 0xD4
    NO_KEY,                                                            // 0xD5
    NO_KEY,                                                            // 0xD6
    NO_KEY,                                                            // 0xD7
    NO_KEY,                                                            // 0xD8
    NO_KEY,                                                            // 0xD9
    NO_KEY,                                                            // 0xDA
    NO_KEY,                                                            // 0xDB
    NO_KEY,                                                            // 0xDC
    NO_KEY,                                                            // 0xDD
    NO_KEY,                                                            // 0xDE
    NO_KEY,                                                            // 0xDF
};


/**
 * Keyboard interrupt handler registered for IRQ # 0.
 * Currently just prints a tick message.
 */
static void keyboard_interrupt_handler(struct interrupt_state *state) {
    (void) state;   /** Unused. */
    print("__0\n");

    // FIXME why allocate and not use a pointer?
    struct keyboard_key_event event = NO_KEY;

    /**
     * Read our the event's scancode. Translate the scancode into a key
     * event, following the scancode set 1 mappings.
     */
    uint8_t scancode = inb(IO_PORT_KBD_DATA);
    if (scancode < 0xE0)
        event = scancode_event_map[scancode];
    else if (scancode == 0xE0) {    // extended set
        uint8_t extendcode = inb(IO_PORT_KBD_DATA);
        if (extendcode < 0xE0)
            event = extendcode_event_map[extendcode];
        else
            ; // shoouldn't happen since FIXME why ignored?
    }

    if (event.press && event.ascii) {
        printi("key=", event.info.codel);
    }
}

/** Initialize the PS/2 keyboard device. */
void keyboard_init() {
    isr_register(IDT_IRQ_KEYBOARD, &keyboard_interrupt_handler);

    pic_enable_irq_line(PIC_INT_KEYBOARD);
}
