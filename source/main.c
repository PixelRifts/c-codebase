#include "defines.h"
#include "os/os.h"
#include "os/window.h"
#include "os/input.h"
#include "base/tctx.h"
#include "core/backend.h"
#include "core/resources.h"

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	OS_Window* window = OS_WindowCreate(1080, 720, str_lit("This should work"));
	B_BackendInit(window);
	OS_WindowShow(window);
	
	OS_Window* window2 = OS_WindowCreate(1080, 720, str_lit("This should work too"));
	B_BackendInitShared(window2, window);
	OS_WindowShow(window2);
	
	R_ShaderPack program = {0};
	R_ShaderPackAllocLoad(&program, str_lit("res/test"));
	
	R_Attribute attribs[] = { Attribute_Float2, Attribute_Float4 };
	
	R_Pipeline vin = {0};
	R_PipelineAlloc(&vin, InputAssembly_Triangles, attribs, ArrayCount(attribs), &program);
	
	float verts[] = {
		-.5f, -.5f, 0.8f, 0.2f, 0.3f, 1.f,
		.5f, -.5f,  0.3f, 0.8f, 0.2f, 1.f,
		.0f, .5f,   0.2f, 0.3f, 0.8f, 1.f,
	};
	R_Buffer buf = {0};
	R_BufferAlloc(&buf, BufferFlag_Type_Vertex);
	R_BufferData(&buf, sizeof(verts), verts);
	
	R_PipelineAddBuffer(&vin, &buf, 2);
	
	while (OS_WindowIsOpen(window)) {
		OS_PollEvents();
		
		B_BackendSelectRenderWindow(window);
		R_Viewport(0, 0, window->width, window->height);
		R_Clear(BufferMask_Color);
		R_PipelineBind(&vin);
		R_Draw(&vin, 0, 3);
		B_BackendSwapchainNext(window);
		
		B_BackendSelectRenderWindow(window2);
		R_Viewport(0, 0, window2->width, window2->height);
		R_Clear(BufferMask_Color);
		R_PipelineBind(&vin);
		R_Draw(&vin, 0, 3);
		B_BackendSwapchainNext(window2);
	}
	
	R_BufferFree(&buf);
	R_PipelineFree(&vin);
	R_ShaderPackFree(&program);
	
	B_BackendFree(window2);
	B_BackendFree(window);
	
	OS_WindowClose(window);
	
	tctx_free(&context);
}
