#include "defines.h"
#include "os/os.h"
#include "base/base.h"

#include <GLFW/glfw3.h>
#include <assert.h>

int another_test();

int test() {
	LogReturn(0, "Returned 0");
}

int main() {
	OS_Init();
	
	Log("A Simple Log");
	LogError("Error %d", 10);
	test();
	another_test();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	glfwInit();
	GLFWwindow* w = glfwCreateWindow(1080, 720, "", nullptr, nullptr);
	while (!glfwWindowShouldClose(w)) glfwPollEvents();
	glfwDestroyWindow(w);
	glfwTerminate();
	
	tctx_free(&context);
}

int another_test() {
	LogReturn(0, "Returned 0 from another test");
}
