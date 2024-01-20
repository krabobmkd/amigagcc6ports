#ifndef INPUT_H
#define INPUT_H

typedef unsigned InputCode;

struct KeyboardInfo
{
	char *name; /* OS dependant name; 0 terminates the list */
	unsigned code; /* OS dependant code */
	InputCode standardcode;	/* CODE_xxx equivalent from list below, or CODE_OTHER if n/a */
};

struct JoystickInfo
{
	char *name; /* OS dependant name; 0 terminates the list */
	unsigned code; /* OS dependant code */
	InputCode standardcode;	/* CODE_xxx equivalent from list below, or CODE_OTHER if n/a */
};

enum
{
	/* key */
	KeyCode_A, KeyCode_B, KeyCode_C, KeyCode_D, KeyCode_E, KeyCode_F,
	KeyCode_G, KeyCode_H, KeyCode_I, KeyCode_J, KeyCode_K, KeyCode_L,
	KeyCode_M, KeyCode_N, KeyCode_O, KeyCode_P, KeyCode_Q, KeyCode_R,
	KeyCode_S, KeyCode_T, KeyCode_U, KeyCode_V, KeyCode_W, KeyCode_X,
	KeyCode_Y, KeyCode_Z, KeyCode_0, KeyCode_1, KeyCode_2, KeyCode_3,
	KeyCode_4, KeyCode_5, KeyCode_6, KeyCode_7, KeyCode_8, KeyCode_9,
	KeyCode_0_PAD, KeyCode_1_PAD, KeyCode_2_PAD, KeyCode_3_PAD, KeyCode_4_PAD,
	KeyCode_5_PAD, KeyCode_6_PAD, KeyCode_7_PAD, KeyCode_8_PAD, KeyCode_9_PAD,
	KeyCode_F1, KeyCode_F2, KeyCode_F3, KeyCode_F4, KeyCode_F5,
	KeyCode_F6, KeyCode_F7, KeyCode_F8, KeyCode_F9, KeyCode_F10,
	KeyCode_F11, KeyCode_F12,
	KeyCode_ESC, KeyCode_TILDE, KeyCode_MINUS, KeyCode_EQUALS, KeyCode_BACKSPACE,
	KeyCode_TAB, KeyCode_OPENBRACE, KeyCode_CLOSEBRACE, KeyCode_ENTER, KeyCode_COLON,
	KeyCode_QUOTE, KeyCode_BACKSLASH, KeyCode_BACKSLASH2, KeyCode_COMMA, KeyCode_STOP,
	KeyCode_SLASH, KeyCode_SPACE, KeyCode_INSERT, KeyCode_DEL,
	KeyCode_HOME, KeyCode_END, KeyCode_PGUP, KeyCode_PGDN, KeyCode_LEFT,
	KeyCode_RIGHT, KeyCode_UP, KeyCode_DOWN,
	KeyCode_SLASH_PAD, KeyCode_ASTERISK, KeyCode_MINUS_PAD, KeyCode_PLUS_PAD,
	KeyCode_DEL_PAD, KeyCode_ENTER_PAD, KeyCode_PRTSCR, KeyCode_PAUSE,
	KeyCode_LSHIFT, KeyCode_RSHIFT, KeyCode_LCONTROL, KeyCode_RCONTROL,
	KeyCode_LALT, KeyCode_RALT, KeyCode_SCRLOCK, KeyCode_NUMLOCK, KeyCode_CAPSLOCK,
	KeyCode_LWIN, KeyCode_RWIN, KeyCode_MENU,
#define __code_key_first KeyCode_A
#define __code_key_last KeyCode_MENU

	/* joy */
	JOYCODE_1_LEFT,JOYCODE_1_RIGHT,JOYCODE_1_UP,JOYCODE_1_DOWN,
	JOYCODE_1_BUTTON1,JOYCODE_1_BUTTON2,JOYCODE_1_BUTTON3,
	JOYCODE_1_BUTTON4,JOYCODE_1_BUTTON5,JOYCODE_1_BUTTON6,
	JOYCODE_2_LEFT,JOYCODE_2_RIGHT,JOYCODE_2_UP,JOYCODE_2_DOWN,
	JOYCODE_2_BUTTON1,JOYCODE_2_BUTTON2,JOYCODE_2_BUTTON3,
	JOYCODE_2_BUTTON4,JOYCODE_2_BUTTON5,JOYCODE_2_BUTTON6,
	JOYCODE_3_LEFT,JOYCODE_3_RIGHT,JOYCODE_3_UP,JOYCODE_3_DOWN,
	JOYCODE_3_BUTTON1,JOYCODE_3_BUTTON2,JOYCODE_3_BUTTON3,
	JOYCODE_3_BUTTON4,JOYCODE_3_BUTTON5,JOYCODE_3_BUTTON6,
	JOYCODE_4_LEFT,JOYCODE_4_RIGHT,JOYCODE_4_UP,JOYCODE_4_DOWN,
	JOYCODE_4_BUTTON1,JOYCODE_4_BUTTON2,JOYCODE_4_BUTTON3,
	JOYCODE_4_BUTTON4,JOYCODE_4_BUTTON5,JOYCODE_4_BUTTON6,
#define __code_joy_first JOYCODE_1_LEFT
#define __code_joy_last JOYCODE_4_BUTTON6

	__code_max, /* Temination of standard code */

	/* special */
	CODE_NONE = 0x8000, /* no code, also marker of sequence end */
	CODE_OTHER, /* OS code not mapped to any other code */
	CODE_DEFAULT, /* special for input port definitions */
        CODE_PREVIOUS, /* special for input port definitions */
	CODE_NOT, /* operators for sequences */
	CODE_OR /* operators for sequences */
};

/* Wrapper for compatibility */
#define KeyCode_OTHER CODE_OTHER
#define JOYCODE_OTHER CODE_OTHER
#define KeyCode_NONE CODE_NONE
#define JOYCODE_NONE CODE_NONE

/***************************************************************************/
/* Single code functions */

int code_init(void);
void code_close(void);

InputCode keyoscode_to_code(unsigned oscode);
InputCode joyoscode_to_code(unsigned oscode);
InputCode savecode_to_code(unsigned savecode);
unsigned code_to_savecode(InputCode code);

const char *code_name(InputCode code);
int code_pressed(InputCode code);
int code_pressed_memory(InputCode code);
int code_pressed_memory_repeat(InputCode code, int speed);
InputCode code_read_async(void);
InputCode code_read_sync(void);
INT8 code_read_hex_async(void);

/* Wrappers for compatibility */
#define keyboard_name                   code_name
#define keyboard_pressed                code_pressed
#define keyboard_pressed_memory         code_pressed_memory
#define keyboard_pressed_memory_repeat  code_pressed_memory_repeat
#define keyboard_read_async	        code_read_async
#define keyboard_read_sync              code_read_sync

/***************************************************************************/
/* Sequence code funtions */

/* NOTE: If you modify this value you need also to modify the SEQ_DEF declarations */
#define SEQ_MAX 16

typedef InputCode InputSeq[SEQ_MAX];

INLINE InputCode seq_get_1(InputSeq* a) {
	return (*a)[0];
}

void seq_set_0(InputSeq* seq);
void seq_set_1(InputSeq* seq, InputCode code);
void seq_set_2(InputSeq* seq, InputCode code1, InputCode code2);
void seq_set_3(InputSeq* seq, InputCode code1, InputCode code2, InputCode code3);
void seq_copy(InputSeq* seqdst, InputSeq* seqsrc);
int seq_cmp(InputSeq* seq1, InputSeq* seq2);
void seq_name(InputSeq* seq, char* buffer, unsigned max);
int seq_pressed(InputSeq* seq);
void seq_read_async_start(void);
int seq_read_async(InputSeq* code, int first);

/* NOTE: It's very important that this sequence is EXACLY long SEQ_MAX */
#define SEQ_DEF_6(a,b,c,d,e,f) { a, b, c, d, e, f, CODE_NONE, CODE_NONE, CODE_NONE, CODE_NONE, CODE_NONE, CODE_NONE, CODE_NONE, CODE_NONE, CODE_NONE, CODE_NONE }
#define SEQ_DEF_5(a,b,c,d,e) SEQ_DEF_6(a,b,c,d,e,CODE_NONE)
#define SEQ_DEF_4(a,b,c,d) SEQ_DEF_5(a,b,c,d,CODE_NONE)
#define SEQ_DEF_3(a,b,c) SEQ_DEF_4(a,b,c,CODE_NONE)
#define SEQ_DEF_2(a,b) SEQ_DEF_3(a,b,CODE_NONE)
#define SEQ_DEF_1(a) SEQ_DEF_2(a,CODE_NONE)
#define SEQ_DEF_0 SEQ_DEF_1(CODE_NONE)

/***************************************************************************/
/* input_ui */

int input_ui_pressed(int code);
int input_ui_pressed_repeat(int code, int speed);

#endif
