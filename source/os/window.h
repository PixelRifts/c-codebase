/* date = July 7th 2022 6:01 pm */

#ifndef WINDOW_H
#define WINDOW_H

#include "base/mem.h"
#include "base/str.h"

typedef struct OS_Window OS_Window;
typedef void ResizeCallback(OS_Window* window, i32 w, i32 h);
typedef void KeyCallback(OS_Window* window, u8 key, i32 action);
typedef void ButtonCallback(OS_Window* window, i32 button, i32 action);

struct OS_Window {
	u32 width;
	u32 height;
	string title;
	ResizeCallback* resize_callback;
	KeyCallback* key_callback;
	ButtonCallback* button_callback;
	void* user_data;
	u64 v[8];
};

OS_Window* OS_WindowCreate(u32 width, u32 height, string title);
void OS_WindowShow(OS_Window* window);
b8   OS_WindowIsOpen(OS_Window* window);
void OS_PollEvents(void);
void OS_WindowClose(OS_Window* window);

#endif //WINDOW_H
