#ifndef LEGACY_H
#define LEGACY_H

/* Functions and Tables used for converting old file to the new format */
/* Define NOLEGACY for excluding the inclusion */

enum
{
	ver_567_KeyCode_START = 0xff00,
	ver_567_KeyCode_A, ver_567_KeyCode_B, ver_567_KeyCode_C, ver_567_KeyCode_D, ver_567_KeyCode_E, ver_567_KeyCode_F,
	ver_567_KeyCode_G, ver_567_KeyCode_H, ver_567_KeyCode_I, ver_567_KeyCode_J, ver_567_KeyCode_K, ver_567_KeyCode_L,
	ver_567_KeyCode_M, ver_567_KeyCode_N, ver_567_KeyCode_O, ver_567_KeyCode_P, ver_567_KeyCode_Q, ver_567_KeyCode_R,
	ver_567_KeyCode_S, ver_567_KeyCode_T, ver_567_KeyCode_U, ver_567_KeyCode_V, ver_567_KeyCode_W, ver_567_KeyCode_X,
	ver_567_KeyCode_Y, ver_567_KeyCode_Z, ver_567_KeyCode_0, ver_567_KeyCode_1, ver_567_KeyCode_2, ver_567_KeyCode_3,
	ver_567_KeyCode_4, ver_567_KeyCode_5, ver_567_KeyCode_6, ver_567_KeyCode_7, ver_567_KeyCode_8, ver_567_KeyCode_9,
	ver_567_KeyCode_0_PAD, ver_567_KeyCode_1_PAD, ver_567_KeyCode_2_PAD, ver_567_KeyCode_3_PAD, ver_567_KeyCode_4_PAD,
	ver_567_KeyCode_5_PAD, ver_567_KeyCode_6_PAD, ver_567_KeyCode_7_PAD, ver_567_KeyCode_8_PAD, ver_567_KeyCode_9_PAD,
	ver_567_KeyCode_F1, ver_567_KeyCode_F2, ver_567_KeyCode_F3, ver_567_KeyCode_F4, ver_567_KeyCode_F5,
	ver_567_KeyCode_F6, ver_567_KeyCode_F7, ver_567_KeyCode_F8, ver_567_KeyCode_F9, ver_567_KeyCode_F10,
	ver_567_KeyCode_F11, ver_567_KeyCode_F12,
	ver_567_KeyCode_ESC, ver_567_KeyCode_TILDE, ver_567_KeyCode_MINUS, ver_567_KeyCode_EQUALS, ver_567_KeyCode_BACKSPACE,
	ver_567_KeyCode_TAB, ver_567_KeyCode_OPENBRACE, ver_567_KeyCode_CLOSEBRACE, ver_567_KeyCode_ENTER, ver_567_KeyCode_COLON,
	ver_567_KeyCode_QUOTE, ver_567_KeyCode_BACKSLASH, ver_567_KeyCode_BACKSLASH2, ver_567_KeyCode_COMMA, ver_567_KeyCode_STOP,
	ver_567_KeyCode_SLASH, ver_567_KeyCode_SPACE, ver_567_KeyCode_INSERT, ver_567_KeyCode_DEL,
	ver_567_KeyCode_HOME, ver_567_KeyCode_END, ver_567_KeyCode_PGUP, ver_567_KeyCode_PGDN, ver_567_KeyCode_LEFT,
	ver_567_KeyCode_RIGHT, ver_567_KeyCode_UP, ver_567_KeyCode_DOWN,
	ver_567_KeyCode_SLASH_PAD, ver_567_KeyCode_ASTERISK, ver_567_KeyCode_MINUS_PAD, ver_567_KeyCode_PLUS_PAD,
	ver_567_KeyCode_DEL_PAD, ver_567_KeyCode_ENTER_PAD, ver_567_KeyCode_PRTSCR, ver_567_KeyCode_PAUSE,
	ver_567_KeyCode_LSHIFT, ver_567_KeyCode_RSHIFT, ver_567_KeyCode_LCONTROL, ver_567_KeyCode_RCONTROL,
	ver_567_KeyCode_LALT, ver_567_KeyCode_RALT, ver_567_KeyCode_SCRLOCK, ver_567_KeyCode_NUMLOCK, ver_567_KeyCode_CAPSLOCK,
	ver_567_KeyCode_OTHER,
	ver_567_KeyCode_NONE
};

enum
{
	ver_567_JOYCODE_START = 0xff00,
	ver_567_JOYCODE_1_LEFT,ver_567_JOYCODE_1_RIGHT,ver_567_JOYCODE_1_UP,ver_567_JOYCODE_1_DOWN,
	ver_567_JOYCODE_1_BUTTON1,ver_567_JOYCODE_1_BUTTON2,ver_567_JOYCODE_1_BUTTON3,
	ver_567_JOYCODE_1_BUTTON4,ver_567_JOYCODE_1_BUTTON5,ver_567_JOYCODE_1_BUTTON6,
	ver_567_JOYCODE_2_LEFT,ver_567_JOYCODE_2_RIGHT,ver_567_JOYCODE_2_UP,ver_567_JOYCODE_2_DOWN,
	ver_567_JOYCODE_2_BUTTON1,ver_567_JOYCODE_2_BUTTON2,ver_567_JOYCODE_2_BUTTON3,
	ver_567_JOYCODE_2_BUTTON4,ver_567_JOYCODE_2_BUTTON5,ver_567_JOYCODE_2_BUTTON6,
	ver_567_JOYCODE_3_LEFT,ver_567_JOYCODE_3_RIGHT,ver_567_JOYCODE_3_UP,ver_567_JOYCODE_3_DOWN,
	ver_567_JOYCODE_3_BUTTON1,ver_567_JOYCODE_3_BUTTON2,ver_567_JOYCODE_3_BUTTON3,
	ver_567_JOYCODE_3_BUTTON4,ver_567_JOYCODE_3_BUTTON5,ver_567_JOYCODE_3_BUTTON6,
	ver_567_JOYCODE_4_LEFT,ver_567_JOYCODE_4_RIGHT,ver_567_JOYCODE_4_UP,ver_567_JOYCODE_4_DOWN,
	ver_567_JOYCODE_4_BUTTON1,ver_567_JOYCODE_4_BUTTON2,ver_567_JOYCODE_4_BUTTON3,
	ver_567_JOYCODE_4_BUTTON4,ver_567_JOYCODE_4_BUTTON5,ver_567_JOYCODE_4_BUTTON6,
	ver_567_JOYCODE_OTHER,
	ver_567_JOYCODE_NONE
};

#define ver_567_IP_KEY_DEFAULT 0xffff
#define ver_567_IP_KEY_NONE 0xfffe
#define ver_567_IP_KEY_PREVIOUS 0xfffd
#define ver_567_IP_JOY_DEFAULT 0xffff
#define ver_567_IP_JOY_NONE 0xfffe
#define ver_567_IP_JOY_PREVIOUS 0xfffd
#define ver_567_IP_CODE_NOT 0xfffc
#define ver_567_IP_CODE_OR 0xfffb

struct ver_table {
	int pre;
	int post;
};

static struct ver_table ver_567_table_keyboard[] = {
	{ 0, CODE_NONE },
	{ ver_567_KeyCode_A, KeyCode_A },
	{ ver_567_KeyCode_B, KeyCode_B },
	{ ver_567_KeyCode_C, KeyCode_C },
	{ ver_567_KeyCode_D, KeyCode_D },
	{ ver_567_KeyCode_E, KeyCode_E },
	{ ver_567_KeyCode_F, KeyCode_F },
	{ ver_567_KeyCode_G, KeyCode_G },
	{ ver_567_KeyCode_H, KeyCode_H },
	{ ver_567_KeyCode_I, KeyCode_I },
	{ ver_567_KeyCode_J, KeyCode_J },
	{ ver_567_KeyCode_K, KeyCode_K },
	{ ver_567_KeyCode_L, KeyCode_L },
	{ ver_567_KeyCode_M, KeyCode_M },
	{ ver_567_KeyCode_N, KeyCode_N },
	{ ver_567_KeyCode_O, KeyCode_O },
	{ ver_567_KeyCode_P, KeyCode_P },
	{ ver_567_KeyCode_Q, KeyCode_Q },
	{ ver_567_KeyCode_R, KeyCode_R },
	{ ver_567_KeyCode_S, KeyCode_S },
	{ ver_567_KeyCode_T, KeyCode_T },
	{ ver_567_KeyCode_U, KeyCode_U },
	{ ver_567_KeyCode_V, KeyCode_V },
	{ ver_567_KeyCode_W, KeyCode_W },
	{ ver_567_KeyCode_X, KeyCode_X },
	{ ver_567_KeyCode_Y, KeyCode_Y },
	{ ver_567_KeyCode_Z, KeyCode_Z },
	{ ver_567_KeyCode_0, KeyCode_0 },
	{ ver_567_KeyCode_1, KeyCode_1 },
	{ ver_567_KeyCode_2, KeyCode_2 },
	{ ver_567_KeyCode_3, KeyCode_3 },
	{ ver_567_KeyCode_4, KeyCode_4 },
	{ ver_567_KeyCode_5, KeyCode_5 },
	{ ver_567_KeyCode_6, KeyCode_6 },
	{ ver_567_KeyCode_7, KeyCode_7 },
	{ ver_567_KeyCode_8, KeyCode_8 },
	{ ver_567_KeyCode_9, KeyCode_9 },
	{ ver_567_KeyCode_0_PAD, KeyCode_0_PAD },
	{ ver_567_KeyCode_1_PAD, KeyCode_1_PAD },
	{ ver_567_KeyCode_2_PAD, KeyCode_2_PAD },
	{ ver_567_KeyCode_3_PAD, KeyCode_3_PAD },
	{ ver_567_KeyCode_4_PAD, KeyCode_4_PAD },
	{ ver_567_KeyCode_5_PAD, KeyCode_5_PAD },
	{ ver_567_KeyCode_6_PAD, KeyCode_6_PAD },
	{ ver_567_KeyCode_7_PAD, KeyCode_7_PAD },
	{ ver_567_KeyCode_8_PAD, KeyCode_8_PAD },
	{ ver_567_KeyCode_9_PAD, KeyCode_9_PAD },
	{ ver_567_KeyCode_F1, KeyCode_F1 },
	{ ver_567_KeyCode_F2, KeyCode_F2 },
	{ ver_567_KeyCode_F3, KeyCode_F3 },
	{ ver_567_KeyCode_F4, KeyCode_F4 },
	{ ver_567_KeyCode_F5, KeyCode_F5 },
	{ ver_567_KeyCode_F6, KeyCode_F6 },
	{ ver_567_KeyCode_F7, KeyCode_F7 },
	{ ver_567_KeyCode_F8, KeyCode_F8 },
	{ ver_567_KeyCode_F9, KeyCode_F9 },
	{ ver_567_KeyCode_F10, KeyCode_F10 },
	{ ver_567_KeyCode_F11, KeyCode_F11 },
	{ ver_567_KeyCode_F12, KeyCode_F12 },
	{ ver_567_KeyCode_ESC, KeyCode_ESC },
	{ ver_567_KeyCode_TILDE, KeyCode_TILDE },
	{ ver_567_KeyCode_MINUS, KeyCode_MINUS },
	{ ver_567_KeyCode_EQUALS, KeyCode_EQUALS },
	{ ver_567_KeyCode_BACKSPACE, KeyCode_BACKSPACE },
	{ ver_567_KeyCode_TAB, KeyCode_TAB },
	{ ver_567_KeyCode_OPENBRACE, KeyCode_OPENBRACE },
	{ ver_567_KeyCode_CLOSEBRACE, KeyCode_CLOSEBRACE },
	{ ver_567_KeyCode_ENTER, KeyCode_ENTER },
	{ ver_567_KeyCode_COLON, KeyCode_COLON },
	{ ver_567_KeyCode_QUOTE, KeyCode_QUOTE },
	{ ver_567_KeyCode_BACKSLASH, KeyCode_BACKSLASH },
	{ ver_567_KeyCode_BACKSLASH2, KeyCode_BACKSLASH2 },
	{ ver_567_KeyCode_COMMA, KeyCode_COMMA },
	{ ver_567_KeyCode_STOP, KeyCode_STOP },
	{ ver_567_KeyCode_SLASH, KeyCode_SLASH },
	{ ver_567_KeyCode_SPACE, KeyCode_SPACE },
	{ ver_567_KeyCode_INSERT, KeyCode_INSERT },
	{ ver_567_KeyCode_DEL, KeyCode_DEL },
	{ ver_567_KeyCode_HOME, KeyCode_HOME },
	{ ver_567_KeyCode_END, KeyCode_END },
	{ ver_567_KeyCode_PGUP, KeyCode_PGUP },
	{ ver_567_KeyCode_PGDN, KeyCode_PGDN },
	{ ver_567_KeyCode_LEFT, KeyCode_LEFT },
	{ ver_567_KeyCode_RIGHT, KeyCode_RIGHT },
	{ ver_567_KeyCode_UP, KeyCode_UP },
	{ ver_567_KeyCode_DOWN, KeyCode_DOWN },
	{ ver_567_KeyCode_SLASH_PAD, KeyCode_SLASH_PAD },
	{ ver_567_KeyCode_ASTERISK, KeyCode_ASTERISK },
	{ ver_567_KeyCode_MINUS_PAD, KeyCode_MINUS_PAD },
	{ ver_567_KeyCode_PLUS_PAD, KeyCode_PLUS_PAD },
	{ ver_567_KeyCode_DEL_PAD, KeyCode_DEL_PAD },
	{ ver_567_KeyCode_ENTER_PAD, KeyCode_ENTER_PAD },
	{ ver_567_KeyCode_PRTSCR, KeyCode_PRTSCR },
	{ ver_567_KeyCode_PAUSE, KeyCode_PAUSE },
	{ ver_567_KeyCode_LSHIFT, KeyCode_LSHIFT },
	{ ver_567_KeyCode_RSHIFT, KeyCode_RSHIFT },
	{ ver_567_KeyCode_LCONTROL, KeyCode_LCONTROL },
	{ ver_567_KeyCode_RCONTROL, KeyCode_RCONTROL },
	{ ver_567_KeyCode_LALT, KeyCode_LALT },
	{ ver_567_KeyCode_RALT, KeyCode_RALT },
	{ ver_567_KeyCode_SCRLOCK, KeyCode_SCRLOCK },
	{ ver_567_KeyCode_NUMLOCK, KeyCode_NUMLOCK },
	{ ver_567_KeyCode_CAPSLOCK, KeyCode_CAPSLOCK },
	{ ver_567_KeyCode_OTHER, CODE_OTHER },
	{ ver_567_KeyCode_NONE, CODE_NONE },
	{ ver_567_IP_KEY_DEFAULT, CODE_DEFAULT },
	{ ver_567_IP_KEY_NONE, CODE_NONE },
	{ ver_567_IP_KEY_PREVIOUS, CODE_PREVIOUS },
	{ ver_567_IP_CODE_NOT, CODE_NOT },
	{ ver_567_IP_CODE_OR, CODE_OR },
	{ -1, -1 }
};

static struct ver_table ver_567_table_joystick[] = {
	{ 0, CODE_NONE },
	{ ver_567_JOYCODE_1_LEFT, JOYCODE_1_LEFT },
	{ ver_567_JOYCODE_1_RIGHT, JOYCODE_1_RIGHT },
	{ ver_567_JOYCODE_1_UP, JOYCODE_1_UP },
	{ ver_567_JOYCODE_1_DOWN, JOYCODE_1_DOWN },
	{ ver_567_JOYCODE_1_BUTTON1, JOYCODE_1_BUTTON1 },
	{ ver_567_JOYCODE_1_BUTTON2, JOYCODE_1_BUTTON2 },
	{ ver_567_JOYCODE_1_BUTTON3, JOYCODE_1_BUTTON3 },
	{ ver_567_JOYCODE_1_BUTTON4, JOYCODE_1_BUTTON4 },
	{ ver_567_JOYCODE_1_BUTTON5, JOYCODE_1_BUTTON5 },
	{ ver_567_JOYCODE_1_BUTTON6, JOYCODE_1_BUTTON6 },
	{ ver_567_JOYCODE_2_LEFT, JOYCODE_2_LEFT },
	{ ver_567_JOYCODE_2_RIGHT, JOYCODE_2_RIGHT },
	{ ver_567_JOYCODE_2_UP, JOYCODE_2_UP },
	{ ver_567_JOYCODE_2_DOWN, JOYCODE_2_DOWN },
	{ ver_567_JOYCODE_2_BUTTON1, JOYCODE_2_BUTTON1 },
	{ ver_567_JOYCODE_2_BUTTON2, JOYCODE_2_BUTTON2 },
	{ ver_567_JOYCODE_2_BUTTON3, JOYCODE_2_BUTTON3 },
	{ ver_567_JOYCODE_2_BUTTON4, JOYCODE_2_BUTTON4 },
	{ ver_567_JOYCODE_2_BUTTON5, JOYCODE_2_BUTTON5 },
	{ ver_567_JOYCODE_2_BUTTON6, JOYCODE_2_BUTTON6 },
	{ ver_567_JOYCODE_3_LEFT, JOYCODE_3_LEFT },
	{ ver_567_JOYCODE_3_RIGHT, JOYCODE_3_RIGHT },
	{ ver_567_JOYCODE_3_UP, JOYCODE_3_UP },
	{ ver_567_JOYCODE_3_DOWN, JOYCODE_3_DOWN },
	{ ver_567_JOYCODE_3_BUTTON1, JOYCODE_3_BUTTON1 },
	{ ver_567_JOYCODE_3_BUTTON2, JOYCODE_3_BUTTON2 },
	{ ver_567_JOYCODE_3_BUTTON3, JOYCODE_3_BUTTON3 },
	{ ver_567_JOYCODE_3_BUTTON4, JOYCODE_3_BUTTON4 },
	{ ver_567_JOYCODE_3_BUTTON5, JOYCODE_3_BUTTON5 },
	{ ver_567_JOYCODE_3_BUTTON6, JOYCODE_3_BUTTON6 },
	{ ver_567_JOYCODE_4_LEFT, JOYCODE_4_LEFT },
	{ ver_567_JOYCODE_4_RIGHT, JOYCODE_4_RIGHT },
	{ ver_567_JOYCODE_4_UP, JOYCODE_4_UP },
	{ ver_567_JOYCODE_4_DOWN, JOYCODE_4_DOWN },
	{ ver_567_JOYCODE_4_BUTTON1, JOYCODE_4_BUTTON1 },
	{ ver_567_JOYCODE_4_BUTTON2, JOYCODE_4_BUTTON2 },
	{ ver_567_JOYCODE_4_BUTTON3, JOYCODE_4_BUTTON3 },
	{ ver_567_JOYCODE_4_BUTTON4, JOYCODE_4_BUTTON4 },
	{ ver_567_JOYCODE_4_BUTTON5, JOYCODE_4_BUTTON5 },
	{ ver_567_JOYCODE_4_BUTTON6, JOYCODE_4_BUTTON6 },
	{ ver_567_JOYCODE_OTHER, CODE_OTHER },
	{ ver_567_JOYCODE_NONE, CODE_NONE },
	{ ver_567_IP_JOY_DEFAULT, CODE_DEFAULT },
	{ ver_567_IP_JOY_NONE, CODE_NONE },
	{ ver_567_IP_JOY_PREVIOUS, CODE_PREVIOUS },
	{ ver_567_IP_CODE_NOT, CODE_NOT },
	{ ver_567_IP_CODE_OR, CODE_OR },
	{ -1, -1 }
};

static int code_table_ver_567_keyboard(int v) {
	struct ver_table* i = ver_567_table_keyboard;

	while (i->pre!=-1 || i->post!=-1) {
		if (i->pre == v)
			return i->post;
		++i;
	}

	v = keyoscode_to_code(v);
	if (v != CODE_NONE)
		return v;

	return -1;
}

static int code_table_ver_567_joystick(int v) {
	struct ver_table* i = ver_567_table_joystick;

	while (i->pre!=-1 || i->post!=-1) {
		if (i->pre == v)
			return i->post;
		++i;
	}

	v = joyoscode_to_code(v);
	if (v != CODE_NONE)
		return v;

	return -1;
}

static int seq_partial_is_special_code(InputCode code) {
	if (code < __code_max)
		return 0;
	if (code == CODE_NOT)
		return 0;
	return 1;
}

static int seq_partial_read(void* f, InputSeq* seq, unsigned* pos, unsigned len, int (*code_table)(int))
{
	UINT16 w;
	unsigned j = 0;
	int code;

	if (readword(f,&w) != 0)
		return -1;
	++j;
	code = code_table(w);
	if (code==-1)
		return -1;

	/* if default + standard code, use the standard code */
	if (*pos == 1 && (*seq)[0] == CODE_DEFAULT && !seq_partial_is_special_code(code))
		*pos = 0;

	if (
		/* if a special code is present don't insert another code */
		(*pos == 0 || !seq_partial_is_special_code((*seq)[0])) &&
		/* don't insert NONE code */
		(code != CODE_NONE) &&
		/* don't insert a special code after another code */
		(*pos == 0 || !seq_partial_is_special_code(code) ))
	{
		if (*pos)
		{
			(*seq)[*pos] = CODE_OR;
			++*pos;
		}

		(*seq)[*pos] = code;
		++*pos;

		while (j<len)
		{
			if (readword(f,&w) != 0)
				return -1;
			++j;

			code = code_table(w);
			if (code==-1)
				return -1;
			if (code == CODE_NONE)
				break;

			(*seq)[*pos] = code;
			++*pos;
		}
	}

	while (j<len)
	{
		if (readword(f,&w) != 0)
			return -1;
		++j;
	}

	return 0;
}

static int seq_read_ver_5(void* f, InputSeq* seq)
{
	unsigned pos = 0;
	seq_set_0(seq);

	if (seq_partial_read(f,seq,&pos,1,code_table_ver_567_keyboard) != 0)
		return -1;
	if (seq_partial_read(f,seq,&pos,1,code_table_ver_567_joystick) != 0)
		return -1;
	return 0;
}

static int seq_read_ver_6(void* f, InputSeq* seq)
{
	unsigned pos = 0;
	seq_set_0(seq);

	if (seq_partial_read(f,seq,&pos,2,code_table_ver_567_keyboard) != 0)
		return -1;
	if (seq_partial_read(f,seq,&pos,2,code_table_ver_567_joystick) != 0)
		return -1;
	return 0;
}

static int seq_read_ver_7(void* f, InputSeq* seq)
{
	unsigned pos = 0;
	seq_set_0(seq);

	if (seq_partial_read(f,seq,&pos,8,code_table_ver_567_keyboard) != 0)
		return -1;
	if (seq_partial_read(f,seq,&pos,8,code_table_ver_567_joystick) != 0)
		return -1;
	return 0;
}

static int input_port_read_ver_5(void *f, struct InputPort *in)
{
	UINT32 i;
	UINT16 w;
	if (readint(f,&i) != 0)
		return -1;
	in->type = i;

	if (readword(f,&w) != 0)
		return -1;
	in->mask = w;

	if (readword(f,&w) != 0)
		return -1;
	in->default_value = w;

	if (seq_read_ver_5(f,&in->seq) != 0)
		return -1;
	return 0;
}

static int input_port_read_ver_6(void *f, struct InputPort *in)
{
	UINT32 i;
	UINT16 w;
	if (readint(f,&i) != 0)
		return -1;
	in->type = i;

	if (readword(f,&w) != 0)
		return -1;
	in->mask = w;

	if (readword(f,&w) != 0)
		return -1;
	in->default_value = w;

	if (seq_read_ver_6(f,&in->seq) != 0)
		return -1;
	return 0;
}

static int input_port_read_ver_7(void *f, struct InputPort *in)
{
	UINT32 i;
	UINT16 w;
	if (readint(f,&i) != 0)
		return -1;
	in->type = i;

	if (readword(f,&w) != 0)
		return -1;
	in->mask = w;

	if (readword(f,&w) != 0)
		return -1;
	in->default_value = w;

	if (seq_read_ver_7(f,&in->seq) != 0)
		return -1;
	return 0;
}

#endif
