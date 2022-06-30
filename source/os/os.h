/* date = March 31st 2022 1:23 pm */

#ifndef OS_H
#define OS_H

#include "base/mem.h"
#include "base/str.h"
#include "base/utils.h"

//~ OS Init

void OS_Init(void);

//~ TLS

void  OS_ThreadContextSet(void* ctx);
void* OS_ThreadContextGet(void);

//~ Memory

void* OS_MemoryReserve(u64 size);
void  OS_MemoryCommit(void* memory, u64 size);
void  OS_MemoryDecommit(void* memory, u64 size);
void  OS_MemoryRelease(void* memory, u64 size);

//~ Files

b32    OS_FileCreate(string filename);

b32    OS_FileRename(string filename, string new_name);
string OS_FileRead(M_Arena* arena, string filename);
b32    OS_FileCreateWrite(string filename, string data);
b32    OS_FileCreateWrite_List(string filename, string_list data);
b32    OS_FileWrite(string filename, string data);
b32    OS_FileWrite_List(string filename, string_list data);
void   OS_FileOpen(string filename);

b32    OS_FileDelete(string filename);

b32    OS_FileCreateDir(string dirname);
b32    OS_FileDeleteDir(string dirname);
void   OS_FileOpenDir(string dirname);

//~ Utility Paths

typedef u32 OS_SystemPath;
enum {
	SystemPath_CurrentDir,
	SystemPath_Binary,
	SystemPath_UserData,
	SystemPath_TempData,
};

string OS_Filepath(M_Arena* arena, OS_SystemPath path);

//~ File Properties

typedef u32 OS_DataAccessFlags;
enum {
	DataAccess_Read  = 0x1,
	DataAccess_Write = 0x2,
	DataAccess_Exec  = 0x4,
};

typedef u32 OS_FilePropertyFlags;
enum {
	FileProperty_IsFolder = 0x1,
};

typedef struct OS_FileProperties {
	u64 size;
	U_DenseTime create_time;
	U_DenseTime modify_time;
	OS_FilePropertyFlags flags;
	OS_DataAccessFlags access;
} OS_FileProperties;

OS_FileProperties OS_FileGetProperties(string filename);

//~ File Iterator

// Just a big buffer. will be OS specific
typedef struct OS_FileIterator {
	u8 v[640];
} OS_FileIterator;

OS_FileIterator OS_FileIterInit(string path);
OS_FileIterator OS_FileIterInitPattern(string path);
b32  OS_FileIterNext(M_Arena* arena, OS_FileIterator* iter, string* name_out, OS_FileProperties* prop_out);
void OS_FileIterEnd(OS_FileIterator* iter);

//~ Time

U_DateTime OS_TimeUniversalNow(void);
U_DateTime OS_TimeLocalFromUniversal(U_DateTime* date_time);
U_DateTime OS_TimeUniversalFromLocal(U_DateTime* date_time);

u64  OS_TimeMicrosecondsNow(void);
void OS_TimeSleepMilliseconds(u32 t);

//~ Shared Libraries

// Just a buffer. will be OS specific
typedef struct OS_Library {
	u64 v[1];
} OS_Library;

OS_Library OS_LibraryLoad(string path);
void_func* OS_LibraryGetFunction(OS_Library lib, char* name);
void       OS_LibraryRelease(OS_Library lib);

#endif //OS_H
