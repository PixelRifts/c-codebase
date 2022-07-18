#include "defines.h"
#include "os/os.h"
#include "os/window.h"
#include "os/input.h"
#include "base/tctx.h"
#include "core/backend.h"
#include "core/resources.h"

// TODO(voxel): Remove this
#include "core/impl/gl_functions.h"

#include <assert.h>

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	Test();
	
	OS_Window window = OS_WindowCreate(1080, 720, str_lit("This should work"));
	B_BackendInit(&window);
	OS_WindowShow(&window);
	
	//- Graphics 
	B_BackendSelectRenderWindow(&window);
	R_ShaderPack program = {0};
	R_ShaderPackAllocLoad(&program, str_lit("res/test"));
	
	R_Attribute attribs[] = { Attribute_Float2, Attribute_Float4 };
	
	R_Pipeline vin = {0};
	R_PipelineAlloc(&vin, InputAssembly_Triangles, attribs, &program, ArrayCount(attribs));
	
	float verts[] = {
		-.5f, -.5f, 0.8f, 0.2f, 0.3f, 1.f,
		.5f, -.5f,  0.3f, 0.8f, 0.2f, 1.f,
		.0f, .5f,   0.2f, 0.3f, 0.8f, 1.f,
	};
	R_Buffer buf = {0};
	R_BufferAlloc(&buf, BufferFlag_Type_Vertex);
	R_BufferData(&buf, sizeof(verts), verts);
	
	R_PipelineAddBuffer(&vin, &buf, 2);
	
	R_CommandBuffer cb = {0};
	R_CommandBufferAlloc(&cb);
	
	// Record Command Buffer
	R_CommandBufferStartRecording(&cb);
	
	R_CommandBufferClearScreen(&cb, BufferMask_Color);
	R_CommandBufferPipelineBind(&cb, &vin);
	R_CommandBufferDraw(&cb, 0, 3);
	
	R_CommandBufferEndRecording(&cb);
	
	//- End Graphics 
	
	while (OS_WindowIsOpen(&window)) {
		OS_PollEvents();
		
		R_CommandBufferExecute(&cb);
		B_BackendSwapchainNext(&window);
	}
	
	//- Graphics 
	R_BufferFree(&buf);
	R_PipelineFree(&vin);
	R_ShaderPackFree(&program);
	//- End Graphics 
	
	B_BackendFree(&window);
	OS_WindowClose(&window);
	
	tctx_free(&context);
}
