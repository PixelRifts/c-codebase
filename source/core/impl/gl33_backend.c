void B_BackendInitWindow() {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
}

void B_BackendInit(GLFWwindow* window) {
	glfwMakeContextCurrent(window);
	gladLoadGL();
}
