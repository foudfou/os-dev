#ifndef KEYBOARD_H
#define KEYBOARD_H

#define KBD_BREAKCODE_LIMIT   0x80

/** Abstract representation of used key. This is part of the information our
    IRQ handler will return eventually. */
enum bkd_keycode {
    KEY_SPACE,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    //
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    //
    KEY_RETURN,
    KEY_ESCAPE,
    KEY_BACKSPACE,
    //
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    //
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_F13,
    KEY_F14,
    KEY_F15,
    //
    KEY_DOT,
    KEY_COMMA,
    KEY_COLON,
    KEY_SEMICOLON,
    KEY_SLASH,
    KEY_BACKSLASH,
    KEY_PLUS,
    KEY_MINUS,
    KEY_ASTERISK,
    KEY_EXCLAMATION,
    KEY_QUESTION,
    KEY_QUOTEDOUBLE,
    KEY_QUOTE,
    KEY_EQUAL,
    KEY_HASH,
    KEY_PERCENT,
    KEY_AMPERSAND,
    KEY_UNDERSCORE,
    KEY_LEFTPARENTHESIS,
    KEY_RIGHTPARENTHESIS,
    KEY_LEFTBRACKET,
    KEY_RIGHTBRACKET,
    KEY_LEFTCURL,
    KEY_RIGHTCURL,
    KEY_DOLLAR,
    KEY_POUND,
    KEY_EURO,
    KEY_LESS,
    KEY_GREATER,
    KEY_BAR,
    KEY_GRAVE,
    KEY_TILDE,
    KEY_AT,
    KEY_CARRET,
    //
    KEY_KP_0,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_PLUS,
    KEY_KP_MINUS,
    KEY_KP_DECIMAL,
    KEY_KP_DIVIDE,
    KEY_KP_ASTERISK,
    KEY_KP_NUMLOCK,
    KEY_KP_ENTER,
    //
    KEY_TAB,
    KEY_CAPSLOCK,
    //
    KEY_LSHIFT,
    KEY_LCTRL,
    KEY_LALT,
    KEY_LWIN,
    KEY_RSHIFT,
    KEY_RCTRL,
    KEY_RALT,
    KEY_RWIN,
    //
    KEY_INSERT,
    KEY_DELETE,
    KEY_HOME,
    KEY_END,
    KEY_PAGEUP,
    KEY_PAGEDOWN,
    KEY_SCROLLLOCK,
    KEY_PAUSE,
    //
    KEY_NULL,
    KEY_NUMKEYCODES,
};

void keyboard_init();

#endif /* KEYBOARD_H */
