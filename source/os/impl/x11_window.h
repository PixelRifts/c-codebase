/* date = April 19th 2023 10:29 am */

#ifndef X11_WINDOW_H
#define X11_WINDOW_H

#include <X11/Xlib.h>

#if defined(BACKEND_GL33) || defined(BACKEND_GL46)
//#  include <GL/glx.h>
#endif

typedef struct X11_Window {
	u32 width;
	u32 height;
	string title;
	ResizeCallback* resize_callback;
	KeyCallback* key_callback;
	ButtonCallback* button_callback;
	void* user_data;
	
	Window handle;
	b32 should_close;
#if defined(BACKEND_GL33) || defined(BACKEND_GL46)
	struct {
		//XVisualInfo* visual_info;
		//GLXContext gl_context;
	};
#endif
} X11_Window;

#endif //X11_WINDOW_H
