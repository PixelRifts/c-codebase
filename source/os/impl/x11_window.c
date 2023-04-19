
#include "x11_window.h"

#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

static u32 _window_ct = 0;
static Display* _display;
static i32 _screen;
static b8 _should_close = false;

OS_Window* OS_WindowCreate(u32 width, u32 height, string title) {
	if (_window_ct == 0) {
		_display = XOpenDisplay(NULL);
		if (_display == NULL) {
			LogReturn((OS_Window*) 0, "X11 Window Display Opening Failed");
		}
		
		_screen = DefaultScreen(_display);
		_window_ct += 1;
	}
	
	X11_Window* window = malloc(sizeof(X11_Window));
	MemoryZeroStruct(window, X11_Window);
	window->width = width;
	window->height = height;
	window->title = title;
	
	window->handle = XCreateSimpleWindow(_display, RootWindow(_display, _screen), 30, 30, width, height, 0, 0, 0);
	XStoreName(_display, window->handle, (char*)title.str);
	
	return (OS_Window*) window;
}

void OS_WindowShow(OS_Window* _window) {
	X11_Window* window = (X11_Window*) _window;
	XMapWindow(_display, window->handle);
}

b8 OS_WindowIsOpen(OS_Window* _window) {
	return !_should_close;
}

void OS_PollEvents(void) {
	XEvent event;
	while(XPending(_display))
	{
		XNextEvent(_display, &event);
		if (event.type == KeyPress)
		{
			switch(XkbKeycodeToKeysym(_display, event.xkey.keycode, 0, 0))
			{
				case XK_Escape: _should_close = true;
			}
		}
	}
}

void OS_WindowClose(OS_Window* _window) {
	X11_Window* window = (X11_Window*) _window;
	XDestroyWindow(_display, window->handle);
	_window_ct -= 1;
	
	if (!_window_ct) {
		XCloseDisplay(_display);
	}
}
