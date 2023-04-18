#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dirent.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <dlfcn.h>

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


// NOTE(voxel): Will uncomment this if I find a way of doing this for linux.
//              This doesn't make much sense for linux to be honest
//void OS_FileOpen(string filename) {
//}

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
			getcwd((i8*)buffer, 4096);
			result.size = strlen((const char*)buffer);
			result = str_alloc(arena, result.size);
			memcpy(result.str, buffer, result.size);
			free(buffer);
			scratch_return(&scratch);
		} break;
		
		case SystemPath_Binary: {
			M_Scratch scratch = scratch_get();
			u8* buffer = malloc(4096); // Should be plenty of space?
			readlink("/proc/self/exe", (i8*)buffer, 4096);
			result.size = strlen((const char*)buffer);
			result = str_alloc(arena, result.size);
			memcpy(result.str, buffer, result.size);
			free(buffer);
			u64 end = str_find_last(result, str_lit("/"), 0);
			result.size = end - 1;
			scratch_return(&scratch);
		} break;
		
		case SystemPath_UserData: {
			char* buffer = getenv("HOME");
			result.str = (u8*)buffer;
			result.size = strlen((const char*)buffer);
		} break;
		
		case SystemPath_TempData: {
			return str_lit("/tmp");
		} break;
	}
	
	return result;
}

//~ File Properties

static U_DateTime linux_date_time_from_system_time(struct tm* in, u16 ms) {
	U_DateTime result = {0};
	result.year   = in->tm_year;
	result.month  = in->tm_mon;
	result.day    = in->tm_mday;
	result.hour   = in->tm_hour;
	result.minute = in->tm_min;
	result.sec    = in->tm_sec;
	result.ms     = ms;
	return result;
}

static struct timespec linux_local_system_time_from_date_time(U_DateTime* in) {
	struct tm result_tm = {0};
	result_tm.tm_year = in->year;
	result_tm.tm_mon = in->month;
	result_tm.tm_mday = in->day;
	result_tm.tm_hour = in->hour;
	result_tm.tm_min = in->minute;
	result_tm.tm_sec = in->sec;
	long ms = in->ms;
	time_t result_tt = timelocal(&result_tm);
	struct timespec result = { .tv_sec = result_tt, .tv_nsec = ms * 1000000 };
	return result;
}

static struct timespec linux_universal_system_time_from_date_time(U_DateTime* in) {
	struct tm result_tm = {0};
	result_tm.tm_year = in->year;
	result_tm.tm_mon = in->month;
	result_tm.tm_mday = in->day;
	result_tm.tm_hour = in->hour;
	result_tm.tm_min = in->minute;
	result_tm.tm_sec = in->sec;
	long ms = in->ms;
	time_t result_tt = timegm(&result_tm);
	struct timespec result = { .tv_sec = result_tt, .tv_nsec = ms * 1000000 };
	return result;
}

OS_FileProperties OS_FileGetProperties(string filename) {
	M_Scratch scratch = scratch_get();
	string nt = str_copy(&scratch.arena, filename);
	struct stat st;
	stat((char*)nt.str, &st);
	
	OS_FileProperties result = {0};
	result.size = st.st_size;
	
	// Looks like this isn't actually recorded on linux, so st_ctimespec is the closest I can get...
	struct tm *ctm = localtime(&st.st_ctim.tv_sec);
	U_DateTime createDT = linux_date_time_from_system_time(ctm, (u16)(st.st_ctim.tv_nsec / 1000000));
	struct tm *mtm = localtime(&st.st_mtim.tv_sec);
	U_DateTime modifyDT = linux_date_time_from_system_time(mtm, (u16)(st.st_ctim.tv_nsec / 1000000));
	
	result.create_time = U_DenseTimeFromDateTime(&createDT);
	result.modify_time = U_DenseTimeFromDateTime(&modifyDT);
	result.flags = 0;
	if (S_ISDIR(st.st_mode)) result.flags |= FileProperty_IsFolder;
	result.access = 0;
	
	// I think these should be or-ed together right?
	if (st.st_mode & S_IRUSR || st.st_mode & S_IRGRP || st.st_mode & S_IROTH)
		result.access |= DataAccess_Read;
	if (st.st_mode & S_IWUSR || st.st_mode & S_IWGRP || st.st_mode & S_IWOTH)
		result.access |= DataAccess_Write;
	if (st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH)
		result.access |= DataAccess_Exec;
	
	scratch_return(&scratch);
	return result;
}

//~ File Iterator

typedef struct Linux_FileIter {
	DIR* directory;
	string lookup_dir;
	b32 done;
} Linux_FileIter;


OS_FileIterator OS_FileIterInit(string path) {
	M_Scratch scratch = scratch_get();
	
	string lookup = str_copy(&scratch.arena, path);
	OS_FileIterator result = {0};
	Linux_FileIter* linux_iter = (Linux_FileIter*) &result;
	linux_iter->lookup_dir = path;
	linux_iter->directory = opendir((char*) lookup.str);
	
	scratch_return(&scratch);
	return result;
}

// NOTE(voxel): Will uncomment this if I find a way of doing this for linux.
//              The only way I can think of is to do the pattern check manually...
//              That would take some time. so I'll just keep it commented so it throws a link error
// OS_FileIterator OS_FileIterInitPattern(string lookup) {}


b32  OS_FileIterNext(M_Arena* arena, OS_FileIterator* iter, string* name_out, OS_FileProperties* prop_out) {
	Linux_FileIter* linux_iter = (Linux_FileIter*) iter;
	struct dirent* entry = NULL;
	if ((entry = readdir(linux_iter->directory)) == nullptr) {
		return false;
	}
	
	M_Scratch scratch = scratch_get();
	
	string fname_nolf = str_make(entry->d_name);
	while (str_eq(fname_nolf, str_lit(".")) || str_eq(fname_nolf, str_lit(".."))) {
		struct dirent* entry = NULL;
		if ((entry = readdir(linux_iter->directory)) == nullptr) {
			return false;
		}
		fname_nolf = str_make(entry->d_name);
	}
	
	*name_out = str_copy(arena, fname_nolf);
	string fullname = str_cat(&scratch.arena, linux_iter->lookup_dir, str_lit("/"));
	fullname = str_cat(&scratch.arena, fullname, fname_nolf);
	
	struct stat st;
	stat((char*)fullname.str, &st);
	
	prop_out->size = st.st_size;
	
	// Looks like this isn't actually recorded on linux, so st_ctimespec is the closest I can get...
	struct tm *ctm = localtime(&st.st_ctim.tv_sec);
	U_DateTime createDT = linux_date_time_from_system_time(ctm, (u16)(st.st_ctim.tv_nsec / 1000000));
	struct tm *mtm = localtime(&st.st_mtim.tv_sec);
	U_DateTime modifyDT = linux_date_time_from_system_time(mtm, (u16)(st.st_ctim.tv_nsec / 1000000));
	
	prop_out->create_time = U_DenseTimeFromDateTime(&createDT);
	prop_out->modify_time = U_DenseTimeFromDateTime(&modifyDT);
	prop_out->flags = 0;
	if (S_ISDIR(st.st_mode)) prop_out->flags |= FileProperty_IsFolder;
	prop_out->access = 0;
	
	// I think these should be or-ed together right?
	if (st.st_mode & S_IRUSR || st.st_mode & S_IRGRP || st.st_mode & S_IROTH)
		prop_out->access |= DataAccess_Read;
	if (st.st_mode & S_IWUSR || st.st_mode & S_IWGRP || st.st_mode & S_IWOTH)
		prop_out->access |= DataAccess_Write;
	if (st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH)
		prop_out->access |= DataAccess_Exec;
	
	scratch_return(&scratch);
	return true;
}

void OS_FileIterEnd(OS_FileIterator* iter) {
	Linux_FileIter *linux_iter = (Linux_FileIter*)iter;
	closedir(linux_iter->directory);
}

//~ Time


U_DateTime OS_TimeUniversalNow(void) {
	struct timespec spec;
	clock_gettime(CLOCK_REALTIME, &spec);
	struct tm *the_tm = localtime(&spec.tv_sec);
	return linux_date_time_from_system_time(the_tm, (u16)(spec.tv_nsec / 1000000));
}

U_DateTime OS_TimeLocalFromUniversal(U_DateTime* date_time) {
	struct timespec local = linux_local_system_time_from_date_time(date_time);
	struct tm *the_tm = localtime(&local.tv_sec);
	return linux_date_time_from_system_time(the_tm, (u16)(local.tv_nsec / 1000000));
}


U_DateTime OS_TimeUniversalFromLocal(U_DateTime* date_time) {
	struct timespec univ = linux_universal_system_time_from_date_time(date_time);
	struct tm *the_tm = localtime(&univ.tv_sec);
	return linux_date_time_from_system_time(the_tm, (u16)(univ.tv_nsec / 1000000));
}


u64 OS_TimeMicrosecondsNow(void) {
	struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	u64 us = ((u64)ts.tv_sec * 1000000) + ((u64)ts.tv_nsec / 1000000);
    return us;
}

void OS_TimeSleepMilliseconds(u32 t) {
	struct timespec ret;
	struct timespec sleep_for = { (i32)(t / 1000), (t % 1000) * 1000000 };
	nanosleep(&sleep_for , &ret);
}

//~

OS_Library OS_LibraryLoad(string path) {
	OS_Library result = {0};
	M_Scratch scratch = scratch_get();
	string nt = str_copy(&scratch.arena, path);
	result.v[0] = (u64) dlopen((char*) nt.str, RTLD_LAZY);
	scratch_return(&scratch);
	return result;
}

void_func* OS_LibraryGetFunction(OS_Library lib, char* name) {
	return (void_func*) dlsym((void*)lib.v[0], name);
}

void OS_LibraryRelease(OS_Library lib) {
	dlclose((void*)lib.v[0]);
}


//~ Threading

typedef struct Linux_Thread {
	pthread_t handle;
} Linux_Thread;

typedef void* linux_thread_func(void* context);

OS_Thread OS_ThreadCreate(thread_func* start, void* context) {
	OS_Thread ret = {0};
	Linux_Thread* linux_thread = (Linux_Thread*)&ret;
	pthread_create(&linux_thread->handle, nullptr, (linux_thread_func*)start, context);
	return ret;
}

void OS_ThreadWaitForJoin(OS_Thread* other) {
	Linux_Thread* linux_thread = (Linux_Thread*) other;
	pthread_join(linux_thread->handle, nullptr);
}

// NOTE(voxel): Will uncomment this if I find a way of doing this for linux.
//              The only way I can think of is to use mutexes and conditions myself.
//              That would take some time. so I'll just keep it commented so it throws a link error
//void OS_ThreadWaitForJoinAll(OS_Thread** threads, u32 count) {}
//void OS_ThreadWaitForJoinAny(OS_Thread** threads, u32 count) {}

