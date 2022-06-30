#include "defines.h"
#include "os/os.h"
#include "base/mem.h"

#include <GLFW/glfw3.h>
#include <assert.h>

b8  is_power_of_two(uintptr_t x);
u64 align_forward_u64(u64 ptr, u64 align);

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	glfwInit();
	GLFWwindow* w = glfwCreateWindow(1080, 720, "", nullptr, nullptr);
	while (!glfwWindowShouldClose(w)) glfwPollEvents();
	glfwDestroyWindow(w);
	glfwTerminate();
	
	tctx_free(&context);
}
