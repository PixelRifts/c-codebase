/* date = July 7th 2022 6:01 pm */

#ifndef WINDOW_H
#define WINDOW_H

#include "base/mem.h"
#include "base/str.h"

typedef struct OS_Window OS_Window;
typedef void ResizeCallback(OS_Window* window, i32 w, i32 h);

struct OS_Window {
	u32 width;
	u32 height;
	string title;
	ResizeCallback* resize_callback;
	u64 v[8];
};

OS_Window* OS_WindowCreate(u32 width, u32 height, string title);
void OS_WindowShow(OS_Window* window);
b8   OS_WindowIsOpen(OS_Window* window);
void OS_PollEvents();
void OS_WindowClose(OS_Window* window);

#endif //WINDOW_H
