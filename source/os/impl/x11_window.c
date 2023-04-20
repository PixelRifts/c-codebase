#include "x11_window.h"
#include <X11/XKBlib.h>

#include "base/ds.h"
#include "os/input.h"


static u32 _window_ct = 0;
static Display* _display;
static i32 _screen;
static b8 _should_close = false;

// Sadly for event callbacks we need to have a hash table.
typedef X11_Window* X11_WindowHandle;
HashTable_Prototype(Window, X11_WindowHandle);

b8 X11WindowHandleIsNull(Window w) { return w == 0; }
b8 X11WindowHandlesAreEqual(Window a, Window b) { return a == b; }
u64 X11WindowHash(Window w) { return (u64)w; }
b8 OSWindowHandleIsNull(X11_WindowHandle w) { return (u64)w == 0; }
b8 OSWindowHandleIsTomb(X11_WindowHandle w) { return (u64)w == 69; }
HashTable_Impl(Window, X11_WindowHandle, X11WindowHandleIsNull, X11WindowHandlesAreEqual,
			   X11WindowHash, (X11_WindowHandle)69, OSWindowHandleIsNull, OSWindowHandleIsTomb);

hash_table(Window, X11_WindowHandle) _window_map;

OS_Window* OS_WindowCreate(u32 width, u32 height, string title) {
	if (_window_ct == 0) {
		_display = XOpenDisplay(NULL);
		if (_display == NULL) {
			LogReturn((OS_Window*) 0, "X11 Window Display Opening Failed");
		}
		XAutoRepeatOn(_display);
		_screen = DefaultScreen(_display);
		_window_ct += 1;
		hash_table_init(Window, X11_WindowHandle, &_window_map);
	}
	
	X11_Window* window = malloc(sizeof(X11_Window));
	MemoryZeroStruct(window, X11_Window);
	window->width = width;
	window->height = height;
	window->title = title;
	
	window->handle =
		XCreateSimpleWindow(_display, RootWindow(_display, _screen), 30, 30, width, height, 0, 0, 0);
	XSelectInput(_display, window->handle,
				 KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask |
				 PointerMotionMask | ResizeRedirectMask);
	XStoreName(_display, window->handle, (char*)title.str);
	
	hash_table_set(Window, X11_WindowHandle, &_window_map, window->handle, window);
	
	return (OS_Window*) window;
}

void OS_WindowShow(OS_Window* _window) {
	X11_Window* window = (X11_Window*) _window;
	XMapWindow(_display, window->handle);
}

// NOTE(voxel): for now all windows share one state for being open or closed.
//              this should probably change but it's fine for now since I'm not doing
//              multiwindow applications
// This is one specific place where a setter has to be used. It could mean different things in
// different OS contexts since should_close is a static variable currently.
// definitely should make this a per window thing but till then....
void OS_WindowSetOpen(b8 open) {
	_should_close = !open;
}

b8 OS_WindowIsOpen(OS_Window* _window) {
	return !_should_close;
}

// A full hash table fetch for EVERY SINGLE GODDAMN EVENT?
// Maybe darrays are just going be better for this usecase

void OS_PollEvents(void) {
	__OS_InputReset();
	XEvent event;
	while (XPending(_display)) {
		XNextEvent(_display, &event);
		if (event.type == KeyPress) {
			KeySym k = XkbKeycodeToKeysym(_display, event.xkey.keycode, 0, 0);
			
			u8 translated = __X11KeyCode_Translate(k);
			X11_Window* window;
			hash_table_get(Window, X11_WindowHandle, &_window_map, event.xkey.window, &window);
			b8 did_repeat = __OS_InputKeyCallbackCheckRepeat(translated, Input_Press);
			
			if (window->key_callback) window->key_callback((OS_Window*) window, translated, did_repeat ? Input_Repeat : Input_Press);
		} else if (event.type == KeyRelease) {
			KeySym k = XkbKeycodeToKeysym(_display, event.xkey.keycode, 0, 0);
			
			if (XEventsQueued(_display, QueuedAfterReading)) {
				XEvent nev;
				XPeekEvent(_display, &nev);
				if (!(nev.type == KeyPress && nev.xkey.time == event.xkey.time &&
					  nev.xkey.keycode == event.xkey.keycode)) {
					u8 translated = __X11KeyCode_Translate(k);
					X11_Window* window;
					hash_table_get(Window, X11_WindowHandle, &_window_map, event.xkey.window,
								   &window);
					__OS_InputKeyCallbackCheckRepeat(translated, Input_Release);
					if (window->key_callback) window->key_callback((OS_Window*) window, translated, Input_Release);
				}
			} else {
				
				u8 translated = __X11KeyCode_Translate(k);
				X11_Window* window;
				hash_table_get(Window, X11_WindowHandle, &_window_map, event.xkey.window, &window);
				__OS_InputKeyCallbackCheckRepeat(translated, Input_Release);
				if (window->key_callback) window->key_callback((OS_Window*) window, translated, Input_Release);
			}
		} else if (event.type == ButtonPress) {
			if (event.xbutton.button != 4 && event.xbutton.button != 5) {
				u8 translated = event.xbutton.button - 1;
				X11_Window* window;
				hash_table_get(Window, X11_WindowHandle, &_window_map, event.xbutton.window, &window);
				__OS_InputButtonCallback(translated, Input_Press);
				if (window->button_callback) window->button_callback((OS_Window*) window, translated, Input_Press);
			} else {
				f32 translated_delta = event.xbutton.button == 4 ? 1.f : -1.f;
				__OS_InputScrollCallback(0, translated_delta);
			}
		} else if (event.type == ButtonRelease) {
			if (event.xbutton.button != 4 && event.xbutton.button != 5) {
				u8 translated = event.xbutton.button - 1;
				X11_Window* window;
				hash_table_get(Window, X11_WindowHandle, &_window_map, event.xbutton.window, &window);
				__OS_InputButtonCallback(translated, Input_Release);
				if (window->button_callback) window->button_callback((OS_Window*) window, translated, Input_Release);
			}
		} else if (event.type == MotionNotify) {
			__OS_InputCursorPosCallback((f32)event.xmotion.x, (f32)event.xmotion.y);
		}
	}
}

void OS_WindowClose(OS_Window* _window) {
	X11_Window* window = (X11_Window*) _window;
	XDestroyWindow(_display, window->handle);
	_window_ct -= 1;
	
	if (!_window_ct) {
		XCloseDisplay(_display);
		hash_table_free(Window, X11_WindowHandle, &_window_map);
	}
}
