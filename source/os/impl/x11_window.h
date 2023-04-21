/* date = April 19th 2023 10:29 am */

#ifndef X11_WINDOW_H
#define X11_WINDOW_H

#include <X11/Xlib.h>

typedef XID GLXWindow;
typedef XID GLXDrawable;
typedef struct __GLXFBConfig* GLXFBConfig;
typedef struct __GLXcontext* GLXContext;
typedef void GLXextproc(void);

typedef struct X11_Window {
	u32 width;
	u32 height;
	string title;
	ResizeCallback* resize_callback;
	KeyCallback* key_callback;
	ButtonCallback* button_callback;
	void* user_data;
	
	Display* display;
	Window handle;
#if defined(BACKEND_GL33) || defined(BACKEND_GL46)
	struct {
		GLXContext gl_context;
	};
#endif
} X11_Window;

#endif //X11_WINDOW_H
