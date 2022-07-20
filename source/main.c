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
	
	OS_Window window = OS_WindowCreate(1080, 720, str_lit("This should work"));
	B_BackendInit(&window);
	OS_WindowShow(&window);
	
	//- Start Graphics 
	B_BackendSelectRenderWindow(&window);
	R_ShaderPack program = {0};
	R_ShaderPackAllocLoad(&program, str_lit("res/test"));
	R_ShaderPackUploadInt(&program, str_lit("u_texture"), 0);
	
	R_Texture2D texture = {0};
	R_Texture2DAllocLoad(&texture, str_lit("res/pepper.png"), TextureResize_Linear, TextureResize_Linear, TextureWrap_Repeat, TextureWrap_Repeat);
	R_Texture2DBindTo(&texture, 0);
	
	R_Pipeline vin = {0};
	R_Attribute attribs[] = { Attribute_Float2, Attribute_Float2 };
	R_PipelineAlloc(&vin, InputAssembly_Triangles, attribs, ArrayCount(attribs), &program);
	
	float verts[] = {
		-.5f, -.5f, 0.f, 0.f,
		.5f, -.5f,  1.f, 0.f,
		.5f, .5f,   1.f, 1.f,
		-.5f, -.5f, 0.f, 0.f,
		.5f, .5f,   1.f, 1.f,
		-.5f, .5f,  0.f, 1.f,
	};
	R_Buffer buf = {0};
	R_BufferAlloc(&buf, BufferFlag_Type_Vertex);
	R_BufferData(&buf, sizeof(verts), verts);
	
	R_PipelineAddBuffer(&vin, &buf, 2);
	//- End Graphics 
	
	while (OS_WindowIsOpen(&window)) {
		OS_PollEvents();
		
		R_Clear(BufferMask_Color);
		R_PipelineBind(&vin);
		R_Draw(&vin, 0, 6);
		B_BackendSwapchainNext(&window);
	}
	
	//- Start Graphics 
	R_Texture2DFree(&texture);
	R_BufferFree(&buf);
	R_PipelineFree(&vin);
	R_ShaderPackFree(&program);
	//- End Graphics 
	
	B_BackendFree(&window);
	OS_WindowClose(&window);
	
	tctx_free(&context);
}
