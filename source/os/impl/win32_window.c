#include <Windows.h>
#include <stdio.h>

typedef struct W32_Window {
	u32 width;
	u32 height;
	string title;
	HWND handle;
} W32_Window;

MSG _Msg;

void Test() {
	printf("sizeof(HWND) = %llu\n", sizeof(HWND));
	printf("sizeof(W32_Window) = %llu\n", sizeof(W32_Window));
	printf("sizeof(OS_Window) = %llu\n", sizeof(OS_Window));
	flush;
}

static LRESULT CALLBACK Win32Proc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam);
OS_Window OS_WindowCreate(u32 width, u32 height, string title) {
	M_Scratch scratch = scratch_get();
	
	string class_name = str_cat(&scratch.arena, str_lit("ClassOf_"), title);
	
	HINSTANCE hinstance = GetModuleHandle(0);
	
	WNDCLASSA wc = {
        .style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        .lpfnWndProc = Win32Proc,
        .hInstance = hinstance,
        .hCursor = LoadCursor(0, IDC_ARROW),
		.hIcon = LoadIcon(0, IDI_APPLICATION),
        .hbrBackground = 0,
        .lpszClassName = (const char*) class_name.str,
    };
    
	if (!RegisterClassA(&wc)) {
		LogReturn((OS_Window) {0}, "Win32 Window Class Registration Failed");
	}
	
	W32_Window window = {0};
	window.width = width;
	window.height = height;
	window.title = title;
	
	RECT r = {
        .right = width,
        .bottom = height,
    };
    DWORD window_style = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&r, window_style, false);
    
	window.handle = CreateWindowExA(0,
									wc.lpszClassName,
									"OpenGL",
									window_style,
									CW_USEDEFAULT,
									CW_USEDEFAULT,
									r.right - r.left,
									r.bottom - r.top,
									0,
									0,
									hinstance,
									0);
    if (!window.handle) {
		LogReturn((OS_Window) {0}, "Win32 Window Creation Failed");
	}
	
	scratch_return(&scratch);
	
	return *((OS_Window*)&window);
}

void OS_WindowShow(OS_Window* _window) {
	W32_Window* window = (W32_Window*) _window;
	ShowWindow(window->handle, true);
	UpdateWindow(window->handle);
}

b8 OS_WindowIsOpen(OS_Window* _window) {
	return GetMessage(&_Msg, NULL, 0, 0);
}

void OS_PollEvents() {
	TranslateMessage(&_Msg);
	DispatchMessage(&_Msg);
}

static LRESULT CALLBACK Win32Proc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
    LRESULT result = 0;
    
    switch (msg) {
        case WM_CLOSE:
        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;
        
        default: {
            result = DefWindowProcA(window, msg, wparam, lparam);
        } break;
    }
    
    return result;
}

