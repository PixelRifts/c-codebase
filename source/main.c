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
	//- End Graphics 
	if (!glClear) { LogError("Clear not found"); flush; }
	
	while (OS_WindowIsOpen(&window)) {
		OS_PollEvents();
		
		if (OS_InputKeyPressed('A')) {
			LogError("We never press the A key :(");
			flush;
		}
		
		if (OS_InputKeyPressed('B')) {
			if (OS_InputKey(Input_Key_Shift)) {
				Log("B with shift is even better :D");
				flush;
			} else {
				Log("B is good though :)");
				flush;
			}
		}
		
		glClear(GL_COLOR_BUFFER_BIT);
		R_PipelineBind(&vin);
		glDrawArrays(GL_TRIANGLES, 0, 3);
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

#if 0
int graphics_example() {
	glfwInit();
	B_BackendInitWindow();
	GLFWwindow* w = glfwCreateWindow(1080, 720, "", nullptr, nullptr);
	B_BackendInit(w);
	
	//- Graphics 
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
	//- End Graphics 
	
	while (!glfwWindowShouldClose(w)) {
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);
		
		R_PipelineBind(&vin);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		
		glfwSwapBuffers(w);
	}
	
	//- Graphics 
	R_BufferFree(&buf);
	R_PipelineFree(&vin);
	R_ShaderPackFree(&program);
	//- End Graphics 
	
	glfwDestroyWindow(w);
	glfwTerminate();
}
#endif