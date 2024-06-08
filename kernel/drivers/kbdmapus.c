#include "drivers/kbd.h"

/** Original xt scan code set */
const int kbd_scanmap_set1[KBD_BREAKCODE_LIMIT] = {
    [0x00] = KEY_NULL,
    //
    [0x01] = KEY_ESCAPE,
    [0x02] = KEY_1,
    [0x03] = KEY_2,
    [0x04] = KEY_3,
    [0x05] = KEY_4,
    [0x06] = KEY_5,
    [0x07] = KEY_6,
    [0x08] = KEY_7,
    [0x09] = KEY_8,
    [0x0a] = KEY_9,
    [0x0b] = KEY_0,
    [0x0c] = KEY_MINUS,
    [0x0d] = KEY_EQUAL,
    [0x0e] = KEY_BACKSPACE,
    //
    [0x0f] = KEY_TAB,
    [0x10] = KEY_Q,
    [0x11] = KEY_W,
    [0x12] = KEY_E,
    [0x13] = KEY_R,
    [0x14] = KEY_T,
    [0x15] = KEY_Y,
    [0x16] = KEY_U,
    [0x17] = KEY_I,
    [0x18] = KEY_O,
    [0x19] = KEY_P,
    [0x1a] = KEY_LEFTBRACKET,
    [0x1b] = KEY_RIGHTBRACKET,
    //
    [0x1c] = KEY_RETURN,
    //
    [0x1d] = KEY_LCTRL,
    //
    [0x1e] = KEY_A,
    [0x1f] = KEY_S,
    [0x20] = KEY_D,
    [0x21] = KEY_F,
    [0x22] = KEY_G,
    [0x23] = KEY_H,
    [0x24] = KEY_J,
    [0x25] = KEY_K,
    [0x26] = KEY_L,
    [0x27] = KEY_SEMICOLON,
    [0x28] = KEY_QUOTE,
    //
    [0x29] = KEY_GRAVE,
    //
    [0x2a] = KEY_LSHIFT,
    //
    [0x2b] = KEY_BACKSLASH,
    //
    [0x2c] = KEY_Z,
    [0x2d] = KEY_X,
    [0x2e] = KEY_C,
    [0x2f] = KEY_V,
    [0x30] = KEY_B,
    [0x31] = KEY_N,
    [0x32] = KEY_M,
    [0x33] = KEY_COMMA,
    [0x34] = KEY_DOT,
    [0x35] = KEY_SLASH,
    [0x36] = KEY_RSHIFT,
    //
    [0x37] = KEY_KP_ASTERISK,
    //
    [0x38] = KEY_LALT,
    [0x39] = KEY_SPACE,
    //
    [0x3a] = KEY_CAPSLOCK,
    //
    [0x3b] = KEY_F1,
    [0x3c] = KEY_F2,
    [0x3d] = KEY_F3,
    [0x3e] = KEY_F4,
    [0x3f] = KEY_F5,
    [0x40] = KEY_F6,
    [0x41] = KEY_F7,
    [0x42] = KEY_F8,
    [0x43] = KEY_F9,
    [0x44] = KEY_F10,
    //
    [0x45] = KEY_KP_NUMLOCK,
    [0x46] = KEY_SCROLLLOCK,
    [0x47] = KEY_HOME,
    [0x48] = KEY_KP_8,       //keypad up
    [0x49] = KEY_KP_9,       //keypad PgUp
    [0x50] = KEY_KP_2,       //keypad down
    [0x51] = KEY_KP_3,       //keypad PgDn
    [0x52] = KEY_KP_0,       //keypad insert
    [0x53] = KEY_KP_DECIMAL, //keypad delete
    //
    [0x54] = KEY_NULL,
    [0x55] = KEY_NULL,
    [0x56] = KEY_NULL,
    //
    [0x57] = KEY_F11,
    [0x58] = KEY_F12
};

/** Ascii representation of key codes */
const char *kbd_scanmap_ascii_regular[KBD_BREAKCODE_LIMIT] = {
    [KEY_ESCAPE]       = "\e",
    [KEY_1]            = "1",
    [KEY_2]            = "2",
    [KEY_3]            = "3",
    [KEY_4]            = "4",
    [KEY_5]            = "5",
    [KEY_6]            = "6",
    [KEY_7]            = "7",
    [KEY_8]            = "8",
    [KEY_9]            = "9",
    [KEY_0]            = "0",
    [KEY_MINUS]        = "-",
    [KEY_EQUAL]        = "=",
    [KEY_BACKSPACE]    = "\b",
    //
    [KEY_TAB]          = "\t",
    [KEY_Q]            = "q",
    [KEY_W]            = "w",
    [KEY_E]            = "e",
    [KEY_R]            = "r",
    [KEY_T]            = "t",
    [KEY_Y]            = "y",
    [KEY_U]            = "u",
    [KEY_I]            = "i",
    [KEY_O]            = "o",
    [KEY_P]            = "p",
    [KEY_LEFTBRACKET]  = "[",
    [KEY_RIGHTBRACKET] = "]",
    //
    [KEY_RETURN]       = "\n",
    //
    [KEY_LCTRL]        = 0,
    //
    [KEY_A]            = "a",
    [KEY_S]            = "s",
    [KEY_D]            = "d",
    [KEY_F]            = "f",
    [KEY_G]            = "g",
    [KEY_H]            = "h",
    [KEY_J]            = "j",
    [KEY_K]            = "k",
    [KEY_L]            = "l",
    [KEY_SEMICOLON]    = ";",
    [KEY_QUOTE]        = "'",
    //
    [KEY_GRAVE]        = "`",
    //
    [KEY_LSHIFT]       = 0,
    //
    [KEY_BACKSLASH]    = "\\",
    //
    [KEY_Z]            = "z",
    [KEY_X]            = "x",
    [KEY_C]            = "c",
    [KEY_V]            = "v",
    [KEY_B]            = "b",
    [KEY_N]            = "n",
    [KEY_M]            = "m",
    [KEY_COMMA]        = ",",
    [KEY_DOT]          = ".",
    [KEY_SLASH]        = "/",
    [KEY_RSHIFT]       = 0,
    //
    [KEY_KP_ASTERISK]  = "*",
    //
    [KEY_LALT]         = 0,
    [KEY_SPACE]        = " ",
    //
    [KEY_CAPSLOCK]     = 0,
    //
    [KEY_F1]           = "\eOP",
    [KEY_F2]           = "\eOQ",
    [KEY_F3]           = "\eOR",
    [KEY_F4]           = "\eOS",
    [KEY_F5]           = "\e[15~",
    [KEY_F6]           = "\e[17~",
    [KEY_F7]           = "\e[18~",
    [KEY_F8]           = "\e[19~",
    [KEY_F9]           = "\e[20~",
    [KEY_F10]          = "\e[21~",
    //
    [KEY_KP_NUMLOCK]   = 0,
    [KEY_SCROLLLOCK]   = 0,
    [KEY_HOME]         = 0,
    [KEY_KP_8]         = "8",
    [KEY_KP_9]         = "9",
    [KEY_KP_2]         = "2",
    [KEY_KP_3]         = "3",
    [KEY_KP_0]         = "0",
    [KEY_KP_DECIMAL]   = ".",
    //
    [KEY_F11]          = "\e[23~",
    [KEY_F12]          = "\e[24~",
};

const char *kbd_scanmap_ascii_shift[KBD_BREAKCODE_LIMIT] = {
    [KEY_ESCAPE]       = 0,
    [KEY_1]            = "!",
    [KEY_2]            = "@",
    [KEY_3]            = "#",
    [KEY_4]            = "$",
    [KEY_5]            = "%",
    [KEY_6]            = "^",
    [KEY_7]            = "&",
    [KEY_8]            = "*",
    [KEY_9]            = "(",
    [KEY_0]            = ")",
    [KEY_MINUS]        = "_",
    [KEY_EQUAL]        = "+",
    [KEY_BACKSPACE]    = "\b",
    //
    [KEY_TAB]          = "\e[Z",
    [KEY_Q]            = "Q",
    [KEY_W]            = "W",
    [KEY_E]            = "E",
    [KEY_R]            = "R",
    [KEY_T]            = "T",
    [KEY_Y]            = "Y",
    [KEY_U]            = "U",
    [KEY_I]            = "I",
    [KEY_O]            = "O",
    [KEY_P]            = "P",
    [KEY_LEFTBRACKET]  = "{",
    [KEY_RIGHTBRACKET] = "}",
    //
    [KEY_RETURN]       = "\n",
    //
    [KEY_LCTRL]        = 0,
    //
    [KEY_A]            = "A",
    [KEY_S]            = "S",
    [KEY_D]            = "D",
    [KEY_F]            = "F",
    [KEY_G]            = "G",
    [KEY_H]            = "H",
    [KEY_J]            = "J",
    [KEY_K]            = "K",
    [KEY_L]            = "L",
    [KEY_SEMICOLON]    = ":",
    [KEY_QUOTE]        = "\"",
    //
    [KEY_GRAVE]        = "~",
    //
    [KEY_LSHIFT]       = 0,
    //
    [KEY_BACKSLASH]    = "|",
    //
    [KEY_Z]            = "Z",
    [KEY_X]            = "X",
    [KEY_C]            = "C",
    [KEY_V]            = "V",
    [KEY_B]            = "B",
    [KEY_N]            = "N",
    [KEY_M]            = "M",
    [KEY_COMMA]        = "<",
    [KEY_DOT]          = ">",
    [KEY_SLASH]        = "?",
    [KEY_RSHIFT]       = 0,
    //
    [KEY_KP_ASTERISK]  = 0,
    //
    [KEY_LALT]         = 0,
    [KEY_SPACE]        = " ",
    //
    [KEY_CAPSLOCK]     = 0,
    //
    [KEY_F1]           = "\eOP",
    [KEY_F2]           = "\eOQ",
    [KEY_F3]           = "\eOR",
    [KEY_F4]           = "\eOS",
    [KEY_F5]           = "\e[15~;2~",
    [KEY_F6]           = "\e[17~;2~",
    [KEY_F7]           = "\e[18~;2~",
    [KEY_F8]           = "\e[19~;2~",
    [KEY_F9]           = "\e[20~;2~",
    [KEY_F10]          = "\e[21~;2~",
    //
    [KEY_KP_NUMLOCK]   = 0,
    [KEY_SCROLLLOCK]   = 0,
    [KEY_HOME]         = 0,
    [KEY_KP_8]         = "8",
    [KEY_KP_9]         = "9",
    [KEY_KP_2]         = "2",
    [KEY_KP_3]         = "3",
    [KEY_KP_0]         = "0",
    [KEY_KP_DECIMAL]   = ".",
    //
    [KEY_F11]          = "\e[23~;2~",
    [KEY_F12]          = "\e[24~;2~",
};
