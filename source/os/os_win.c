#include <Windows.h>
#include <userenv.h>
#include <stdio.h>

static u64 w32_ticks_per_sec = 1;
static u32 win32_thread_context_index;

//~ OS Init

void OS_Init(void) {
    LARGE_INTEGER perf_freq = {0};
    if (QueryPerformanceFrequency(&perf_freq)) {
        w32_ticks_per_sec = ((u64)perf_freq.HighPart << 32) | perf_freq.LowPart;
    }
    timeBeginPeriod(1);
	
	win32_thread_context_index = TlsAlloc();
}

//~ TLS

void OS_ThreadContextSet(void* ctx) {
	TlsSetValue(win32_thread_context_index, ctx);
}

void* OS_ThreadContextGet(void) {
	return TlsGetValue(win32_thread_context_index);
}

//~ Memory

void* OS_MemoryReserve(u64 size) {
    return VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
}

void OS_MemoryCommit(void* memory, u64 size) {
    VirtualAlloc(memory, size, MEM_COMMIT, PAGE_READWRITE);
}

void OS_MemoryDecommit(void* memory, u64 size) {
    VirtualFree(memory, size, MEM_DECOMMIT);
}

void OS_MemoryRelease(void* memory, u64 size) {
    VirtualFree(memory, 0, MEM_RELEASE);
}

//~ Files

b32 OS_FileCreate(string filename) {
    M_Scratch scratch = scratch_get();
    string_utf16 filename16 = str16_from_str8(&scratch.arena, filename);
    b32 result = true;
    HANDLE file = CreateFileW((WCHAR*)filename16.str,
                              GENERIC_READ, 0, 0,
                              CREATE_NEW, FILE_ATTRIBUTE_NORMAL,
                              0);
    if (GetLastError() == ERROR_FILE_EXISTS) {
        result = false;
    }
    scratch_return(&scratch);
    CloseHandle(file);
    return result;
}

b32 OS_FileExists(string filename) {
	M_Scratch scratch = scratch_get();
    string_utf16 filename16 = str16_from_str8(&scratch.arena, filename);
    DWORD ret = GetFileAttributesW((WCHAR*)filename16.str);
	scratch_return(&scratch);
	return (ret != INVALID_FILE_ATTRIBUTES && !(ret & FILE_ATTRIBUTE_DIRECTORY));
}

b32 OS_FileRename(string filename, string new_name) {
	M_Scratch scratch = scratch_get();
	string_utf16 oldname16 = str16_from_str8(&scratch.arena, filename);
	string_utf16 newname16 = str16_from_str8(&scratch.arena, new_name);
	b32 result = MoveFileW((WCHAR*)oldname16.str, (WCHAR*)newname16.str);
	scratch_return(&scratch);
	return result;
}

string OS_FileRead(M_Arena* arena, string filename) {
	M_Scratch scratch = scratch_get();
	string_utf16 filename16 = str16_from_str8(&scratch.arena, filename);
	HANDLE file = CreateFileW((WCHAR*)filename16.str,
							  GENERIC_READ, 0, 0,
							  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
							  0);
	string result = {0};
	
	if (file != INVALID_HANDLE_VALUE) {
		// get size
		DWORD hi_size = 0;
		DWORD lo_size = GetFileSize(file, &hi_size);
		u64 total_size = (((u64)hi_size) << 32) | (u64)lo_size;
		
		// allocate buffer
		M_ArenaTemp restore_point = arena_begin_temp(arena);
		u8 *buffer = arena_alloc_array(arena, u8, total_size);
		
		// read
		u8 *ptr = buffer;
		u8 *opl = buffer + total_size;
		b32 success = true;
		for (;ptr < opl;){
			u64 total_to_read = (u64)(opl - ptr);
			DWORD to_read = (DWORD)total_to_read;
			if (total_to_read > u32_max){
				to_read = u32_max;
			}
			DWORD actual_read = 0;
			if (!ReadFile(file, ptr, to_read, &actual_read, 0)){
				success = false;
				break;
			}
			ptr += actual_read;
		}
		
		// set result or reset memory
		if (success){
			result.str = buffer;
			result.size = total_size;
		}
		else{
			arena_end_temp(restore_point);
		}
		
		CloseHandle(file);
	}
	
	scratch_return(&scratch);
	return result;
}

b32 OS_FileCreateWrite_List(string filename, string_list data) {
	M_Scratch scratch = scratch_get();
	string_utf16 filename16 = str16_from_str8(&scratch.arena, filename);
	HANDLE file = CreateFileW((WCHAR*)filename16.str,
							  GENERIC_READ, 0, 0,
							  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
							  0);
	
	b32 result = false;
	if (file != INVALID_HANDLE_VALUE){
		result = true;
		
		for (string_list_node *node = data.first;
			 node != 0;
			 node = node->next){
			u8 *ptr = node->str.str;
			u8 *opl = ptr + node->str.size;
			for (;ptr < opl;){
				u64 total_to_write = (u64)(opl - ptr);
				DWORD to_write = total_to_write;
				if (total_to_write > u32_max){
					to_write = u32_max;
				}
				DWORD actual_write = 0;
				if (!WriteFile(file, ptr, to_write, &actual_write, 0)){
					result = false;
					goto dblbreak;
				}
				ptr += actual_write;
			}
		}
		dblbreak:;
		
		CloseHandle(file);
	}
	
	scratch_return(&scratch);
	return result;
}

b32 OS_FileCreateWrite(string filename, string data) {
	M_Scratch scratch = scratch_get();
	string_utf16 filename16 = str16_from_str8(&scratch.arena, filename);
	HANDLE file = CreateFileW((WCHAR*)filename16.str,
							  GENERIC_WRITE, 0, 0,
							  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
							  0);
	
	b32 result = false;
	if (file != INVALID_HANDLE_VALUE){
		result = true;
		
		u8 *ptr = data.str;
		u8 *opl = ptr + data.size;
		for (;ptr < opl;) {
			u64 total_to_write = (u64)(opl - ptr);
			DWORD to_write = total_to_write;
			if (total_to_write > u32_max){
				to_write = u32_max;
			}
			DWORD actual_write = 0;
			if (!WriteFile(file, ptr, to_write, &actual_write, 0)) {
				printf("%lu\n", GetLastError());
				result = false;
				goto dblbreak;
			}
			ptr += actual_write;
		}
		dblbreak:;
		
		CloseHandle(file);
	}
	
	scratch_return(&scratch);
	return result;
}

b32 OS_FileWrite_List(string filename, string_list data) {
	M_Scratch scratch = scratch_get();
	string_utf16 filename16 = str16_from_str8(&scratch.arena, filename);
	HANDLE file = CreateFileW((WCHAR*)filename16.str,
							  GENERIC_READ, 0, 0,
							  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
							  0);
	
	b32 result = false;
	if (file != INVALID_HANDLE_VALUE){
		result = true;
		
		for (string_list_node *node = data.first;
			 node != 0;
			 node = node->next){
			u8 *ptr = node->str.str;
			u8 *opl = ptr + node->str.size;
			for (;ptr < opl;){
				u64 total_to_write = (u64)(opl - ptr);
				DWORD to_write = total_to_write;
				if (total_to_write > u32_max){
					to_write = u32_max;
				}
				DWORD actual_write = 0;
				if (!WriteFile(file, ptr, to_write, &actual_write, 0)){
					result = false;
					goto dblbreak;
				}
				ptr += actual_write;
			}
		}
		dblbreak:;
		
		CloseHandle(file);
	}
	
	scratch_return(&scratch);
	return result;
}

b32 OS_FileWrite(string filename, string data) {
	M_Scratch scratch = scratch_get();
	string_utf16 filename16 = str16_from_str8(&scratch.arena, filename);
	HANDLE file = CreateFileW((WCHAR*)filename16.str,
							  GENERIC_WRITE, 0, 0,
							  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
							  0);
	
	b32 result = false;
	if (file != INVALID_HANDLE_VALUE) {
		result = true;
		
		u8 *ptr = data.str;
		u8 *opl = ptr + data.size;
		for (;ptr < opl;) {
			u64 total_to_write = (u64)(opl - ptr);
			DWORD to_write = total_to_write;
			if (total_to_write > u32_max) {
				to_write = u32_max;
			}
			DWORD actual_write = 0;
			if (!WriteFile(file, ptr, to_write, &actual_write, 0)) {
				printf("%lu\n", GetLastError());
				result = false;
				goto dblbreak;
			}
			ptr += actual_write;
		}
		dblbreak:;
		
		CloseHandle(file);
	}
	
	scratch_return(&scratch);
	return result;
}

void OS_FileOpen(string filename) {
	M_Scratch scratch = scratch_get();
	string_utf16 filename16 = str16_from_str8(&scratch.arena, filename);
	ShellExecuteW(nullptr, nullptr, (WCHAR*) filename16.str, nullptr, nullptr, SW_SHOWNORMAL);
	scratch_return(&scratch);
}

b32 OS_FileDelete(string filename) {
	M_Scratch scratch = scratch_get();
	string_utf16 filename16 = str16_from_str8(&scratch.arena, filename);
	b32 result = DeleteFileW((WCHAR*)filename16.str);
	scratch_return(&scratch);
	return result;
}

//~ File Properties

static U_DateTime w32_date_time_from_system_time(SYSTEMTIME* in){
	U_DateTime result = {};
	result.year   = in->wYear;
	result.month  = (u8)in->wMonth;
	result.day    = in->wDay;
	result.hour   = in->wHour;
	result.minute = in->wMinute;
	result.sec    = in->wSecond;
	result.ms     = in->wMilliseconds;
	return result;
}

static SYSTEMTIME w32_system_time_from_date_time(U_DateTime* in){
	SYSTEMTIME result    = {0};
	result.wYear         = in->year;
	result.wMonth        = in->month;
	result.wDay          = in->day;
	result.wHour         = in->hour;
	result.wMinute       = in->minute;
	result.wSecond       = in->sec;
	result.wMilliseconds = in->ms;
	return result;
}

static u64 w32_dense_time_from_file_time(FILETIME *file_time) {
	SYSTEMTIME system_time;
	FileTimeToSystemTime(file_time, &system_time);
	U_DateTime date_time = w32_date_time_from_system_time(&system_time);
	U_DenseTime result = U_DenseTimeFromDateTime(&date_time);
	return result;
}

static OS_FilePropertyFlags w32_prop_flags_from_attribs(DWORD attribs) {
	OS_FilePropertyFlags result = 0;
	if (attribs & FILE_ATTRIBUTE_DIRECTORY){
		result |= FileProperty_IsFolder;
	}
	return result;
}

static OS_DataAccessFlags w32_access_from_attributes(DWORD attribs) {
	OS_DataAccessFlags result = DataAccess_Read | DataAccess_Exec;
	if (!(attribs & FILE_ATTRIBUTE_READONLY)){
		result |= DataAccess_Write;
	}
	return result;
}

OS_FileProperties OS_FileGetProperties(string filename) {
	M_Scratch scratch = scratch_get();
	string_utf16 filename16 = str16_from_str8(&scratch.arena, filename);
	OS_FileProperties result = {0};
	WIN32_FILE_ATTRIBUTE_DATA attribs = {0};
	if (GetFileAttributesExW((WCHAR*)filename16.str, GetFileExInfoStandard,
							 &attribs)) {
		result.size = ((u64)attribs.nFileSizeHigh << 32) | (u64)attribs.nFileSizeLow;
		result.flags = w32_prop_flags_from_attribs(attribs.dwFileAttributes);
		result.create_time = w32_dense_time_from_file_time(&attribs.ftCreationTime);
		result.modify_time = w32_dense_time_from_file_time(&attribs.ftLastWriteTime);
		result.access = w32_access_from_attributes(attribs.dwFileAttributes);
	}
	return result;
}

b32 OS_FileCreateDir(string dirname) {
	M_Scratch scratch = scratch_get();
	string_utf16 dirname16 = str16_from_str8(&scratch.arena, dirname);
	b32 result = CreateDirectoryW((WCHAR*) dirname16.str, 0);
	scratch_return(&scratch);
	return result;
}

b32 OS_FileDeleteDir(string dirname) {
	M_Scratch scratch = scratch_get();
	string_utf16 dirname16 = str16_from_str8(&scratch.arena, dirname);
	b32 result = RemoveDirectoryW((WCHAR*) dirname16.str);
	scratch_return(&scratch);
	return result;
}

void OS_FileOpenDir(string dirname) {
	M_Scratch scratch = scratch_get();
	string_utf16 dirname16 = str16_from_str8(&scratch.arena, dirname);
	string_utf16 explore = str16_from_str8(&scratch.arena, str_lit("explore"));
	ShellExecuteW(nullptr, (WCHAR*) explore.str, (WCHAR*) dirname16.str, nullptr, nullptr, SW_SHOWNORMAL);
	scratch_return(&scratch);
}

//~ File Iterator

typedef struct W32_FileIter {
	HANDLE handle;
	WIN32_FIND_DATAW find_data;
	b32 done;
} W32_FileIter;

OS_FileIterator OS_FileIterInit(string path) {
	M_Scratch scratch = scratch_get();
	
	string lookup = str_cat(&scratch.arena, path, str_lit("\\*"));
	string_utf16 lookup16 = str16_from_str8(&scratch.arena, lookup);
	OS_FileIterator result = {0};
	W32_FileIter* w32_iter = (W32_FileIter*) &result;
	w32_iter->handle = FindFirstFileW((WCHAR*) lookup16.str, &w32_iter->find_data);
	
	scratch_return(&scratch);
	return result;
}

OS_FileIterator OS_FileIterInitPattern(string lookup) {
	M_Scratch scratch = scratch_get();
	
	string_utf16 lookup16 = str16_from_str8(&scratch.arena, lookup);
	OS_FileIterator result = {0};
	W32_FileIter* w32_iter = (W32_FileIter*) &result;
	w32_iter->handle = FindFirstFileW((WCHAR*) lookup16.str, &w32_iter->find_data);
	
	scratch_return(&scratch);
	return result;
}

b32  OS_FileIterNext(M_Arena* arena, OS_FileIterator* iter, string* name_out, OS_FileProperties* prop_out) {
	b32 result = false;
	W32_FileIter* w32_iter = (W32_FileIter*) iter;
	if (w32_iter->handle != 0 &&
		w32_iter->handle != INVALID_HANDLE_VALUE){
		for (;!w32_iter->done;){
			// check for . and ..
			WCHAR *file_name = w32_iter->find_data.cFileName;
			b32 is_dot = (file_name[0] == '.' && file_name[1] == 0);
			b32 is_dotdot = (file_name[0] == '.' && file_name[1] == '.' &&
							 file_name[2] == 0);
			
			// setup to emit
			b32 emit = (!is_dot && !is_dotdot);
			WIN32_FIND_DATAW data = {};
			if (emit){
				MemoryCopyStruct(&data, &w32_iter->find_data);
			}
			
			// increment the iterator
			if (!FindNextFileW(w32_iter->handle, &w32_iter->find_data)){
				w32_iter->done = true;
			}
			
			// do the emit if we saved one earlier
			if (emit){
				*name_out = str8_from_str16(arena, str16_cstring((u16*)data.cFileName));
				prop_out->size = ((u64)data.nFileSizeHigh << 32) | (u64)data.nFileSizeLow;
				prop_out->flags = w32_prop_flags_from_attribs(data.dwFileAttributes);
				prop_out->create_time = w32_dense_time_from_file_time(&data.ftCreationTime);
				prop_out->modify_time = w32_dense_time_from_file_time(&data.ftLastWriteTime);
				prop_out->access = w32_access_from_attributes(data.dwFileAttributes);
				result = true;
				break;
			}
		}
	}
	return result;
}

void OS_FileIterEnd(OS_FileIterator* iter) {
	W32_FileIter *w32_iter = (W32_FileIter*)iter;
	if (w32_iter->handle != 0 &&
		w32_iter->handle != INVALID_HANDLE_VALUE){
		FindClose(w32_iter->handle);
	}
}

//~ Utility Paths

string OS_Filepath(M_Arena* arena, OS_SystemPath path) {
	string result = {0};
	switch (path) {
		case SystemPath_CurrentDir: {
			M_Scratch scratch = scratch_get();
			DWORD cap = 2048;
			u16* buffer = arena_alloc_array(&scratch.arena, u16, cap);
			DWORD size = GetCurrentDirectoryW(cap, (WCHAR*) buffer);
			if (size >= cap) {
				scratch_reset(&scratch);
				buffer = arena_alloc_array(&scratch.arena, u16, size + 1);
				size = GetCurrentDirectoryW(size + 1, (WCHAR*) buffer);
			}
			result = str8_from_str16(&scratch.arena, (string_utf16) { buffer, size });
			result = str_replace_all(arena, result, str_lit("\\"), str_lit("/"));
			
			scratch_return(&scratch);
		} break;
		
		case SystemPath_Binary: {
			M_Scratch scratch = scratch_get();
			
			DWORD cap = 2048;
			u16 *buffer = 0;
			DWORD size = 0;
			for (u64 r = 0; r < 4; r += 1, cap *= 4){
				u16* try_buffer = arena_alloc_array(&scratch.arena, u16, cap);
				DWORD try_size = GetModuleFileNameW(0, (WCHAR*)try_buffer, cap);
				
				if (try_size == cap && GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
					scratch_reset(&scratch);
				} else {
					buffer = try_buffer;
					size = try_size;
					break;
				}
			}
			
			string full_path = str8_from_str16(&scratch.arena, (string_utf16) { buffer, size });
			string binary_path = U_GetDirectoryFromFilepath(full_path);
			result = str_replace_all(arena, binary_path, str_lit("\\"), str_lit("/"));
			
			scratch_return(&scratch);
		} break;
		
		case SystemPath_UserData: {
			M_Scratch scratch = scratch_get();
			
			HANDLE token = GetCurrentProcessToken();
			DWORD cap = 2048;
			u16 *buffer = arena_alloc_array(&scratch.arena, u16, cap);
			if (!GetUserProfileDirectoryW(token, (WCHAR*)buffer, &cap)) {
				scratch_reset(&scratch);
				buffer = arena_alloc_array(&scratch.arena, u16, cap + 1);
				if (GetUserProfileDirectoryW(token, (WCHAR*)buffer, &cap)) {
					buffer = 0;
				}
			}
			
			if (buffer) {
				result = str8_from_str16(&scratch.arena, str16_cstring(buffer));
				result = str_replace_all(arena, result, str_lit("\\"), str_lit("/"));
			}
			
			scratch_return(&scratch);
		} break;
		
		case SystemPath_TempData: {
			M_Scratch scratch = scratch_get();
			DWORD cap = 2048;
			u16 *buffer = arena_alloc_array(&scratch.arena, u16, cap);
			DWORD size = GetTempPathW(cap, (WCHAR*)buffer);
			if (size >= cap){
				scratch_reset(&scratch);
				buffer = arena_alloc_array(&scratch.arena, u16, size + 1);
				size = GetTempPathW(size + 1, (WCHAR*)buffer);
			}
			result = str8_from_str16(&scratch.arena, (string_utf16) { buffer, size - 1 });
			result = str_replace_all(arena, result, str_lit("\\"), str_lit("/"));
			
			scratch_return(&scratch);
		} break;
	}
	
	return result;
}

//~ Time

U_DateTime OS_TimeUniversalNow(void) {
	SYSTEMTIME system_time;
	GetSystemTime(&system_time);
	U_DateTime result = w32_date_time_from_system_time(&system_time);
	return result;
}

U_DateTime OS_TimeLocalFromUniversal(U_DateTime* date_time) {
	SYSTEMTIME univ_system_time = w32_system_time_from_date_time(date_time);
	FILETIME univ_file_time;
	SystemTimeToFileTime(&univ_system_time, &univ_file_time);
	FILETIME local_file_time;
	FileTimeToLocalFileTime(&univ_file_time, &local_file_time);
	SYSTEMTIME local_system_time;
	FileTimeToSystemTime(&local_file_time, &local_system_time);
	U_DateTime result = w32_date_time_from_system_time(&local_system_time);
	return result;
}

U_DateTime OS_TimeUniversalFromLocal(U_DateTime* date_time) {
	SYSTEMTIME local_system_time = w32_system_time_from_date_time(date_time);
	FILETIME local_file_time;
	SystemTimeToFileTime(&local_system_time, &local_file_time);
	FILETIME univ_file_time;
	LocalFileTimeToFileTime(&local_file_time, &univ_file_time);
	SYSTEMTIME univ_system_time;
	FileTimeToSystemTime(&univ_file_time, &univ_system_time);
	U_DateTime result = w32_date_time_from_system_time(&univ_system_time);
	return result;
}

u64 OS_TimeMicrosecondsNow(void) {
	u64 result = 0;
	LARGE_INTEGER perf_counter = {0};
	if (QueryPerformanceCounter(&perf_counter)) {
		u64 ticks = ((u64)perf_counter.HighPart << 32) | perf_counter.LowPart;
		result = ticks * 1000000 / w32_ticks_per_sec;
	}
	return result;
}

void OS_TimeSleepMilliseconds(u32 t) {
	Sleep(t);
}

//~ Shared Libraries

OS_Library OS_LibraryLoad(string path) {
	OS_Library result = {0};
	M_Scratch scratch = scratch_get();
	string_utf16 path16 = str16_from_str8(&scratch.arena, path);
	result.v[0] = (u64) LoadLibraryW((WCHAR*) path16.str);
	scratch_return(&scratch);
	return result;
}

void_func* OS_LibraryGetFunction(OS_Library lib, char* name) {
	HMODULE module = (HMODULE) lib.v[0];
	void_func* result = (void_func*) GetProcAddress(module, (LPCSTR) name);
	return result;
}

void OS_LibraryRelease(OS_Library lib) {
	HMODULE module = (HMODULE) lib.v[0];
	FreeLibrary(module);
}
