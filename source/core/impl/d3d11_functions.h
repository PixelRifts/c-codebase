/* date = March 22nd 2023 0:00 pm */

#ifndef D3D11_FUNCTIONS_H
#define D3D11_FUNCTIONS_H

#include "defines.h"

#if defined(_DEBUG)
#define CHECK_HR(hrcall) Statement(\
u64 start = 0;\
if (!IsDebuggerPresent())\
start = IDXGIInfoQueue_GetNumStoredMessages(s_wnd->dbg_queue, DXGI_DEBUG_ALL);\
\
if (FAILED(hr = hrcall)) {\
if (IsDebuggerPresent()) {\
LogError("[D3D11 Backend] Function call failed: %s", #hrcall);\
LogError("[D3D11 Backend] Check Debugger Debug Output");\
} else {\
u64 end =\
IDXGIInfoQueue_GetNumStoredMessages(s_wnd->dbg_queue,\
DXGI_DEBUG_ALL);\
\
M_Scratch scratch = scratch_get();\
for (u64 i = start; i < end; i++) {\
SIZE_T msg_size = 0;\
IDXGIInfoQueue_GetMessage(s_wnd->dbg_queue,\
DXGI_DEBUG_ALL, i, nullptr,\
&msg_size);\
DXGI_INFO_QUEUE_MESSAGE* msg = arena_alloc(&scratch.arena, msg_size);\
IDXGIInfoQueue_GetMessage(s_wnd->dbg_queue,\
DXGI_DEBUG_ALL, i, msg,\
&msg_size);\
LogError("[D3D11 Backend] %s", msg->pDescription);\
scratch_reset(&scratch);\
}\
scratch_return(&scratch);\
}\
}\
)
#else
#define CHECK_HR(hrcall) Statement(\
if (FAILED(hr = hrcall)) {\
LogError("[D3D11 Backend] Function call failed: %s", #hrcall);\
LogError("[D3D11 Backend] Enable Debug mode by defining _DEBUG to know more");\
}\
)
#endif

#define SAFE_RELEASE(release, obj) if (obj) release##_Release(obj)



#if defined(BACKEND_D3D11)
#  define DXGI_FUNCTIONS \
X(DXGIGetDebugInterface, HRESULT, (REFIID riid, void **ppDebug))
#else
#  define DXGI_FUNCTIONS
#endif


//#define X(Name, Return, Args)\
//typedef Return DXGI_##Name##_Func Args;\
//extern DXGI_##Name##_Func* Name;
//DXGI_FUNCTIONS
//#undef X


#endif //D3D11_FUNCTIONS_H
