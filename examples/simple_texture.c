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
	R_ShaderPackAllocLoad(&s, str_lit("res/shaders/simple_texture"));
	
	
	
	R_Attribute attribs[] = {
		{ str_lit("Position"), AttributeType_Float2 },
		{ str_lit("TexCoord"), AttributeType_Float2 },
	};
	
	R_Pipeline p = {0};
	R_PipelineAlloc(&p, InputAssembly_Triangles, attribs, ArrayCount(attribs), &s, BlendMode_Alpha);
	
	
	
	typedef struct vertex {
		vec2 pos;
		vec2 tex_coord;
	} vertex;
	vertex data[] = {
		{ v2(-.5f, -.5f), v2(0.f, 0.f) },
		{ v2( .5f, -.5f), v2(1.f, 0.f) },
		{ v2( .5f,  .5f), v2(1.f, 1.f) },
		{ v2(-.5f, -.5f), v2(0.f, 0.f) },
		{ v2( .5f,  .5f), v2(1.f, 1.f) },
		{ v2(-.5f,  .5f), v2(0.f, 1.f) },
	};
	R_Buffer b = {0};
	R_BufferAlloc(&b, BufferFlag_Dynamic | BufferFlag_Type_Vertex, sizeof(vertex));
	R_BufferData(&b, sizeof(data), nullptr);
	R_BufferUpdate(&b, 0, sizeof(data), data);
	R_PipelineAddBuffer(&p, &b, ArrayCount(attribs));
	
	
	
	string_array u_layout = {0};
	string_array_add(&u_layout, str_lit("val"));
	string_array_add(&u_layout, str_lit("color"));
	
	R_UniformBuffer u = {0};
	R_UniformBufferAlloc(&u, str_lit("TESTBUFFER"), u_layout, &s, ShaderType_Fragment);
	R_UniformBufferSetVec4(&u, str_lit("color"), Color_Blue);
	R_PipelineAddUniformBuffer(&p, &u);
	
	string_array_free(&u_layout);
	
	
	R_Texture2D t = {0};
	R_Texture2DAllocLoad(&t, str_lit("res/pepper.png"), TextureResize_Linear, TextureResize_Linear, TextureWrap_Repeat, TextureWrap_Repeat, TextureMutability_Immutable, TextureUsage_ShaderResource);
	
	
	R_ClearColor(.2f, .2f, .2f, 1.f);
	
	while (OS_WindowIsOpen(window)) {
		OS_PollEvents();
		U_ResetFrameArena();
		
		B_BackendSelectRenderWindow(window);
		R_Clear(BufferMask_Color);
		
		R_Texture2DBindTo(&t, 0, ShaderType_Fragment);
		R_PipelineBind(&p);
		
		R_Draw(&p, 0, 6);
		B_BackendSwapchainNext(window);
	}
	
	R_Texture2DFree(&t);
	R_UniformBufferFree(&u);
	R_BufferFree(&b);
	R_PipelineFree(&p);
	R_ShaderPackFree(&s);
	
	//- 
	
	B_BackendFree(window);
	
	OS_WindowClose(window);
	U_FrameArenaFree();
	tctx_free(&context);
}

