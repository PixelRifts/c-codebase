#include <Windows.h>
#include <Windowsx.h>
#include <stdio.h>
#include <stdlib.h>

#include "os/input.h"
#include "os/win32_window.h"

static void CALLBACK OS_PollEvents_Fiber(W32_Window* param);

static char* _classname_buffer = {0};
static u32 _window_ct = 0;
static b8 _should_close = false;
static void* _main_fibre = 0;
static void* _event_fibre = 0;

static LRESULT CALLBACK Win32Proc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
static void DefaultResizeCallback(OS_Window* _window, i32 w, i32 h);

OS_Window* OS_WindowCreate(u32 width, u32 height, string title) {
	M_Scratch scratch = scratch_get();
	
	HINSTANCE hinstance = GetModuleHandle(0);
	
	if (_window_ct == 0) {
		string prefix = str_lit("ClassOf_");
		string final = str_cat(&scratch.arena, prefix, title);
		final = str_copy(&scratch.arena, final);
		_classname_buffer = calloc(final.size + 1, sizeof(char));
		_classname_buffer = memmove(_classname_buffer, final.str, final.size + 1);
		
		WNDCLASSA wc = {
			.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
			.lpfnWndProc = Win32Proc,
			.hInstance = hinstance,
			.hCursor = LoadCursor(0, IDC_ARROW),
			.hIcon = LoadIcon(0, IDI_APPLICATION),
			.hbrBackground = 0,
			.lpszClassName = _classname_buffer,
		};
		
		if (!RegisterClassA(&wc)) {
			LogReturn((OS_Window*) 0, "Win32 Window Class Registration Failed");
		}
	}
	
	W32_Window* window = malloc(sizeof(W32_Window));
	window->width = width;
	window->height = height;
	window->title = title;
	window->resize_callback = nullptr;
	
	RECT r = {
		.right = width,
		.bottom = height,
	};
	DWORD window_style = WS_OVERLAPPEDWINDOW;
	AdjustWindowRect(&r, window_style, false);
	
	window->handle = CreateWindowExA(0,
									 _classname_buffer,
									 (const char*) title.str,
									 window_style,
									 CW_USEDEFAULT,
									 CW_USEDEFAULT,
									 r.right - r.left,
									 r.bottom - r.top,
									 0,
									 0,
									 hinstance,
									 0);
	
	
	if (_window_ct == 0) {
		_main_fibre = ConvertThreadToFiber(0);
		_event_fibre = CreateFiber(0, (LPFIBER_START_ROUTINE) OS_PollEvents_Fiber, &window);
	}
	SetWindowLongPtr(window->handle, GWLP_USERDATA, (LONG_PTR) window);
	
	if (!window->handle) {
		LogReturn((OS_Window*) 0, "Win32 Window Creation Failed");
	}
	
	scratch_return(&scratch);
	
	_window_ct++;
	
	return (OS_Window*)window;
}

void OS_WindowShow(OS_Window* _window) {
	W32_Window* window = (W32_Window*) _window;
	ShowWindow(window->handle, true);
	UpdateWindow(window->handle);
}

b8 OS_WindowIsOpen(OS_Window* _window) {
	return !_should_close;
}

void OS_WindowClose(OS_Window* _window) {
	W32_Window* window = (W32_Window*) _window;
	DestroyWindow(window->handle);
	_window_ct--;
	if (_window_ct == 0) {
		UnregisterClass(_classname_buffer, GetModuleHandle(0));
		free(_classname_buffer);
	}
	free(window);
}

static void CALLBACK OS_PollEvents_Fiber(W32_Window* param) {
	MSG _Msg = {0};
	while (true) {
		if (PeekMessage(&_Msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&_Msg);
			DispatchMessage(&_Msg);
		}
		SwitchToFiber(_main_fibre);
	}
}

void OS_PollEvents() {
	__OS_InputReset();
	SwitchToFiber(_event_fibre);
}

static void DefaultResizeCallback(OS_Window* _window, i32 w, i32 h) {
	W32_Window* window = (W32_Window*) _window;
	window->width = (u32)w;
	window->height = (u32)h;
	if (window->resize_callback) window->resize_callback(_window, w, h);
}

static LRESULT CALLBACK Win32Proc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
	LRESULT result = 0;
	
	OS_Window* os_window = (OS_Window*) GetWindowLongPtr(window, GWLP_USERDATA);
	
	switch (msg) {
		case WM_TIMER: {
			RECT r;
			GetClientRect(window, &r);
			int width = r.right - r.left;
			int height = r.bottom - r.top;
			
			DefaultResizeCallback(os_window, width, height);
			
			SwitchToFiber(_main_fibre);
		} break;
		
		case WM_ENTERSIZEMOVE: {
			SetTimer(window, 1, 1, 0);
		} break;
		
		case WM_EXITSIZEMOVE: {
			KillTimer(window, 1);
		} break;
		
		case WM_SYSKEYDOWN:
        case WM_KEYDOWN: {
			__OS_InputKeyCallback((u8)wparam, (lparam >> 30) & 0x01 ? Input_Repeat : Input_Press);
			if (os_window->key_callback) os_window->key_callback(os_window, (u8)wparam, (lparam >> 30) & 0x01 ? Input_Repeat : Input_Press);
        } break;
		
		case WM_SYSKEYUP:
        case WM_KEYUP: {
			__OS_InputKeyCallback((u8)wparam, Input_Release);
			if (os_window->key_callback) os_window->key_callback(os_window, (u8)wparam, Input_Release);
        } break;
        
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN: {
            u8 btn = msg == WM_LBUTTONDOWN
				? Input_MouseButton_Left : msg == WM_RBUTTONDOWN
				? Input_MouseButton_Right : Input_MouseButton_Middle;
            
			__OS_InputButtonCallback(btn, Input_Press);
			if (os_window->button_callback) os_window->button_callback(os_window, btn, Input_Press);
        } break;
        
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP: {
            u8 btn = msg == WM_LBUTTONUP
				? Input_MouseButton_Left : msg == WM_RBUTTONUP
				? Input_MouseButton_Right : Input_MouseButton_Middle;
			__OS_InputButtonCallback(btn, Input_Release);
			if (os_window->button_callback) os_window->button_callback(os_window, btn, Input_Release);
        } break;
        
		case WM_MOUSEMOVE: {
			__OS_InputCursorPosCallback((f32)GET_X_LPARAM(lparam), (f32)GET_Y_LPARAM(lparam));
		} break;
		
		case WM_MOUSEWHEEL: {
			__OS_InputScrollCallback(0, (f32)GET_WHEEL_DELTA_WPARAM(wparam) / 120.f);
		} break;
		
		case WM_CLOSE:
		case WM_DESTROY: {
			PostQuitMessage(0);
			_should_close = true;
		} break;
		
		default: {
			result = DefWindowProcA(window, msg, wparam, lparam);
		} break;
		
	}
	
	return result;
}
