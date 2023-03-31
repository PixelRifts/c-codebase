#define D3D11_NO_HELPERS
#define CINTERFACE
#define COBJMACROS
#include <d3d11.h>
#include <dxgi.h>

#include "defines.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "os/impl/win32_window.h"

#include "d3d11_functions.h"

// Talking with resources layer
void __SetCurrentWindow(W32_Window* window);
void __LoadDXGIFunctions(void);
void R_Viewport(i32 x, i32 y, i32 w, i32 h);

typedef HRESULT W32_D3D11CreateDeviceAndSwapChain(IDXGIAdapter* pAdapter, D3D_DRIVER_TYPE DriverType,
												  HMODULE Software, UINT Flags,
												  const D3D_FEATURE_LEVEL* pFeatureLevels,
												  UINT FeatureLevels, UINT SDKVersion,
												  const DXGI_SWAP_CHAIN_DESC* pSwapChainDesc,
												  IDXGISwapChain** ppSwapChain,
												  ID3D11Device** ppDevice,
												  D3D_FEATURE_LEVEL* pFeatureLevel,
												  ID3D11DeviceContext** ppImmediateContext);

typedef HRESULT W32_DXGIGetDebugInterface(REFIID riid, void **ppDebug);


static HMODULE d3d11_module;
static HMODULE dxgidebug_module;
static W32_D3D11CreateDeviceAndSwapChain* v_d3d11CreateDeviceAndSwapchain;
static W32_DXGIGetDebugInterface* v_dxgiGetDebugInterface;

void B_BackendInit(OS_Window* _window) {
	W32_Window* window = (W32_Window*)_window;
	
	if (!d3d11_module) {
		d3d11_module = LoadLibraryA("D3D11.dll");
		
		v_d3d11CreateDeviceAndSwapchain = (W32_D3D11CreateDeviceAndSwapChain*)
			GetProcAddress(d3d11_module, "D3D11CreateDeviceAndSwapChain");
	}
	if (!dxgidebug_module) {
		dxgidebug_module = LoadLibraryA("dxgidebug.dll");
		
		v_dxgiGetDebugInterface = (W32_DXGIGetDebugInterface*)
			GetProcAddress(dxgidebug_module, "DXGIGetDebugInterface");
	}
	
	DXGI_SWAP_CHAIN_DESC swapchain_desc = {0};
	swapchain_desc.BufferDesc.Width = 0;
	swapchain_desc.BufferDesc.Height = 0;
	swapchain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapchain_desc.BufferDesc.RefreshRate.Numerator = 0;
	swapchain_desc.BufferDesc.RefreshRate.Denominator = 0;
	swapchain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapchain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapchain_desc.SampleDesc.Count = 1;
	swapchain_desc.SampleDesc.Quality = 0;
	swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchain_desc.BufferCount = 1;
	swapchain_desc.OutputWindow = window->handle;
	swapchain_desc.Windowed = true;
	swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapchain_desc.Flags = 0;
	
	HRESULT hr;
	
	u32 flags = 0;
#if defined(_DEBUG)
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	
	if (FAILED(hr = v_d3d11CreateDeviceAndSwapchain(nullptr,
													D3D_DRIVER_TYPE_HARDWARE,
													nullptr,
													flags,
													nullptr,
													0,
													D3D11_SDK_VERSION,
													&swapchain_desc,
													&window->swapchain,
													&window->device,
													nullptr,
													&window->context))) {
		LogError("[D3D11 Backend] Create Device and Swapchain Failed");
	}
	
#if defined(_DEBUG)
	if (IsDebuggerPresent()) {
		ID3D11InfoQueue* info;
		ID3D11Device_QueryInterface(window->device, &IID_ID3D11InfoQueue, (void**)&info);
		ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		ID3D11InfoQueue_SetBreakOnSeverity(info, D3D11_MESSAGE_SEVERITY_ERROR, TRUE);
		ID3D11InfoQueue_Release(info);
	} else {
		if (FAILED(v_dxgiGetDebugInterface(&IID_IDXGIInfoQueue, (void**)&window->dbg_queue))) {
			LogError("[D3D11 Backend] Couldn't get the Debug Interfrace from DXGI");
		}
	}
#endif
	
	ID3D11Resource* back_buffer_resource = nullptr;
	IDXGISwapChain_GetBuffer(window->swapchain, 0, &IID_ID3D11Resource, (void**) &back_buffer_resource);
	ID3D11Device_CreateRenderTargetView(window->device, back_buffer_resource, nullptr, &window->rtv);
	ID3D11Resource_Release(back_buffer_resource);
	
	__LoadDXGIFunctions();
	
	__SetCurrentWindow(window);
	R_Viewport(0, 0, window->width, window->height);
	
	ID3D11RasterizerState* rstate;
	D3D11_RASTERIZER_DESC rdesc = {0};
	rdesc.FillMode = D3D11_FILL_SOLID;
	rdesc.CullMode = D3D11_CULL_BACK;
	rdesc.FrontCounterClockwise = true;
	rdesc.DepthClipEnable = false;
	rdesc.ScissorEnable = false;
	rdesc.MultisampleEnable = false;
	rdesc.AntialiasedLineEnable = true; // Uh... probably right?
	
	ID3D11Device_CreateRasterizerState(window->device, &rdesc, &rstate);
	ID3D11DeviceContext_RSSetState(window->context, rstate);
	ID3D11RasterizerState_Release(rstate);
}

void B_BackendInitShared(OS_Window* window, OS_Window* share) {
	// TODO(voxel): 
}

void B_BackendSelectRenderWindow(OS_Window* _window) {
	W32_Window* window = (W32_Window*)_window;
	// TODO(voxel): OMSetRenderTarget here
	__SetCurrentWindow(window);
}

void B_BackendSwapchainNext(OS_Window* _window) {
	W32_Window* window = (W32_Window*)_window;
	
	// NOTE(voxel): sync interval (2nd param) is set to 0 for uncapped fps
	IDXGISwapChain_Present(window->swapchain, 0, 0);
}

void B_BackendFree(OS_Window* _window) {
	W32_Window* window = (W32_Window*)_window;
	
	SAFE_RELEASE(IDXGIInfoQueue, window->dbg_queue);
	SAFE_RELEASE(ID3D11RenderTargetView, window->rtv);
	SAFE_RELEASE(ID3D11DeviceContext, window->context);
	SAFE_RELEASE(IDXGISwapChain, window->swapchain);
	SAFE_RELEASE(ID3D11Device, window->device);
}
