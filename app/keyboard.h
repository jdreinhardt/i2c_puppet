#pragma once

#include <stdbool.h>
#include <stdint.h>

enum key_state
{
	KEY_STATE_IDLE = 0,
	KEY_STATE_PRESSED,
	KEY_STATE_HOLD,
	KEY_STATE_RELEASED,
};

enum key_mod
{
	KEY_MOD_ID_NONE = 0,
	KEY_MOD_ID_SYM,
	KEY_MOD_ID_ALT,
	KEY_MOD_ID_SHL,
	KEY_MOD_ID_SHR,

	KEY_MOD_ID_LAST,
};

#define KEY_JOY_UP		0x81
#define KEY_JOY_DOWN	0x82
#define KEY_JOY_LEFT	0x83
#define KEY_JOY_RIGHT	0x84
#define KEY_JOY_CENTER	0x85
#define KEY_BTN_LEFT1	0x86
#define KEY_BTN_RIGHT1	0x87
// 0x08 - BACKSPACE
// 0x09 - TAB
// 0x0A - NEW LINE
// 0x0D - CARRIAGE RETURN
#define KEY_BTN_LEFT2	0x91
#define KEY_BTN_RIGHT2	0x92

#define KEY_MOD_ALT		0x9A
#define KEY_MOD_SHL		0x9B // Left Shift
#define KEY_MOD_SHR		0x9C // Right Shift
#define KEY_MOD_SYM		0x9D

#define KEY_GUI			0xA0
#define KEY_ESCAPE		0xA1
#define KEY_CAPS_LOCK	0xA2
#define KEY_TAB			0xA3

#define KEY_PAGE_UP		0xB2
#define KEY_PAGE_DOWN	0xB3
#define KEY_HOME		0xB4
#define KEY_END			0xB5


struct key_callback
{
	void (*func)(char, enum key_state);
	struct key_callback *next;
};

struct key_lock_callback
{
	void (*func)(bool, bool);
	struct key_lock_callback *next;
};

void keyboard_inject_event(char key, enum key_state state);

bool keyboard_is_key_down(char key);
bool keyboard_is_mod_on(enum key_mod mod);

void keyboard_add_key_callback(struct key_callback *callback);
void keyboard_add_lock_callback(struct key_lock_callback *callback);

bool keyboard_get_capslock(void);
bool keyboard_get_numlock(void);

void keyboard_init(void);
