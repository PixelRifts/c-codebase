
#include "gl_functions.h"
#include "os/impl/x11_window.h"
#include "base/log.h"
#include <dlfcn.h>

// TODO(voxel): 
// TODO(voxel): Maybe switch to egl because apparently it's just better :pain:
// TODO(voxel): Thanks Martins
// TODO(voxel): 

typedef i64 long_func();
typedef long_func* loader_func(const char* name);
void __LoadGLFunctions(loader_func* load_proc, loader_func* fallback);

#define GLX_VENDOR 1
#define GLX_RGBA_BIT 0x00000001
#define GLX_WINDOW_BIT 0x00000001
#define GLX_DRAWABLE_TYPE 0x8010
#define GLX_RENDER_TYPE 0x8011
#define GLX_RGBA_TYPE 0x8014
#define GLX_DOUBLEBUFFER 5
#define GLX_STEREO 6
#define GLX_AUX_BUFFERS 7
#define GLX_RED_SIZE 8
#define GLX_GREEN_SIZE 9
#define GLX_BLUE_SIZE 10
#define GLX_ALPHA_SIZE 11
#define GLX_DEPTH_SIZE 12
#define GLX_STENCIL_SIZE 13
#define GLX_ACCUM_RED_SIZE 14
#define GLX_ACCUM_GREEN_SIZE 15
#define GLX_ACCUM_BLUE_SIZE 16
#define GLX_ACCUM_ALPHA_SIZE 17
#define GLX_SAMPLES 0x186a1
#define GLX_VISUAL_ID 0x800b
#define GLX_X_RENDERABLE 0x8012
#define GLX_X_VISUAL_TYPE 0x22
#define GLX_TRUE_COLOR 0x8002

#define GLX_FRAMEBUFFER_SRGB_CAPABLE_ARB 0x20b2
#define GLX_CONTEXT_DEBUG_BIT_ARB 0x00000001
#define GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB 0x00000002
#define GLX_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define GLX_CONTEXT_PROFILE_MASK_ARB 0x9126
#define GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB 0x00000002
#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
#define GLX_CONTEXT_FLAGS_ARB 0x2094
#define GLX_CONTEXT_ES2_PROFILE_BIT_EXT 0x00000004
#define GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB 0x00000004
#define GLX_LOSE_CONTEXT_ON_RESET_ARB 0x8252
#define GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB 0x8256
#define GLX_NO_RESET_NOTIFICATION_ARB 0x8261
#define GLX_CONTEXT_RELEASE_BEHAVIOR_ARB 0x2097
#define GLX_CONTEXT_RELEASE_BEHAVIOR_NONE_ARB 0
#define GLX_CONTEXT_RELEASE_BEHAVIOR_FLUSH_ARB 0x2098
#define GLX_CONTEXT_OPENGL_NO_ERROR_ARB 0x31b3

void
MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void *userParam) {
	LogError("[GL46 Backend] OpenGL Error: %s", message);
}


//~ Fn types and loaded pointers

static void* opengl_module = 0;

//- These functions are loaded using standard linux so loading 
typedef GLXextproc* Linux_glXGetProcAddress(u8* procName);
typedef Bool Linux_glXQueryVersion(Display* display, int* major, int* minor);
typedef GLXFBConfig* Linux_glXChooseFBConfig(Display* display, int screen, const int* attrib_list, int* nelements);

static Linux_glXGetProcAddress* v_glXGetProcAddress = 0;
static Linux_glXQueryVersion* v_glXQueryVersion = 0;
static Linux_glXChooseFBConfig* v_glXChooseFBConfig = 0;

//- These functions are loaded using glXGetProcAddress
typedef GLXContext
Linux_glXCreateContextAttribsARBProc(Display*, GLXFBConfig, GLXContext, Bool, const int*);
typedef Bool Linux_glXMakeCurrent(Display*, GLXDrawable, GLXContext);
typedef void Linux_glXSwapBuffers(Display*, GLXDrawable);
typedef void Linux_glXDestroyContext(Display*, GLXContext);

static Linux_glXCreateContextAttribsARBProc* v_glXCreateContextAttribsARB = 0;
static Linux_glXMakeCurrent* v_glXMakeCurrent = 0;
static Linux_glXSwapBuffers* v_glXSwapBuffers = 0;
static Linux_glXDestroyContext* v_glXDestroyContext = 0;



//~

long_func* error_loader(const char* name) {
	LogError("[X11 GL33 Backend] Failed to load %s", name);
	return (long_func*) nullptr;
}

long_func* glXGetProcAddress(const char* name) {
	return (long_func*) v_glXGetProcAddress((u8*)name);
}

void B_BackendInitShared(OS_Window* _window, OS_Window* _share) {
	
	if (!opengl_module || !v_glXGetProcAddress || !v_glXQueryVersion || !v_glXChooseFBConfig) {
		const char* sonames[] = {
			"libGL.so.1",
			"libGL.so",
			NULL
		};
		
		for (u32 i = 0; sonames[i]; i++) {
			opengl_module = dlopen(sonames[i], RTLD_LAZY);
			if (opengl_module)
				break;
		}
		
		v_glXGetProcAddress = (Linux_glXGetProcAddress*) dlsym(opengl_module, "glXGetProcAddress");
		if (!v_glXGetProcAddress) {
			LogReturn(, "Linux OpenGL GLX Function Loading: Loading glXGetProcAddress failed");
		}
		v_glXQueryVersion = (Linux_glXQueryVersion*) dlsym(opengl_module, "glXQueryVersion");
		if (!v_glXQueryVersion) {
			LogReturn(, "Linux OpenGL GLX Function Loading: Loading glXQueryVersion failed");
		}
		v_glXChooseFBConfig = (Linux_glXChooseFBConfig*) dlsym(opengl_module, "glXChooseFBConfig");
		if (!v_glXChooseFBConfig) {
			LogReturn(, "Linux OpenGL GLX Function Loading: Loading glXChooseFBConfig failed");
		}
	}
	
	if (!v_glXCreateContextAttribsARB || !v_glXMakeCurrent || !v_glXSwapBuffers) {
		v_glXCreateContextAttribsARB = (Linux_glXCreateContextAttribsARBProc*)
			v_glXGetProcAddress((u8*)"glXCreateContextAttribsARB");
		if (!v_glXCreateContextAttribsARB) {
			LogReturn(, "Linux OpenGL GLX Function Loading: Loading glXCreateContextAttribsARB failed");
		}
		v_glXMakeCurrent = (Linux_glXMakeCurrent*) v_glXGetProcAddress((u8*)"glXMakeCurrent");
		if (!v_glXMakeCurrent) {
			LogReturn(, "Linux OpenGL GLX Function Loading: Loading glXMakeCurrent failed");
		}
		v_glXSwapBuffers = (Linux_glXSwapBuffers*) v_glXGetProcAddress((u8*)"glXSwapBuffers");
		if (!v_glXSwapBuffers) {
			LogReturn(, "Linux OpenGL GLX Function Loading: Loading glXSwapBuffers failed");
		}
		v_glXDestroyContext = (Linux_glXDestroyContext*)
			v_glXGetProcAddress((u8*)"glXDestroyContext");
		if (!v_glXSwapBuffers) {
			LogReturn(, "Linux OpenGL GLX Function Loading: Loading glXDestroyContext failed");
		}
	}
	
	X11_Window* window = (X11_Window*) _window;
	X11_Window* share = (X11_Window*) _share;
	
	if (share) {
		window->gl_context = share->gl_context;
	} else {
		static int visual_attribs[] = {
			GLX_X_RENDERABLE    , true,
			GLX_DRAWABLE_TYPE   , GLX_WINDOW_BIT,
			GLX_RENDER_TYPE     , GLX_RGBA_BIT,
			GLX_X_VISUAL_TYPE   , GLX_TRUE_COLOR,
			GLX_RED_SIZE        , 8,
			GLX_GREEN_SIZE      , 8,
			GLX_BLUE_SIZE       , 8,
			GLX_ALPHA_SIZE      , 8,
			GLX_DEPTH_SIZE      , 24,
			GLX_STENCIL_SIZE    , 8,
			GLX_DOUBLEBUFFER    , true,
			None
		};
		int glx_major, glx_minor;
		if (!v_glXQueryVersion(window->display, &glx_major, &glx_minor) || 
			((glx_major == 1) && (glx_minor < 3)) || (glx_major < 1)) {
			LogReturn(, "Invalid GLX version");
		}
		
		int fbcount;
		GLXFBConfig* fbc = v_glXChooseFBConfig(window->display, DefaultScreen(window->display), visual_attribs, &fbcount);
		if (!fbc) {
			LogReturn(, "Failed to retrieve a framebuffer config\n");
		}
		
		static int context_attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
			GLX_CONTEXT_MINOR_VERSION_ARB, 6,
			GLX_CONTEXT_PROFILE_MASK_ARB,  GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
#if defined (_DEBUG)
			GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,
#endif
			None
		};
		
		window->gl_context = v_glXCreateContextAttribsARB(window->display, fbc[0], NULL, true, context_attribs);
		
		__LoadGLFunctions(glXGetProcAddress, error_loader);
	}
	
	v_glXMakeCurrent(window->display, window->handle, window->gl_context);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(MessageCallback, 0);
}

void B_BackendInit(OS_Window* _window) {
	B_BackendInitShared(_window, 0);
}

void B_BackendSelectRenderWindow(OS_Window* _window) {
	X11_Window* window = (X11_Window*) _window;
	v_glXMakeCurrent(window->display, window->handle, window->gl_context);
	
}

void B_BackendSwapchainNext(OS_Window* _window) {
	X11_Window* window = (X11_Window*) _window;
	v_glXSwapBuffers(window->display, window->handle);
}

void B_BackendFree(OS_Window* _window) {
	X11_Window* window = (X11_Window*) _window;
	v_glXMakeCurrent(window->display, window->handle, 0);
	v_glXDestroyContext(window->display, window->gl_context);
}
