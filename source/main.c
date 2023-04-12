#include "defines.h"
#include "os/os.h"
#include "base/tctx.h"

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	OS_FileCreateWrite(str_lit("hi.txt"), str_lit("TEST"));
	
	tctx_free(&context);
}
