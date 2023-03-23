/* date = September 6th 2022 8:32 pm */

#ifndef WIN32_WINDOW_H
#define WIN32_WINDOW_H

#include "defines.h"

#if defined(BACKEND_D3D11)
#  include <d3d11.h>
#  include <dxgidebug.h>
#endif

typedef struct W32_Window {
	u32 width;
	u32 height;
	string title;
	ResizeCallback* resize_callback;
	KeyCallback* key_callback;
	ButtonCallback* button_callback;
	void* user_data;
	HWND handle;
	
#if defined(BACKEND_GL33) || defined(BACKEND_GL46)
	
	struct {
		HGLRC glrc;
		u64 v[6];
	};
	
#elif defined(BACKEND_D3D11)
	
	struct {
		IDXGISwapChain* swapchain;
		ID3D11Device* device;
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* rtv;
		IDXGIInfoQueue* dbg_queue;
		u64 v[2];
	};
	
#endif
	
} W32_Window;

#endif //WIN32_WINDOW_H
