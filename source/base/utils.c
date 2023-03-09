#include "utils.h"

#include <stdio.h>
#include <string.h>

//~ Time

U_DenseTime U_DenseTimeFromDateTime(U_DateTime* datetime) {
    u32 encoded = (u32)((s32)datetime->year + 0x8000);
    U_DenseTime result = 0;
    result += encoded;
    result *= 12;
    result += datetime->month;
    result *= 31;
    result += datetime->day;
    result *= 24;
    result += datetime->hour;
    result *= 60;
    result += datetime->minute;
    result *= 60;
    result += datetime->sec;
    result *= 1000;
    result += datetime->ms;
    return result;
}

U_DateTime U_DateTimeFromDenseTime(U_DenseTime densetime) {
    U_DateTime result = {0};
    result.ms  = densetime % 1000;
    densetime /= 1000;
    result.sec = densetime % 60;
    densetime /= 60;
    result.minute = densetime % 60;
    densetime /= 60;
    result.hour = densetime % 24;
    densetime /= 24;
    result.day = densetime % 31;
    densetime /= 31;
    result.month = densetime % 12;
    densetime /= 12;
    result.year = ((s32)densetime) - 0x8000;
    return result;
}

//~ Time

string U_FixFilepath(M_Arena* arena, string filepath) {
    M_Scratch scratch = scratch_get();
	
    string fixed = filepath;
    fixed = str_replace_all(scratch.arena, fixed, str_lit("\\"), str_lit("/"));
    fixed = str_replace_all(arena, fixed, str_lit("/./"), str_lit("/"));
    while (true) {
        u64 dotdot = str_find_first(fixed, str_lit(".."), 0);
        if (dotdot == fixed.size) break;
		
        u64 last_slash = str_find_last(fixed, str_lit("/"), dotdot - 1);
		
        u64 range = (dotdot + 3) - last_slash;
        string old = fixed;
        fixed = str_alloc(scratch.arena, fixed.size - range);
        memcpy(fixed.str, old.str, last_slash);
        memcpy(fixed.str + last_slash, old.str + dotdot + 3, old.size - range - last_slash + 1);
    }
	
    fixed = str_copy(arena, fixed);
    scratch_return(&scratch);
	
    return fixed;
}

string U_GetFullFilepath(M_Arena* arena, string filename) {
    M_Scratch scratch = scratch_get();
	
    char buffer[PATH_MAX];
    get_cwd(buffer, PATH_MAX);
    string cwd = { .str = (u8*) buffer, .size = strlen(buffer) };
	
    string finalized = str_cat(scratch.arena, cwd, str_lit("/"));
    finalized = str_cat(scratch.arena, finalized, filename);
    finalized = U_FixFilepath(arena, finalized);
	
    scratch_return(&scratch);
    return finalized;
}

string U_GetFilenameFromFilepath(string filepath) {
    u64 last_slash = str_find_last(filepath, str_lit("/"), 0);
	
    if (last_slash == filepath.size)
        last_slash = 0;
    else if (last_slash == 0)
        last_slash = str_find_last(filepath, str_lit("\\"), 0);
	
    u64 sizeof_filename = filepath.size - last_slash;
    return (string) { .str = filepath.str + last_slash, .size = sizeof_filename };
}

string U_GetDirectoryFromFilepath(string filepath) {
    u64 last_slash = str_find_last(filepath, str_lit("/"), 0);
    if (last_slash == 0) last_slash = str_find_last(filepath, str_lit("\\"), 0);
    return (string) { .str = filepath.str, .size = last_slash - 1 };
}

//~ Frame Arena

// Just a global. Maybe should be a thread local but /shrug
M_Arena frame_arena;

void U_FrameArenaInit(void) {
	arena_init(&frame_arena);
}

void U_FrameArenaFree(void) {
	arena_free(&frame_arena);
}

M_Arena* U_GetFrameArena(void) {
	return &frame_arena;
}

void U_ResetFrameArena(void) {
	arena_dealloc_to(&frame_arena, 0);
}

