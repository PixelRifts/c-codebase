#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>

// TODO(voxel): Error Checking ?!

//~ OS Init

static pthread_key_t linux_thread_context_key;

void OS_Init(void) {
    pthread_key_create(&linux_thread_context_key, nullptr);
}

//~ TLS

void OS_ThreadContextSet(void* ctx) {
    pthread_setspecific(linux_thread_context_key, ctx);
}

void* OS_ThreadContextGet(void) {
    return pthread_getspecific(linux_thread_context_key);
}

//~ Memory

void* OS_MemoryReserve(u64 size) {
    return mmap(nullptr, size, PROT_NONE, MAP_PRIVATE | MAP_ANON, -1, 0);
}

void OS_MemoryCommit(void* memory, u64 size) {
    mprotect(memory, size, PROT_READ | PROT_WRITE);
}

void OS_MemoryDecommit(void* memory, u64 size) {
    mprotect(memory, size, PROT_NONE);
}

void OS_MemoryRelease(void* memory, u64 size) {
    munmap(memory, size);
}

//~ Files

b32 OS_FileCreate(string filename) {
    M_Scratch scratch = scratch_get();
    string nt = str_copy(&scratch.arena, filename);
    b32 result = true;
    size_t handle = open((const char*) nt.str, O_RDWR | O_CREAT, S_IRUSR | S_IRGRP | S_IROTH);
    if (handle == -1) {
        result = false;
    }
    scratch_return(&scratch);
    close(handle);
    return true;
}

b32 OS_FileExists(string filename) {
    M_Scratch scratch = scratch_get();
    string nt = str_copy(&scratch.arena, filename);
    b32 result = access((const char*) nt.str, F_OK);
    scratch_return(&scratch);
    return result == 0;
}

b32 OS_FileRename(string filename, string new_name) {
    M_Scratch scratch = scratch_get();
    string ntold = str_copy(&scratch.arena, filename);
    string ntnew = str_copy(&scratch.arena, new_name);
    b32 result = rename((const char*)ntold.str, (const char*)ntnew.str);
    scratch_return(&scratch);
    return result == 0;
}

string OS_FileRead(M_Arena* arena, string filename) {
    M_Scratch scratch = scratch_get();
    string nt = str_copy(&scratch.arena, filename);
    
    struct stat st;
    stat((const char*) nt.str, &st);
    string result = str_alloc(arena, st.st_size);
    
    size_t handle = open((const char*) nt.str, O_RDWR, S_IRUSR | S_IRGRP | S_IROTH);
    read(handle, result.str, st.st_size);
    close(handle);
    
    scratch_return(&scratch);
    return result;
}

b32 OS_FileCreateWrite(string filename, string data) {
    M_Scratch scratch = scratch_get();
    string nt = str_copy(&scratch.arena, filename);
    b32 result = true;
    size_t handle =
        open((const char*) nt.str, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP | S_IROTH);
    if (handle == -1) result = false;
    write(handle, data.str, data.size);
    close(handle);
    scratch_return(&scratch);
    return result;
}

b32 OS_FileCreateWrite_List(string filename, string_list data) {
    M_Arena arena;
    arena_init(&arena);
    string nt = str_copy(&arena, filename);
	b32 result = true;
    string o = string_list_flatten(&arena, &data);
    size_t handle =
        open((const char*) nt.str, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IRGRP | S_IROTH);
    if (handle == -1) result = false;
    write(handle, o.str, o.size);
    close(handle);
    
    arena_free(&arena);
	return result;
}

b32 OS_FileWrite(string filename, string data) {
    M_Scratch scratch = scratch_get();
    string nt = str_copy(&scratch.arena, filename);
    b32 result = true;
    size_t handle =
        open((const char*) nt.str, O_RDWR | O_TRUNC, S_IRUSR | S_IRGRP | S_IROTH);
    if (handle == -1) result = false;
    write(handle, data.str, data.size);
    close(handle);
    return result;
}

b32 OS_FileWrite_List(string filename, string_list data) {
	M_Arena arena;
    arena_init(&arena);
    string nt = str_copy(&arena, filename);
	b32 result = true;
    string o = string_list_flatten(&arena, &data);
    size_t handle =
        open((const char*) nt.str, O_RDWR | O_TRUNC, S_IRUSR | S_IRGRP | S_IROTH);
    if (handle == -1) result = false;
    write(handle, o.str, o.size);
    close(handle);
    
    arena_free(&arena);
	return result;
}

void OS_FileOpen(string filename) {
	// TODO(voxel): Is this even possible?
}

b32 OS_FileDelete(string filename) {
	M_Scratch scratch = scratch_get();
    string nt = str_copy(&scratch.arena, filename);
	b32 result = true;
	size_t o = remove((const char*) nt.str);
	if (o == -1) result = false;
	scratch_return(&scratch);
	return result;
}

b32 OS_FileCreateDir(string dirname) {
	M_Scratch scratch = scratch_get();
    string nt = str_copy(&scratch.arena, dirname);
	b32 result = true;
	// NOTE(voxel): Not sure what mode is actually a good default...
	size_t o = mkdir((const char*) nt.str, S_IRUSR | S_IRGRP | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP);
	if (o == -1) result = false;
	scratch_return(&scratch);
	return result;
}

b32 OS_FileDeleteDir(string dirname) {
	M_Scratch scratch = scratch_get();
    string nt = str_copy(&scratch.arena, dirname);
	b32 result = true;
	size_t o = rmdir((const char*) nt.str);
	if (o == -1) result = false;
	scratch_return(&scratch);
	return result;
}

void OS_FileOpenDir(string dirname) {
	// TODO(voxel): Is this even possible?
}

//~ Utility Paths

string OS_Filepath(M_Arena* arena, OS_SystemPath path) {
	string result = {0};
	
	switch (path) {
		case SystemPath_CurrentDir: {
			M_Scratch scratch = scratch_get();
			u8* buffer = malloc(4096); // Should be plenty of space?
			size_t o = getcwd((i8*)buffer, 4096);
			result.size = strlen(buffer);
			result = str_alloc(arena, result.size);
			memcpy(result, buffer, result.size);
			free(buffer);
			scratch_return(&scratch);
		} break;
		
		case SystemPath_Binary: {
			M_Scratch scratch = scratch_get();
			u8* buffer = malloc(4096); // Should be plenty of space?
			size_t o = readlink("/proc/self/exe", (i8*)buf, 4096);
			result.size = strlen(buffer);
			result = str_alloc(arena, result.size);
			memcpy(result, buffer, result.size);
			free(buffer);
			scratch_return(&scratch);
		} break;
		
		case SystemPath_UserData: {
			// TODO(voxel): 
		} break;
		
		case SystemPath_TempData: {
			// TODO(voxel): 
		} break;
	}
	
	return result;
}

// TODO(voxel): 
