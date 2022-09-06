/* date = September 6th 2022 8:32 pm */

#ifndef WIN32_WINDOW_H
#define WIN32_WINDOW_H

typedef struct W32_Window {
	u32 width;
	u32 height;
	string title;
	ResizeCallback* resize_callback;
	KeyCallback* key_callback;
	ButtonCallback* button_callback;
	HWND handle;
	HGLRC glrc;
	u64 v[6];
} W32_Window;

#endif //WIN32_WINDOW_H
