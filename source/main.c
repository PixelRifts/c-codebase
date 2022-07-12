#include <glad/glad.h>

#include "defines.h"
#include "os/os.h"
#include "base/base.h"
#include "core/backend.h"
#include "core/resources.h"

#include <GLFW/glfw3.h>
#include <assert.h>

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
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
	
	tctx_free(&context);
}
