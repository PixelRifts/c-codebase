/* date = April 19th 2023 4:12 pm */

#ifndef X11_KEY_CODES_H
#define X11_KEY_CODES_H

// https://www.cl.cam.ac.uk/~mgk25/ucs/keysymdef.h

#define X11Input_Key_LeftArrow 0xff96
#define X11Input_Key_UpArrow 0xff97
#define X11Input_Key_RightArrow 0xff98
#define X11Input_Key_DownArrow 0xff99

#define X11Input_Key_Minus 0x002d
#define X11Input_Key_Equals 0x003d
#define X11Input_Key_Backspace 0xff08

#define X11Input_Key_Numpad0 0xffb0
#define X11Input_Key_Numpad1 0xffb1
#define X11Input_Key_Numpad2 0xffb2
#define X11Input_Key_Numpad3 0xffb3
#define X11Input_Key_Numpad4 0xffb4
#define X11Input_Key_Numpad5 0xffb5
#define X11Input_Key_Numpad6 0xffb6
#define X11Input_Key_Numpad7 0xffb7
#define X11Input_Key_Numpad8 0xffb8
#define X11Input_Key_Numpad9 0xffb9
#define X11Input_Key_NumpadPlus 0xffab
#define X11Input_Key_NumpadMinus 0xffad
#define X11Input_Key_NumpadStar 0xffaa
#define X11Input_Key_NumpadSlash 0xffaf
#define X11Input_Key_NumpadPeriod 0xffae

#define X11Input_Key_LeftShift 0xffe1
#define X11Input_Key_RightShift 0xffe2
#define X11Input_Key_LeftControl 0xffe3
#define X11Input_Key_RightControl 0xffe4
#define X11Input_Key_LeftAlt 0xffe9
#define X11Input_Key_RightAlt 0xffea
#define X11Input_Key_CapsLock 0xffe5
#define X11Input_Key_ScrollLock 0xff14
#define X11Input_Key_NumLock 0xff7f
#define X11Input_Key_Grave 0x0060
#define X11Input_Key_Enter 0xff0d

#define X11Input_Key_Period 0x002e
#define X11Input_Key_Comma 0x002c
#define X11Input_Key_ForwardSlash 0x002f
#define X11Input_Key_BackSlash 0x005c
#define X11Input_Key_Semicolon 0x003b
#define X11Input_Key_Apostrophe 0x0027
#define X11Input_Key_OpenBracket 0x005b
#define X11Input_Key_CloseBracket 0x005d
#define X11Input_Key_Escape 0xff1b
#define X11Input_Key_Pause 0xff13

#define X11Input_Key_F1  0xffbe
#define X11Input_Key_F2  0xffbf
#define X11Input_Key_F3  0xffc0
#define X11Input_Key_F4  0xffc1
#define X11Input_Key_F5  0xffc2
#define X11Input_Key_F6  0xffc3
#define X11Input_Key_F7  0xffc4
#define X11Input_Key_F8  0xffc5
#define X11Input_Key_F9  0xffc6
#define X11Input_Key_F10 0xffc7
#define X11Input_Key_F11 0xffc8
#define X11Input_Key_F12 0xffc9

#define X11Input_Key_PageUp 0xff55
#define X11Input_Key_PageDown 0xff56
#define X11Input_Key_End 0xff57
#define X11Input_Key_Home 0xff50
#define X11Input_Key_Insert 0xff63
#define X11Input_Key_Delete 0xffff

static inline u8 __X11KeyCode_Translate(u32 in) {
	// is this smart? there HAS to be a better way
	switch (in) {
		case X11Input_Key_LeftArrow: return Input_Key_LeftArrow;
		case X11Input_Key_UpArrow: return Input_Key_UpArrow;
		case X11Input_Key_RightArrow: return Input_Key_RightArrow;
		case X11Input_Key_DownArrow: return Input_Key_DownArrow;
		case X11Input_Key_Minus: return Input_Key_Minus;
		case X11Input_Key_Equals: return Input_Key_Equals;
		case X11Input_Key_Backspace: return Input_Key_Backspace;
		case X11Input_Key_Numpad0: return Input_Key_Numpad0;
		case X11Input_Key_Numpad1: return Input_Key_Numpad1;
		case X11Input_Key_Numpad2: return Input_Key_Numpad2;
		case X11Input_Key_Numpad3: return Input_Key_Numpad3;
		case X11Input_Key_Numpad4: return Input_Key_Numpad4;
		case X11Input_Key_Numpad5: return Input_Key_Numpad5;
		case X11Input_Key_Numpad6: return Input_Key_Numpad6;
		case X11Input_Key_Numpad7: return Input_Key_Numpad7;
		case X11Input_Key_Numpad8: return Input_Key_Numpad8;
		case X11Input_Key_Numpad9: return Input_Key_Numpad9;
		case X11Input_Key_NumpadPlus: return Input_Key_NumpadPlus;
		case X11Input_Key_NumpadMinus: return Input_Key_NumpadMinus;
		case X11Input_Key_NumpadStar: return Input_Key_NumpadStar;
		case X11Input_Key_NumpadSlash: return Input_Key_NumpadSlash;
		case X11Input_Key_NumpadPeriod: return Input_Key_NumpadPeriod;
		case X11Input_Key_LeftShift: return Input_Key_LeftShift;
		case X11Input_Key_RightShift: return Input_Key_RightShift;
		case X11Input_Key_LeftControl: return Input_Key_LeftControl;
		case X11Input_Key_RightControl: return Input_Key_RightControl;
		case X11Input_Key_LeftAlt: return Input_Key_LeftAlt;
		case X11Input_Key_RightAlt: return Input_Key_RightAlt;
		case X11Input_Key_CapsLock: return Input_Key_CapsLock;
		case X11Input_Key_ScrollLock: return Input_Key_ScrollLock;
		case X11Input_Key_NumLock: return Input_Key_NumLock;
		case X11Input_Key_Grave: return Input_Key_Grave;
		case X11Input_Key_Enter: return Input_Key_Enter;
		case X11Input_Key_Period: return Input_Key_Period;
		case X11Input_Key_Comma: return Input_Key_Comma;
		case X11Input_Key_ForwardSlash: return Input_Key_ForwardSlash;
		case X11Input_Key_BackSlash: return Input_Key_BackSlash;
		case X11Input_Key_Semicolon: return Input_Key_Semicolon;
		case X11Input_Key_Apostrophe: return Input_Key_Apostrophe;
		case X11Input_Key_OpenBracket: return Input_Key_OpenBracket;
		case X11Input_Key_CloseBracket: return Input_Key_CloseBracket;
		case X11Input_Key_Escape: return Input_Key_Escape;
		case X11Input_Key_Pause: return Input_Key_Pause;
		case X11Input_Key_F1: return Input_Key_F1;
		case X11Input_Key_F2: return Input_Key_F2;
		case X11Input_Key_F3: return Input_Key_F3;
		case X11Input_Key_F4: return Input_Key_F4;
		case X11Input_Key_F5: return Input_Key_F5;
		case X11Input_Key_F6: return Input_Key_F6;
		case X11Input_Key_F7: return Input_Key_F7;
		case X11Input_Key_F8: return Input_Key_F8;
		case X11Input_Key_F9: return Input_Key_F9;
		case X11Input_Key_F10: return Input_Key_F10;
		case X11Input_Key_F11: return Input_Key_F11;
		case X11Input_Key_F12: return Input_Key_F12;
		case X11Input_Key_PageUp: return Input_Key_PageUp;
		case X11Input_Key_PageDown: return Input_Key_PageDown;
		case X11Input_Key_End: return Input_Key_End;
		case X11Input_Key_Home: return Input_Key_Home;
		case X11Input_Key_Insert: return Input_Key_Insert;
		case X11Input_Key_Delete: return Input_Key_Delete;
	}
	return in;
}

#endif //X11_KEY_CODES_H
