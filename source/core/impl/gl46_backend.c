static void debug_proc(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar *message,const void *userParam) {
    printf("OpenGL Backend Error: %s\n", message);
    flush;
}

void B_BackendInitWindow() {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	
#if defined(_DEBUG)
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
}

void B_BackendInit(GLFWwindow* window) {
	glfwMakeContextCurrent(window);
	gladLoadGL();
	
#if defined(_DEBUG)
	glDebugMessageCallback(debug_proc, nullptr);
#endif
}
