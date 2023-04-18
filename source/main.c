#include "defines.h"
#include "os/os.h"
#include "base/tctx.h"
#include "base/log.h"

int main() {
	OS_Init();
	
	ThreadContext context = {0};
	tctx_init(&context);
	
	OS_FileCreateWrite(str_lit("hi.txt"), str_lit("TEST"));
	
	M_Arena global_arena = {0};
	arena_init(&global_arena);
	
	string cwd = OS_Filepath(&global_arena, SystemPath_CurrentDir);
	string bin = OS_Filepath(&global_arena, SystemPath_Binary);
	string usr = OS_Filepath(&global_arena, SystemPath_UserData);
	string tmp = OS_Filepath(&global_arena, SystemPath_TempData);
	Log(
		"CurrentDir: %.*s\nBinaryDir: %.*s\nUserDir: %.*s\nTempDir: %.*s\n",
		str_expand(cwd), str_expand(bin), str_expand(usr), str_expand(tmp)
		);
	
	OS_FileIterator iter = OS_FileIterInit(str_lit("."));
	string curr_name = {0};
	OS_FileProperties curr_props = {0};
	while (OS_FileIterNext(&global_arena, &iter, &curr_name, &curr_props)) {
		Log("%.*s: %x", str_expand(curr_name), curr_props.flags);
	}
	OS_FileIterEnd(&iter);
	
	Log("%llu", OS_TimeMicrosecondsNow());
	
	arena_free(&global_arena);
	tctx_free(&context);
}
