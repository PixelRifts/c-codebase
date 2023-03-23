#include "defines.h"
#include "os/os.h"
#include "os/window.h"
#include "os/input.h"
#include "base/tctx.h"
#include "core/backend.h"
#include "core/resources.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


void MyResizeCallback(OS_Window* window, i32 w, i32 h) {
	R_Viewport(0, 0, w, h);
}

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	M_Arena global_arena;
	arena_init(&global_arena);
	
	U_FrameArenaInit();
	
	OS_Window* window = OS_WindowCreate(1080, 720, str_lit("This should work"));
	window->resize_callback = MyResizeCallback;
	
	B_BackendInit(window);
	OS_WindowShow(window);
	
	f32 start = 0.f; f32 end = 0.016f;
	f32 delta = 0.016f;
	
	//-
	
	R_ShaderPack s = {0};
	R_ShaderPackAllocLoad(&s, str_lit("res/test"));
	
	R_Attribute attribs[] = {
		{ str_lit("Position"), AttributeType_Float2 },
		{ str_lit("Color"), AttributeType_Float4 },
	};
	
	R_Pipeline p = {0};
	R_PipelineAlloc(&p, InputAssembly_Triangles, attribs, ArrayCount(attribs), &s, BlendMode_Alpha);
	
	typedef struct vertex {
		vec2 pos;
		vec4 color;
	} vertex;
	
	vertex data[] = {
		{ v2( 0.f,  .5f), v4(.8f, .2f, .3f, 1.f) },
		{ v2( .5f, -.5f), v4(.2f, .3f, .8f, 1.f) },
		{ v2(-.5f, -.5f), v4(.3f, .8f, .2f, 1.f) },
	};
	
	R_Buffer b = {0};
	R_BufferAlloc(&b, BufferFlag_Dynamic | BufferFlag_Type_Vertex, sizeof(vertex));
	R_BufferData(&b, sizeof(data), nullptr);
	R_BufferUpdate(&b, 0, sizeof(data), data);
	
	R_PipelineAddBuffer(&p, &b, ArrayCount(attribs));
	
	u32 color = 0xb942f5ff;
	vec4 vc = color_code_to_vec4(color);
	
	while (OS_WindowIsOpen(window)) {
		delta = end - start;
		start = OS_TimeMicrosecondsNow();
		
		U_ResetFrameArena();
		OS_PollEvents();
		
		R_ClearColor(vc.x, vc.y, vc.z, vc.w);
		R_Clear(BufferMask_Color);
		
		R_PipelineBind(&p);
		R_Draw(&p, 0, 3);
		
		B_BackendSwapchainNext(window);
		end = OS_TimeMicrosecondsNow();
	}
	
	R_BufferFree(&b);
	R_PipelineFree(&p);
	R_ShaderPackFree(&s);
	
	//-
	
	
	B_BackendFree(window);
	
	OS_WindowClose(window);
	
	U_FrameArenaFree();
	arena_free(&global_arena);
	tctx_free(&context);
}
