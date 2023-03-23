#include "defines.h"
#include "os/os.h"
#include "os/window.h"
#include "base/tctx.h"
#include "core/backend.h"
#include "core/resources.h"

void MyResizeCallback(OS_Window* window, i32 w, i32 h) {
	R_Viewport(0, 0, w, h);
}

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	U_FrameArenaInit();
	
	OS_Window* window = OS_WindowCreate(1080, 720, str_lit("This should work"));
	window->resize_callback = MyResizeCallback;
	B_BackendInit(window);
	OS_WindowShow(window);
	
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
	R_ClearColor(.2f, .2f, .2f, 1.f);
	
	while (OS_WindowIsOpen(window)) {
		OS_PollEvents();
		
		B_BackendSelectRenderWindow(window);
		R_Clear(BufferMask_Color);
		
		R_PipelineBind(&p);
		R_Draw(&p, 0, 3);
		B_BackendSwapchainNext(window);
	}
	
	
	R_BufferFree(&b);
	R_PipelineFree(&p);
	R_ShaderPackFree(&s);
	
	B_BackendFree(window);
	
	OS_WindowClose(window);
	U_FrameArenaFree();
	tctx_free(&context);
}
