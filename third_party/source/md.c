// LICENSE AT END OF FILE (MIT).

/* 
** Overrides & Options Macros
**
** Overridable
**  "basic types" ** REQUIRED
**   #define/typedef MD_i8, MD_i16, MD_i32, MD_i64
**   #define/typedef MD_u8, MD_u16, MD_u32, MD_u64
**   #define/typedef MD_f32, MD_f64
**
**  "memset" ** REQUIRED
**   #define MD_IMPL_Memset             (void*, int, uint64) -> void*
**   #define MD_IMPL_Memmove            (void*, void*, uint64) -> void*
**
**  "file iteration" ** OPTIONAL (required for the metadesk FileIter helpers to work)
**   #define MD_IMPL_FileIterBegin      (MD_FileIter*, MD_String8) -> Boolean
**   #define MD_IMPL_FileIterNext       (MD_Arena*, MD_FileIter*) -> MD_FileInfo
**   #define MD_IMPL_FileIterEnd        (MD_FileIter*) -> void
**
**  "file load" ** OPTIONAL (required for MD_ParseWholeFile to work)
**   #define MD_IMPL_LoadEntireFile     (MD_Arena*, MD_String8 filename) -> MD_String8   
**
**  "low level memory" ** OPTIONAL (required when relying on the default arenas)
**   #define MD_IMPL_Reserve            (uint64) -> void*
**   #define MD_IMPL_Commit             (void*, uint64) -> MD_b32
**   #define MD_IMPL_Decommit           (void*, uint64) -> void
**   #define MD_IMPL_Release            (void*, uint64) -> void
**
**
**  "arena" ** REQUIRED
**   #define MD_IMPL_Arena              <type> (must set before including md.h)
**   #define MD_IMPL_ArenaMinPos        uint64
**   #define MD_IMPL_ArenaAlloc         () -> MD_IMPL_Arena*
**   #define MD_IMPL_ArenaRelease       (MD_IMPL_Arena*) -> void
**   #define MD_IMPL_ArenaGetPos        (MD_IMPL_Arena*) -> uint64
**   #define MD_IMPL_ArenaPush          (MD_IMPL_Arena*, uint64) -> void*
**   #define MD_IMPL_ArenaPopTo         (MD_IMPL_Arena*, uint64) -> void
**   #define MD_IMPL_ArenaSetAutoAlign  (MD_IMPL_Arena*, uint64) -> void
**
**  "scratch" ** REQUIRED
**   #define MD_IMPL_GetScratch         (MD_IMPL_Arena**, uint64) -> MD_IMPL_Arena*
**  "scratch constants" ** OPTIONAL (required for default scratch)
**   #define MD_IMPL_ScratchCount       uint64 [default 2]
**
**  "sprintf" ** REQUIRED
**   #define MD_IMPL_Vsnprintf          (char*, uint64, char const*, va_list) -> uint64
**
** Static Parameters to the Default Arena Implementation
**   #define MD_DEFAULT_ARENA_RES_SIZE  uint64 [default 64 megabytes]
**   #define MD_DEFAULT_ARENA_CMT_SIZE  uint64 [default 64 kilabytes]
**
** Default Implementation Controls
**  These controls default to '1' i.e. 'enabled'
**   #define MD_DEFAULT_BASIC_TYPES -> construct "basic types" from stdint.h header
**   #define MD_DEFAULT_MEMSET    -> construct "memset" from CRT
**   #define MD_DEFAULT_FILE_ITER -> construct "file iteration" from OS headers
**   #define MD_DEFAULT_MEMORY    -> construct "low level memory" from OS headers
**   #define MD_DEFAULT_ARENA     -> construct "arena" from "low level memory"
**   #define MD_DEFAULT_SCRATCH   -> construct "scratch" from "arena"
**   #define MD_DEFAULT_SPRINTF   -> construct "vsnprintf" from internal implementaion
**
*/

#if !defined(MD_C)
#define MD_C

//~/////////////////////////////////////////////////////////////////////////////
/////////////////////////// CRT Implementation /////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#if MD_DEFAULT_MEMSET

#include <stdlib.h>
#include <string.h>

#if !defined(MD_IMPL_Memset)
# define MD_IMPL_Memset MD_CRT_Memset
#endif
#if !defined(MD_IMPL_Memmove)
# define MD_IMPL_Memmove MD_CRT_Memmove
#endif

#define MD_CRT_Memset memset
#define MD_CRT_Memmove memmove

#endif

#if MD_DEFAULT_FILE_LOAD

#include <stdio.h>

#if !defined(MD_IMPL_LoadEntireFile)
# define MD_IMPL_LoadEntireFile MD_CRT_LoadEntireFile
#endif

MD_FUNCTION MD_String8
MD_CRT_LoadEntireFile(MD_Arena *arena, MD_String8 filename)
{
    MD_ArenaTemp scratch = MD_GetScratch(&arena, 1);
    MD_String8 file_contents = MD_ZERO_STRUCT;
    MD_String8 filename_copy = MD_S8Copy(scratch.arena, filename);
    FILE *file = fopen((char*)filename_copy.str, "rb");
    if(file != 0)
    {
        fseek(file, 0, SEEK_END);
        MD_u64 file_size = ftell(file);
        fseek(file, 0, SEEK_SET);
        file_contents.str = MD_PushArray(arena, MD_u8, file_size+1);
        if(file_contents.str)
        {
            file_contents.size = file_size;
            fread(file_contents.str, 1, file_size, file);
            file_contents.str[file_contents.size] = 0;
        }
        fclose(file);
    }
    MD_ReleaseScratch(scratch);
    return file_contents;
}

#endif


//~/////////////////////////////////////////////////////////////////////////////
/////////////////////////// Win32 Implementation ///////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//- win32 header
#if (MD_DEFAULT_FILE_ITER || MD_2DEFAULT_MEMORY) && MD_OS_WINDOWS
# include <Windows.h>
# pragma comment(lib, "User32.lib")
#endif

//- win32 "file iteration"
#if MD_DEFAULT_FILE_ITER && MD_OS_WINDOWS

#if !defined(MD_IMPL_FileIterBegin)
# define MD_IMPL_FileIterBegin MD_WIN32_FileIterBegin
#endif
#if !defined(MD_IMPL_FileIterNext)
# define MD_IMPL_FileIterNext MD_WIN32_FileIterNext
#endif
#if !defined(MD_IMPL_FileIterEnd)
# define MD_IMPL_FileIterEnd MD_WIN32_FileIterEnd
#endif

typedef struct MD_WIN32_FileIter{
    HANDLE state;
    MD_u64 first;
    WIN32_FIND_DATAW find_data;
} MD_WIN32_FileIter;

MD_StaticAssert(sizeof(MD_FileIter) >= sizeof(MD_WIN32_FileIter), file_iter_size_check);

static MD_b32
MD_WIN32_FileIterBegin(MD_FileIter *it, MD_String8 path)
{
    //- init search
    MD_ArenaTemp scratch = MD_GetScratch(0, 0);
    
    MD_u8 c = path.str[path.size - 1];
    MD_b32 need_star = (c == '/' || c == '\\');
    MD_String8 cpath = need_star ? MD_S8Fmt(scratch.arena, "%.*s*", MD_S8VArg(path)) : path;
    MD_String16 cpath16 = MD_S16FromS8(scratch.arena, cpath);
    
    WIN32_FIND_DATAW find_data = MD_ZERO_STRUCT;
    HANDLE state = FindFirstFileW((WCHAR*)cpath16.str, &find_data);
    
    MD_ReleaseScratch(scratch);
    
    //- fill results
    MD_b32 result = !!state;
    if (result)
    {
        MD_WIN32_FileIter *win32_it = (MD_WIN32_FileIter*)it; 
        win32_it->state = state;
        win32_it->first = 1;
        MD_MemoryCopy(&win32_it->find_data, &find_data, sizeof(find_data));
    }
    return(result);
}

static MD_FileInfo
MD_WIN32_FileIterNext(MD_Arena *arena, MD_FileIter *it)
{
    //- get low-level file info for this step
    MD_b32 good = 0;
    
    MD_WIN32_FileIter *win32_it = (MD_WIN32_FileIter*)it; 
    WIN32_FIND_DATAW *find_data = &win32_it->find_data;
    if (win32_it->first)
    {
        win32_it->first = 0;
        good = 1;
    }
    else
    {
        good = FindNextFileW(win32_it->state, find_data);
    }
    
    //- convert to MD_FileInfo
    MD_FileInfo result = {0};
    if (good)
    {
        if (find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            result.flags |= MD_FileFlag_Directory;
        }
        MD_u16 *filename_base = (MD_u16*)find_data->cFileName;
        MD_u16 *ptr = filename_base;
        for (;*ptr != 0; ptr += 1);
        MD_String16 filename16 = {0};
        filename16.str = filename_base;
        filename16.size = (MD_u64)(ptr - filename_base);
        result.filename = MD_S8FromS16(arena, filename16);
        result.file_size = ((((MD_u64)find_data->nFileSizeHigh) << 32) |
                            ((MD_u64)find_data->nFileSizeLow));
    }
    return(result);
}

static void
MD_WIN32_FileIterEnd(MD_FileIter *it)
{
    MD_WIN32_FileIter *win32_it = (MD_WIN32_FileIter*)it; 
    FindClose(win32_it->state);
}

#endif

//- win32 "low level memory"
#if MD_DEFAULT_MEMORY && MD_OS_WINDOWS

#if !defined(MD_IMPL_Reserve)
# define MD_IMPL_Reserve MD_WIN32_Reserve
#endif
#if !defined(MD_IMPL_Commit)
# define MD_IMPL_Commit MD_WIN32_Commit
#endif
#if !defined(MD_IMPL_Decommit)
# define MD_IMPL_Decommit MD_WIN32_Decommit
#endif
#if !defined(MD_IMPL_Release)
# define MD_IMPL_Release MD_WIN32_Release
#endif

static void*
MD_WIN32_Reserve(MD_u64 size)
{
    void *result = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
    return(result);
}

static MD_b32
MD_WIN32_Commit(void *ptr, MD_u64 size)
{
    MD_b32 result = (VirtualAlloc(ptr, size, MEM_COMMIT, PAGE_READWRITE) != 0);
    return(result);
}

static void
MD_WIN32_Decommit(void *ptr, MD_u64 size)
{
    VirtualFree(ptr, size, MEM_DECOMMIT);
}

static void
MD_WIN32_Release(void *ptr, MD_u64 size)
{
    (void)size;
    VirtualFree(ptr, 0, MEM_RELEASE);
}

#endif

//~/////////////////////////////////////////////////////////////////////////////
////////////////////////// Linux Implementation ////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

//- linux headers
#if (MD_DEFAULT_FILE_ITER || MD_DEFAULT_MEMORY) && (MD_OS_LINUX || MD_OS_MAC)
# include <dirent.h>
# include <sys/stat.h>
# include <fcntl.h>
# include <unistd.h>
# include <sys/syscall.h>
# include <sys/mman.h>
// NOTE(mal): To get these constants I need to #define _GNU_SOURCE,
// which invites non-POSIX behavior I'd rather avoid
# ifndef O_PATH
#  define O_PATH                010000000
# endif
# ifndef AT_NO_AUTOMOUNT
#  define AT_NO_AUTOMOUNT        0x800
# endif
# ifndef AT_SYMLINK_NOFOLLOW
#  define AT_SYMLINK_NOFOLLOW    0x100
# endif
#endif

//- linux "file iteration"
#if MD_DEFAULT_FILE_ITER && MD_OS_LINUX

#if !defined(MD_IMPL_FileIterIncrement)
# define MD_IMPL_FileIterIncrement MD_LINUX_FileIterIncrement
#endif

typedef struct MD_LINUX_FileIter MD_LINUX_FileIter;
struct MD_LINUX_FileIter
{
    int dir_fd;
    DIR *dir;
};
MD_StaticAssert(sizeof(MD_LINUX_FileIter) <= sizeof(MD_FileIter), file_iter_size_check);

static MD_b32
MD_LINUX_FileIterIncrement(MD_Arena *arena, MD_FileIter *opaque_it, MD_String8 path,
                           MD_FileInfo *out_info)
{
    MD_b32 result = 0;
    
    MD_LINUX_FileIter *it = (MD_LINUX_FileIter *)opaque_it;
    if(it->dir == 0)
    {
        it->dir = opendir((char*)path.str);
        it->dir_fd = open((char *)path.str, O_PATH|O_CLOEXEC);
    }
    
    if(it->dir != 0 && it->dir_fd != -1)
    {
        struct dirent *dir_entry = readdir(it->dir);
        if(dir_entry)
        {
            out_info->filename = MD_S8Fmt(arena, "%s", dir_entry->d_name);
            out_info->flags = 0;
            
            struct stat st; 
            if(fstatat(it->dir_fd, dir_entry->d_name, &st, AT_NO_AUTOMOUNT|AT_SYMLINK_NOFOLLOW) == 0)
            {
                if((st.st_mode & S_IFMT) == S_IFDIR)
                {
                    out_info->flags |= MD_FileFlag_Directory;
                }
                out_info->file_size = st.st_size;
            }
            result = 1;
        }
    }
    
    if(result == 0)
    {
        if(it->dir != 0)
        {
            closedir(it->dir);
            it->dir = 0;
        }
        if(it->dir_fd != -1)
        {
            close(it->dir_fd);
            it->dir_fd = -1;
        }
    }
    
    return result;
}

#endif

//- linux "low level memory"
#if MD_DEFAULT_MEMORY && (MD_OS_LINUX || MD_OS_MAC)

#if !defined(MD_IMPL_Reserve)
# define MD_IMPL_Reserve MD_LINUX_Reserve
#endif
#if !defined(MD_IMPL_Commit)
# define MD_IMPL_Commit MD_LINUX_Commit
#endif
#if !defined(MD_IMPL_Decommit)
# define MD_IMPL_Decommit MD_LINUX_Decommit
#endif
#if !defined(MD_IMPL_Release)
# define MD_IMPL_Release MD_LINUX_Release
#endif

static void*
MD_LINUX_Reserve(MD_u64 size)
{
    void *result = mmap(0, size, PROT_NONE, MAP_PRIVATE|MAP_ANONYMOUS, -1, (off_t)0);
    return(result);
}

static MD_b32
MD_LINUX_Commit(void *ptr, MD_u64 size)
{
    MD_b32 result = (mprotect(ptr, size, PROT_READ|PROT_WRITE) == 0);
    return(result);
}

static void
MD_LINUX_Decommit(void *ptr, MD_u64 size)
{
    mprotect(ptr, size, PROT_NONE);
    madvise(ptr, size, MADV_DONTNEED);
}

static void
MD_LINUX_Release(void *ptr, MD_u64 size)
{
    munmap(ptr, size);
}

#endif

//~/////////////////////////////////////////////////////////////////////////////
///////////// MD Arena From Reserve/Commit/Decommit/Release ////////////////////
////////////////////////////////////////////////////////////////////////////////

#if MD_DEFAULT_ARENA

#if !defined(MD_DEFAULT_ARENA_RES_SIZE)
# define MD_DEFAULT_ARENA_RES_SIZE (64 << 20)
#endif
#if !defined(MD_DEFAULT_ARENA_CMT_SIZE)
# define MD_DEFAULT_ARENA_CMT_SIZE (64 << 10)
#endif

#define MD_DEFAULT_ARENA_VERY_BIG (MD_DEFAULT_ARENA_RES_SIZE - MD_IMPL_ArenaMinPos)/2

//- "low level memory" implementation check
#if !defined(MD_IMPL_Reserve)
# error Missing implementation for MD_IMPL_Reserve
#endif
#if !defined(MD_IMPL_Commit)
# error Missing implementation for MD_IMPL_Commit
#endif
#if !defined(MD_IMPL_Decommit)
# error Missing implementation for MD_IMPL_Decommit
#endif
#if !defined(MD_IMPL_Release)
# error Missing implementation for MD_IMPL_Release
#endif

#define MD_IMPL_ArenaMinPos 64
MD_StaticAssert(sizeof(MD_ArenaDefault) <= MD_IMPL_ArenaMinPos, arena_def_size_check);

#define MD_IMPL_ArenaAlloc     MD_ArenaDefaultAlloc
#define MD_IMPL_ArenaRelease   MD_ArenaDefaultRelease
#define MD_IMPL_ArenaGetPos    MD_ArenaDefaultGetPos
#define MD_IMPL_ArenaPush      MD_ArenaDefaultPush
#define MD_IMPL_ArenaPopTo     MD_ArenaDefaultPopTo
#define MD_IMPL_ArenaSetAutoAlign MD_ArenaDefaultSetAutoAlign

static MD_ArenaDefault*
MD_ArenaDefaultAlloc__Size(MD_u64 cmt, MD_u64 res)
{
    MD_Assert(MD_IMPL_ArenaMinPos < cmt && cmt <= res);
    MD_u64 cmt_clamped = MD_ClampTop(cmt, res);
    MD_ArenaDefault *result = 0;
    void *mem = MD_IMPL_Reserve(res);
    if (MD_IMPL_Commit(mem, cmt_clamped))
    {
        result = (MD_ArenaDefault*)mem;
        result->prev = 0;
        result->current = result;
        result->base_pos = 0;
        result->pos = MD_IMPL_ArenaMinPos;
        result->cmt = cmt_clamped;
        result->cap = res;
        result->align = 8;
    }
    return(result);
}

static MD_ArenaDefault*
MD_ArenaDefaultAlloc(void)
{
    MD_ArenaDefault *result = MD_ArenaDefaultAlloc__Size(MD_DEFAULT_ARENA_CMT_SIZE,
                                                         MD_DEFAULT_ARENA_RES_SIZE);
    return(result);
}

static void
MD_ArenaDefaultRelease(MD_ArenaDefault *arena)
{
    for (MD_ArenaDefault *node = arena->current, *prev = 0;
         node != 0;
         node = prev)
    {
        prev = node->prev;
        MD_IMPL_Release(node, node->cap);
    }
}

static MD_u64
MD_ArenaDefaultGetPos(MD_ArenaDefault *arena)
{
    MD_ArenaDefault *current = arena->current;
    MD_u64 result = current->base_pos + current->pos;
    return(result);
}

static void*
MD_ArenaDefaultPush(MD_ArenaDefault *arena, MD_u64 size)
{
    // try to be fast!
    MD_ArenaDefault *current = arena->current;
    MD_u64 align = arena->align;
    MD_u64 pos = current->pos;
    MD_u64 pos_aligned = MD_AlignPow2(pos, align);
    MD_u64 new_pos = pos_aligned + size;
    void *result = (MD_u8*)current + pos_aligned;
    current->pos = new_pos;
    
    // if it's not going to work do the slow path
    if (new_pos > current->cmt)
    {
        result = 0;
        current->pos = pos;
        
        // new chunk if necessary
        if (new_pos > current->cap)
        {
            MD_ArenaDefault *new_arena = 0;
            if (size > MD_DEFAULT_ARENA_VERY_BIG)
            {
                MD_u64 big_size_unrounded = size + MD_IMPL_ArenaMinPos;
                MD_u64 big_size = MD_AlignPow2(big_size_unrounded, (4 << 10));
                new_arena = MD_ArenaDefaultAlloc__Size(big_size, big_size);
            }
            else
            {
                new_arena = MD_ArenaDefaultAlloc();
            }
            
            // link in new chunk & recompute new_pos
            if (new_arena != 0)
            {
                new_arena->base_pos = current->base_pos + current->cap;
                new_arena->prev = current;
                current = new_arena;
                pos_aligned = current->pos;
                new_pos = pos_aligned + size;
            }
        }
        
        // move ahead if the current chunk has enough reserve
        if (new_pos <= current->cap)
        {
            
            // extend commit if necessary
            if (new_pos > current->cmt)
            {
                MD_u64 new_cmt_unclamped = MD_AlignPow2(new_pos, MD_DEFAULT_ARENA_CMT_SIZE);
                MD_u64 new_cmt = MD_ClampTop(new_cmt_unclamped, current->cap);
                MD_u64 cmt_size = new_cmt - current->cmt;
                if (MD_IMPL_Commit((MD_u8*)current + current->cmt, cmt_size))
                {
                    current->cmt = new_cmt;
                }
            }
            
            // move ahead if the current chunk has enough commit
            if (new_pos <= current->cmt)
            {
                result = (MD_u8*)current + current->pos;
                current->pos = new_pos;
            }
        }
    }
    
    return(result);
}

static void
MD_ArenaDefaultPopTo(MD_ArenaDefault *arena, MD_u64 pos)
{
    // pop chunks in the chain
    MD_u64 pos_clamped = MD_ClampBot(MD_IMPL_ArenaMinPos, pos);
    (void)pos_clamped;
    {
        MD_ArenaDefault *node = arena->current;
        for (MD_ArenaDefault *prev = 0;
             node != 0 && node->base_pos >= pos;
             node = prev)
        {
            prev = node->prev;
            MD_IMPL_Release(node, node->cap);
        }
        arena->current = node;
    }
    
    // reset the pos of the current
    {
        MD_ArenaDefault *current = arena->current;
        MD_u64 local_pos_unclamped = pos - current->base_pos;
        MD_u64 local_pos = MD_ClampBot(local_pos_unclamped, MD_IMPL_ArenaMinPos);
        current->pos = local_pos;
    }
}

static void
MD_ArenaDefaultSetAutoAlign(MD_ArenaDefault *arena, MD_u64 align)
{
    arena->align = align;
}

static void
MD_ArenaDefaultAbsorb(MD_ArenaDefault *arena, MD_ArenaDefault *sub_arena)
{
    MD_ArenaDefault *current = arena->current;
    MD_u64 base_pos_shift = current->base_pos + current->cap;
    for (MD_ArenaDefault *node = sub_arena->current;
         node != 0;
         node = node->prev)
    {
        node->base_pos += base_pos_shift;
    }
    sub_arena->prev = arena->current;
    arena->current = sub_arena->current;
}

#endif

//- "arena" implementation checks
#if !defined(MD_IMPL_ArenaAlloc)
# error Missing implementation for MD_IMPL_ArenaAlloc
#endif
#if !defined(MD_IMPL_ArenaRelease)
# error Missing implementation for MD_IMPL_ArenaRelease
#endif
#if !defined(MD_IMPL_ArenaGetPos)
# error Missing implementation for MD_IMPL_ArenaGetPos
#endif
#if !defined(MD_IMPL_ArenaPush)
# error Missing implementation for MD_IMPL_ArenaPush
#endif
#if !defined(MD_IMPL_ArenaPopTo)
# error Missing implementation for MD_IMPL_ArenaPopTo
#endif
#if !defined(MD_IMPL_ArenaSetAutoAlign)
# error Missing implementation for MD_IMPL_ArenaSetAutoAlign
#endif
#if !defined(MD_IMPL_ArenaMinPos)
# error Missing implementation for MD_IMPL_ArenaMinPos
#endif

//~/////////////////////////////////////////////////////////////////////////////
///////////////////////////// MD Scratch Pool //////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#if MD_DEFAULT_SCRATCH

#if !defined(MD_IMPL_ScratchCount)
# define MD_IMPL_ScratchCount 2llu
#endif

#if !defined(MD_IMPL_GetScratch)
# define MD_IMPL_GetScratch MD_GetScratchDefault
#endif

MD_THREAD_LOCAL MD_Arena *md_thread_scratch_pool[MD_IMPL_ScratchCount] = {0, 0};

static MD_Arena*
MD_GetScratchDefault(MD_Arena **conflicts, MD_u64 count)
{
    MD_Arena **scratch_pool = md_thread_scratch_pool;
    if (scratch_pool[0] == 0)
    {
        MD_Arena **arena_ptr = scratch_pool;
        for (MD_u64 i = 0; i < MD_IMPL_ScratchCount; i += 1, arena_ptr += 1)
        {
            *arena_ptr = MD_ArenaAlloc();
        }
    }
    MD_Arena *result = 0;
    MD_Arena **arena_ptr = scratch_pool;
    for (MD_u64 i = 0; i < MD_IMPL_ScratchCount; i += 1, arena_ptr += 1)
    {
        MD_Arena *arena = *arena_ptr;
        MD_Arena **conflict_ptr = conflicts;
        for (MD_u32 j = 0; j < count; j += 1, conflict_ptr += 1)
        {
            if (arena == *conflict_ptr)
            {
                arena = 0;
                break;
            }
        }
        if (arena != 0)
        {
            result = arena;
            break;
        }
    }
    return(result);
}

#endif

//~/////////////////////////////////////////////////////////////////////////////
//////////////////////// MD Library Implementation /////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#if MD_DEFAULT_SPRINTF
#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_DECORATE(name) md_stbsp_##name
#include "md/md_stb_sprintf.h"
#endif


//~ Nil Node Definition

static MD_Node _md_nil_node =
{
    &_md_nil_node,         // next
    &_md_nil_node,         // prev
    &_md_nil_node,         // parent
    &_md_nil_node,         // first_child
    &_md_nil_node,         // last_child
    &_md_nil_node,         // first_tag
    &_md_nil_node,         // last_tag
    MD_NodeKind_Nil,       // kind
    0,                     // flags
    MD_ZERO_STRUCT,        // string
    MD_ZERO_STRUCT,        // raw_string
    0,                     // at
    &_md_nil_node,         // ref_target
    MD_ZERO_STRUCT,        // prev_comment
    MD_ZERO_STRUCT,        // next_comment
};

//~ Arena Functions

MD_FUNCTION MD_Arena*
MD_ArenaAlloc(void)
{
    return(MD_IMPL_ArenaAlloc());
}

MD_FUNCTION void
MD_ArenaRelease(MD_Arena *arena)
{
    MD_IMPL_ArenaRelease(arena);
}

MD_FUNCTION void*
MD_ArenaPush(MD_Arena *arena, MD_u64 size)
{
    void *result = MD_IMPL_ArenaPush(arena, size);
    return(result);
}

MD_FUNCTION void
MD_ArenaPutBack(MD_Arena *arena, MD_u64 size)
{
    MD_u64 pos = MD_IMPL_ArenaGetPos(arena);
    MD_u64 new_pos = pos - size;
    MD_u64 new_pos_clamped = MD_ClampBot(MD_IMPL_ArenaMinPos, new_pos);
    MD_IMPL_ArenaPopTo(arena, new_pos_clamped);
}

MD_FUNCTION void
MD_ArenaSetAlign(MD_Arena *arena, MD_u64 boundary)
{
    MD_IMPL_ArenaSetAutoAlign(arena, boundary);
}

MD_FUNCTION void
MD_ArenaPushAlign(MD_Arena *arena, MD_u64 boundary)
{
    MD_u64 pos = MD_IMPL_ArenaGetPos(arena);
    MD_u64 align_m1 = boundary - 1;
    MD_u64 new_pos_aligned = (pos + align_m1)&(~align_m1);
    if (new_pos_aligned > pos)
    {
        MD_u64 amt = new_pos_aligned - pos;
        MD_MemoryZero(MD_IMPL_ArenaPush(arena, amt), amt);
    }
}

MD_FUNCTION void
MD_ArenaClear(MD_Arena *arena)
{
    MD_IMPL_ArenaPopTo(arena, MD_IMPL_ArenaMinPos);
}

MD_FUNCTION MD_ArenaTemp
MD_ArenaBeginTemp(MD_Arena *arena)
{
    MD_ArenaTemp result;
    result.arena = arena;
    result.pos   = MD_IMPL_ArenaGetPos(arena);
    return(result);
}

MD_FUNCTION void
MD_ArenaEndTemp(MD_ArenaTemp temp)
{
    MD_IMPL_ArenaPopTo(temp.arena, temp.pos);
}

//~ Arena Scratch Pool

MD_FUNCTION MD_ArenaTemp
MD_GetScratch(MD_Arena **conflicts, MD_u64 count)
{
    MD_Arena *arena = MD_IMPL_GetScratch(conflicts, count);
    MD_ArenaTemp result = MD_ZERO_STRUCT;
    if (arena != 0)
    {
        result = MD_ArenaBeginTemp(arena);
    }
    return(result);
}

//~ Characters

MD_FUNCTION MD_b32
MD_CharIsAlpha(MD_u8 c)
{
    return MD_CharIsAlphaUpper(c) || MD_CharIsAlphaLower(c);
}

MD_FUNCTION MD_b32
MD_CharIsAlphaUpper(MD_u8 c)
{
    return c >= 'A' && c <= 'Z';
}

MD_FUNCTION MD_b32
MD_CharIsAlphaLower(MD_u8 c)
{
    return c >= 'a' && c <= 'z';
}

MD_FUNCTION MD_b32
MD_CharIsDigit(MD_u8 c)
{
    return (c >= '0' && c <= '9');
}

MD_FUNCTION MD_b32
MD_CharIsUnreservedSymbol(MD_u8 c)
{
    return (c == '~' || c == '!' || c == '$' || c == '%' || c == '^' ||
            c == '&' || c == '*' || c == '-' || c == '=' || c == '+' ||
            c == '<' || c == '.' || c == '>' || c == '/' || c == '?' ||
            c == '|');
}

MD_FUNCTION MD_b32
MD_CharIsReservedSymbol(MD_u8 c)
{
    return (c == '{' || c == '}' || c == '(' || c == ')' || c == '\\' ||
            c == '[' || c == ']' || c == '#' || c == ',' || c == ';'  ||
            c == ':' || c == '@');
}

MD_FUNCTION MD_b32
MD_CharIsSpace(MD_u8 c)
{
    return c == ' ' || c == '\r' || c == '\t' || c == '\f' || c == '\v' || c == '\n';
}

MD_FUNCTION MD_u8
MD_CharToUpper(MD_u8 c)
{
    return (c >= 'a' && c <= 'z') ? ('A' + (c - 'a')) : c;
}

MD_FUNCTION MD_u8
MD_CharToLower(MD_u8 c)
{
    return (c >= 'A' && c <= 'Z') ? ('a' + (c - 'A')) : c;
}

MD_FUNCTION MD_u8
MD_CharToForwardSlash(MD_u8 c)
{
    return (c == '\\' ? '/' : c);
}

//~ Strings

MD_FUNCTION MD_u64
MD_CalculateCStringLength(char *cstr)
{
    MD_u64 i = 0;
    for(; cstr[i]; i += 1);
    return i;
}

MD_FUNCTION MD_String8
MD_S8(MD_u8 *str, MD_u64 size)
{
    MD_String8 string;
    string.str = str;
    string.size = size;
    return string;
}

MD_FUNCTION MD_String8
MD_S8Range(MD_u8 *first, MD_u8 *opl)
{
    MD_String8 string;
    string.str = first;
    string.size = (MD_u64)(opl - first);
    return string;
}

MD_FUNCTION MD_String8
MD_S8Substring(MD_String8 str, MD_u64 min, MD_u64 max)
{
    if(max > str.size)
    {
        max = str.size;
    }
    if(min > str.size)
    {
        min = str.size;
    }
    if(min > max)
    {
        MD_u64 swap = min;
        min = max;
        max = swap;
    }
    str.size = max - min;
    str.str += min;
    return str;
}

MD_FUNCTION MD_String8
MD_S8Skip(MD_String8 str, MD_u64 min)
{
    return MD_S8Substring(str, min, str.size);
}

MD_FUNCTION MD_String8
MD_S8Chop(MD_String8 str, MD_u64 nmax)
{
    return MD_S8Substring(str, 0, str.size - nmax);
}

MD_FUNCTION MD_String8
MD_S8Prefix(MD_String8 str, MD_u64 size)
{
    return MD_S8Substring(str, 0, size);
}

MD_FUNCTION MD_String8
MD_S8Suffix(MD_String8 str, MD_u64 size)
{
    return MD_S8Substring(str, str.size - size, str.size);
}

MD_FUNCTION MD_b32
MD_S8Match(MD_String8 a, MD_String8 b, MD_MatchFlags flags)
{
    int result = 0;
    if(a.size == b.size || flags & MD_StringMatchFlag_RightSideSloppy)
    {
        result = 1;
        for(MD_u64 i = 0; i < a.size && i < b.size; i += 1)
        {
            MD_b32 match = (a.str[i] == b.str[i]);
            if(flags & MD_StringMatchFlag_CaseInsensitive)
            {
                match |= (MD_CharToLower(a.str[i]) == MD_CharToLower(b.str[i]));
            }
            if(flags & MD_StringMatchFlag_SlashInsensitive)
            {
                match |= (MD_CharToForwardSlash(a.str[i]) == MD_CharToForwardSlash(b.str[i]));
            }
            if(match == 0)
            {
                result = 0;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION MD_u64
MD_S8FindSubstring(MD_String8 str, MD_String8 substring, MD_u64 start_pos, MD_MatchFlags flags)
{
    MD_b32 found = 0;
    MD_u64 found_idx = str.size;
    for(MD_u64 i = start_pos; i < str.size; i += 1)
    {
        if(i + substring.size <= str.size)
        {
            MD_String8 substr_from_str = MD_S8Substring(str, i, i+substring.size);
            if(MD_S8Match(substr_from_str, substring, flags))
            {
                found_idx = i;
                found = 1;
                if(!(flags & MD_MatchFlag_FindLast))
                {
                    break;
                }
            }
        }
    }
    return found_idx;
}

MD_FUNCTION MD_String8
MD_S8Copy(MD_Arena *arena, MD_String8 string)
{
    MD_String8 res;
    res.size = string.size;
    res.str = MD_PushArray(arena, MD_u8, string.size + 1);
    MD_MemoryCopy(res.str, string.str, string.size);
    res.str[string.size] = 0;
    return(res);
}

MD_FUNCTION MD_String8
MD_S8FmtV(MD_Arena *arena, char *fmt, va_list args)
{
    MD_String8 result = MD_ZERO_STRUCT;
    va_list args2;
    va_copy(args2, args);
    MD_u64 needed_bytes = MD_IMPL_Vsnprintf(0, 0, fmt, args)+1;
    result.str = MD_PushArray(arena, MD_u8, needed_bytes);
    result.size = needed_bytes - 1;
    result.str[needed_bytes-1] = 0;
    MD_IMPL_Vsnprintf((char*)result.str, (int)needed_bytes, fmt, args2);
    return result;
}

MD_FUNCTION MD_String8
MD_S8Fmt(MD_Arena *arena, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    MD_String8 result = MD_S8FmtV(arena, fmt, args);
    va_end(args);
    return result;
}

MD_FUNCTION void
MD_S8ListPush(MD_Arena *arena, MD_String8List *list, MD_String8 string)
{
    MD_String8Node *node = MD_PushArrayZero(arena, MD_String8Node, 1);
    node->string = string;
    
    MD_QueuePush(list->first, list->last, node);
    list->node_count += 1;
    list->total_size += string.size;
}

MD_FUNCTION void
MD_S8ListPushFmt(MD_Arena *arena, MD_String8List *list, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    MD_String8 string = MD_S8FmtV(arena, fmt, args);
    va_end(args);
    MD_S8ListPush(arena, list, string);
}

MD_FUNCTION void
MD_S8ListConcat(MD_String8List *list, MD_String8List *to_push)
{
    if(to_push->first)
    {
        list->node_count += to_push->node_count;
        list->total_size += to_push->total_size;
        
        if(list->last == 0)
        {
            *list = *to_push;
        }
        else
        {
            list->last->next = to_push->first;
            list->last = to_push->last;
        }
    }
    MD_MemoryZeroStruct(to_push);
}

MD_FUNCTION MD_String8List
MD_S8Split(MD_Arena *arena, MD_String8 string, int splitter_count,
           MD_String8 *splitters)
{
    MD_String8List list = MD_ZERO_STRUCT;
    
    MD_u64 split_start = 0;
    for(MD_u64 i = 0; i < string.size; i += 1)
    {
        MD_b32 was_split = 0;
        for(int split_idx = 0; split_idx < splitter_count; split_idx += 1)
        {
            MD_b32 match = 0;
            if(i + splitters[split_idx].size <= string.size)
            {
                match = 1;
                for(MD_u64 split_i = 0; split_i < splitters[split_idx].size && i + split_i < string.size; split_i += 1)
                {
                    if(splitters[split_idx].str[split_i] != string.str[i + split_i])
                    {
                        match = 0;
                        break;
                    }
                }
            }
            if(match)
            {
                MD_String8 split_string = MD_S8(string.str + split_start, i - split_start);
                MD_S8ListPush(arena, &list, split_string);
                split_start = i + splitters[split_idx].size;
                i += splitters[split_idx].size - 1;
                was_split = 1;
                break;
            }
        }
        
        if(was_split == 0 && i == string.size - 1)
        {
            MD_String8 split_string = MD_S8(string.str + split_start, i+1 - split_start);
            MD_S8ListPush(arena, &list, split_string);
            break;
        }
    }
    
    return list;
}

MD_FUNCTION MD_String8
MD_S8ListJoin(MD_Arena *arena, MD_String8List list, MD_StringJoin *join_ptr)
{
    // setup join parameters
    MD_StringJoin join = MD_ZERO_STRUCT;
    if (join_ptr != 0)
    {
        MD_MemoryCopy(&join, join_ptr, sizeof(join));
    }
    
    // calculate size & allocate
    MD_u64 sep_count = 0;
    if (list.node_count > 1)
    {
        sep_count = list.node_count - 1;
    }
    MD_String8 result = MD_ZERO_STRUCT;
    result.size = (list.total_size + join.pre.size +
                   sep_count*join.mid.size + join.post.size);
    result.str = MD_PushArrayZero(arena, MD_u8, result.size);
    
    // fill
    MD_u8 *ptr = result.str;
    MD_MemoryCopy(ptr, join.pre.str, join.pre.size);
    ptr += join.pre.size;
    for(MD_String8Node *node = list.first; node; node = node->next)
    {
        MD_MemoryCopy(ptr, node->string.str, node->string.size);
        ptr += node->string.size;
        if (node != list.last)
        {
            MD_MemoryCopy(ptr, join.mid.str, join.mid.size);
            ptr += join.mid.size;
        }
    }
    MD_MemoryCopy(ptr, join.post.str, join.post.size);
    ptr += join.post.size;
    
    return(result);
}

MD_FUNCTION MD_String8
MD_S8ListJoinMid(MD_Arena *arena, MD_String8List list,
                 MD_String8 mid_separator)
{
    MD_StringJoin join = MD_ZERO_STRUCT;
    join.pre = MD_S8Lit("");
    join.post = MD_S8Lit("");
    join.mid = mid_separator;
    MD_String8 result = MD_S8ListJoin(arena, list, &join);
    return result;
}

MD_FUNCTION MD_String8
MD_S8Stylize(MD_Arena *arena, MD_String8 string, MD_IdentifierStyle word_style,
             MD_String8 separator)
{
    MD_String8 result = MD_ZERO_STRUCT;
    
    MD_String8List words = MD_ZERO_STRUCT;
    
    MD_b32 break_on_uppercase = 0;
    {
        break_on_uppercase = 1;
        for(MD_u64 i = 0; i < string.size; i += 1)
        {
            if(!MD_CharIsAlpha(string.str[i]) && !MD_CharIsDigit(string.str[i]))
            {
                break_on_uppercase = 0;
                break;
            }
        }
    }
    
    MD_b32 making_word = 0;
    MD_String8 word = MD_ZERO_STRUCT;
    
    for(MD_u64 i = 0; i < string.size;)
    {
        if(making_word)
        {
            if((break_on_uppercase && MD_CharIsAlphaUpper(string.str[i])) ||
               string.str[i] == '_' || MD_CharIsSpace(string.str[i]) ||
               i == string.size - 1)
            {
                if(i == string.size - 1)
                {
                    word.size += 1;
                }
                making_word = 0;
                MD_S8ListPush(arena, &words, word);
            }
            else
            {
                word.size += 1;
                i += 1;
            }
        }
        else
        {
            if(MD_CharIsAlpha(string.str[i]))
            {
                making_word = 1;
                word.str = string.str + i;
                word.size = 1;
            }
            i += 1;
        }
    }
    
    result.size = words.total_size;
    if(words.node_count > 1)
    {
        result.size += separator.size*(words.node_count-1);
    }
    result.str = MD_PushArrayZero(arena, MD_u8, result.size);
    
    {
        MD_u64 write_pos = 0;
        for(MD_String8Node *node = words.first; node; node = node->next)
        {
            
            // NOTE(rjf): Write word string to result.
            {
                MD_MemoryCopy(result.str + write_pos, node->string.str, node->string.size);
                
                // NOTE(rjf): Transform string based on word style.
                switch(word_style)
                {
                    case MD_IdentifierStyle_UpperCamelCase:
                    {
                        result.str[write_pos] = MD_CharToUpper(result.str[write_pos]);
                        for(MD_u64 i = write_pos+1; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToLower(result.str[i]);
                        }
                    }break;
                    
                    case MD_IdentifierStyle_LowerCamelCase:
                    {
                        MD_b32 is_first = (node == words.first);
                        result.str[write_pos] = (is_first ?
                                                 MD_CharToLower(result.str[write_pos]) :
                                                 MD_CharToUpper(result.str[write_pos]));
                        for(MD_u64 i = write_pos+1; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToLower(result.str[i]);
                        }
                    }break;
                    
                    case MD_IdentifierStyle_UpperCase:
                    {
                        for(MD_u64 i = write_pos; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToUpper(result.str[i]);
                        }
                    }break;
                    
                    case MD_IdentifierStyle_LowerCase:
                    {
                        for(MD_u64 i = write_pos; i < write_pos + node->string.size; i += 1)
                        {
                            result.str[i] = MD_CharToLower(result.str[i]);
                        }
                    }break;
                    
                    default: break;
                }
                
                write_pos += node->string.size;
            }
            
            if(node->next)
            {
                MD_MemoryCopy(result.str + write_pos, separator.str, separator.size);
                write_pos += separator.size;
            }
        }
    }
    
    return result;
}

//~ Unicode Conversions

MD_GLOBAL MD_u8 md_utf8_class[32] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0,2,2,2,2,3,3,4,5,
};

MD_FUNCTION MD_DecodedCodepoint
MD_DecodeCodepointFromUtf8(MD_u8 *str, MD_u64 max)
{
#define MD_bitmask1 0x01
#define MD_bitmask2 0x03
#define MD_bitmask3 0x07
#define MD_bitmask4 0x0F
#define MD_bitmask5 0x1F
#define MD_bitmask6 0x3F
#define MD_bitmask7 0x7F
#define MD_bitmask8 0xFF
#define MD_bitmask9  0x01FF
#define MD_bitmask10 0x03FF
    
    MD_DecodedCodepoint result = {~((MD_u32)0), 1};
    MD_u8 byte = str[0];
    MD_u8 byte_class = md_utf8_class[byte >> 3];
    switch (byte_class)
    {
        case 1:
        {
            result.codepoint = byte;
        }break;
        
        case 2:
        {
            if (2 <= max)
            {
                MD_u8 cont_byte = str[1];
                if (md_utf8_class[cont_byte >> 3] == 0)
                {
                    result.codepoint = (byte & MD_bitmask5) << 6;
                    result.codepoint |=  (cont_byte & MD_bitmask6);
                    result.advance = 2;
                }
            }
        }break;
        
        case 3:
        {
            if (3 <= max)
            {
                MD_u8 cont_byte[2] = {0};
                cont_byte[0] = str[1];
                cont_byte[1] = str[2];
                if (md_utf8_class[cont_byte[0] >> 3] == 0 &&
                    md_utf8_class[cont_byte[1] >> 3] == 0)
                {
                    result.codepoint = (byte & MD_bitmask4) << 12;
                    result.codepoint |= ((cont_byte[0] & MD_bitmask6) << 6);
                    result.codepoint |=  (cont_byte[1] & MD_bitmask6);
                    result.advance = 3;
                }
            }
        }break;
        
        case 4:
        {
            if (4 <= max)
            {
                MD_u8 cont_byte[3] = {0};
                cont_byte[0] = str[1];
                cont_byte[1] = str[2];
                cont_byte[2] = str[3];
                if (md_utf8_class[cont_byte[0] >> 3] == 0 &&
                    md_utf8_class[cont_byte[1] >> 3] == 0 &&
                    md_utf8_class[cont_byte[2] >> 3] == 0)
                {
                    result.codepoint = (byte & MD_bitmask3) << 18;
                    result.codepoint |= ((cont_byte[0] & MD_bitmask6) << 12);
                    result.codepoint |= ((cont_byte[1] & MD_bitmask6) <<  6);
                    result.codepoint |=  (cont_byte[2] & MD_bitmask6);
                    result.advance = 4;
                }
            }
        }break;
    }
    
    return(result);
}

MD_FUNCTION MD_DecodedCodepoint
MD_DecodeCodepointFromUtf16(MD_u16 *out, MD_u64 max)
{
    MD_DecodedCodepoint result = {~((MD_u32)0), 1};
    result.codepoint = out[0];
    result.advance = 1;
    if (1 < max && 0xD800 <= out[0] && out[0] < 0xDC00 && 0xDC00 <= out[1] && out[1] < 0xE000)
    {
        result.codepoint = ((out[0] - 0xD800) << 10) | (out[1] - 0xDC00);
        result.advance = 2;
    }
    return(result);
}

MD_FUNCTION MD_u32
MD_Utf8FromCodepoint(MD_u8 *out, MD_u32 codepoint)
{
#define MD_bit8 0x80
    MD_u32 advance = 0;
    if (codepoint <= 0x7F)
    {
        out[0] = (MD_u8)codepoint;
        advance = 1;
    }
    else if (codepoint <= 0x7FF)
    {
        out[0] = (MD_bitmask2 << 6) | ((codepoint >> 6) & MD_bitmask5);
        out[1] = MD_bit8 | (codepoint & MD_bitmask6);
        advance = 2;
    }
    else if (codepoint <= 0xFFFF)
    {
        out[0] = (MD_bitmask3 << 5) | ((codepoint >> 12) & MD_bitmask4);
        out[1] = MD_bit8 | ((codepoint >> 6) & MD_bitmask6);
        out[2] = MD_bit8 | ( codepoint       & MD_bitmask6);
        advance = 3;
    }
    else if (codepoint <= 0x10FFFF)
    {
        out[0] = (MD_bitmask4 << 3) | ((codepoint >> 18) & MD_bitmask3);
        out[1] = MD_bit8 | ((codepoint >> 12) & MD_bitmask6);
        out[2] = MD_bit8 | ((codepoint >>  6) & MD_bitmask6);
        out[3] = MD_bit8 | ( codepoint        & MD_bitmask6);
        advance = 4;
    }
    else
    {
        out[0] = '?';
        advance = 1;
    }
    return(advance);
}

MD_FUNCTION MD_u32
MD_Utf16FromCodepoint(MD_u16 *out, MD_u32 codepoint)
{
    MD_u32 advance = 1;
    if (codepoint == ~((MD_u32)0))
    {
        out[0] = (MD_u16)'?';
    }
    else if (codepoint < 0x10000)
    {
        out[0] = (MD_u16)codepoint;
    }
    else
    {
        MD_u64 v = codepoint - 0x10000;
        out[0] = (MD_u16)(0xD800 + (v >> 10));
        out[1] = 0xDC00 + (v & MD_bitmask10);
        advance = 2;
    }
    return(advance);
}

MD_FUNCTION MD_String8
MD_S8FromS16(MD_Arena *arena, MD_String16 in)
{
    MD_u64 cap = in.size*3;
    MD_u8 *str = MD_PushArrayZero(arena, MD_u8, cap + 1);
    MD_u16 *ptr = in.str;
    MD_u16 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_DecodedCodepoint consume;
    for (;ptr < opl;)
    {
        consume = MD_DecodeCodepointFromUtf16(ptr, opl - ptr);
        ptr += consume.advance;
        size += MD_Utf8FromCodepoint(str + size, consume.codepoint);
    }
    str[size] = 0;
    MD_ArenaPutBack(arena, cap - size); // := ((cap + 1) - (size + 1))
    return(MD_S8(str, size));
}

MD_FUNCTION MD_String16
MD_S16FromS8(MD_Arena *arena, MD_String8 in)
{
    MD_u64 cap = in.size*2;
    MD_u16 *str = MD_PushArrayZero(arena, MD_u16, cap + 1);
    MD_u8 *ptr = in.str;
    MD_u8 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_DecodedCodepoint consume;
    for (;ptr < opl;)
    {
        consume = MD_DecodeCodepointFromUtf8(ptr, opl - ptr);
        ptr += consume.advance;
        size += MD_Utf16FromCodepoint(str + size, consume.codepoint);
    }
    str[size] = 0;
    MD_ArenaPutBack(arena, 2*(cap - size)); // := 2*((cap + 1) - (size + 1))
    MD_String16 result = {0};
    result.str = str;
    result.size = size;
    return(result);
}

MD_FUNCTION MD_String8
MD_S8FromS32(MD_Arena *arena, MD_String32 in)
{
    MD_u64 cap = in.size*4;
    MD_u8 *str = MD_PushArrayZero(arena, MD_u8, cap + 1);
    MD_u32 *ptr = in.str;
    MD_u32 *opl = ptr + in.size;
    MD_u64 size = 0;
    for (;ptr < opl; ptr += 1)
    {
        size += MD_Utf8FromCodepoint(str + size, *ptr);
    }
    str[size] = 0;
    MD_ArenaPutBack(arena, cap - size); // := ((cap + 1) - (size + 1))
    return(MD_S8(str, size));
}

MD_FUNCTION MD_String32
MD_S32FromS8(MD_Arena *arena, MD_String8 in)
{
    MD_u64 cap = in.size;
    MD_u32 *str = MD_PushArrayZero(arena, MD_u32, cap + 1);
    MD_u8 *ptr = in.str;
    MD_u8 *opl = ptr + in.size;
    MD_u64 size = 0;
    MD_DecodedCodepoint consume;
    for (;ptr < opl;)
    {
        consume = MD_DecodeCodepointFromUtf8(ptr, opl - ptr);
        ptr += consume.advance;
        str[size] = consume.codepoint;
        size += 1;
    }
    str[size] = 0;
    MD_ArenaPutBack(arena, 4*(cap - size)); // := 4*((cap + 1) - (size + 1))
    MD_String32 result = {0};
    result.str = str;
    result.size = size;
    return(result);
}

//~ File Name Strings

MD_FUNCTION MD_String8
MD_PathChopLastPeriod(MD_String8 string)
{
    MD_u64 period_pos = MD_S8FindSubstring(string, MD_S8Lit("."), 0, MD_MatchFlag_FindLast);
    if(period_pos < string.size)
    {
        string.size = period_pos;
    }
    return string;
}

MD_FUNCTION MD_String8
MD_PathSkipLastSlash(MD_String8 string)
{
    MD_u64 slash_pos = MD_S8FindSubstring(string, MD_S8Lit("/"), 0,
                                          MD_StringMatchFlag_SlashInsensitive|
                                          MD_MatchFlag_FindLast);
    if(slash_pos < string.size)
    {
        string.str += slash_pos+1;
        string.size -= slash_pos+1;
    }
    return string;
}

MD_FUNCTION MD_String8
MD_PathSkipLastPeriod(MD_String8 string)
{
    MD_u64 period_pos = MD_S8FindSubstring(string, MD_S8Lit("."), 0, MD_MatchFlag_FindLast);
    if(period_pos < string.size)
    {
        string.str += period_pos+1;
        string.size -= period_pos+1;
    }
    return string;
}

MD_FUNCTION MD_String8
MD_PathChopLastSlash(MD_String8 string)
{
    MD_u64 slash_pos = MD_S8FindSubstring(string, MD_S8Lit("/"), 0,
                                          MD_StringMatchFlag_SlashInsensitive|
                                          MD_MatchFlag_FindLast);
    if(slash_pos < string.size)
    {
        string.size = slash_pos;
    }
    return string;
}

MD_FUNCTION MD_String8
MD_S8SkipWhitespace(MD_String8 string)
{
    for(MD_u64 i = 0; i < string.size; i += 1)
    {
        if(!MD_CharIsSpace(string.str[i]))
        {
            string = MD_S8Skip(string, i);
            break;
        }
    }
    return string;
}

MD_FUNCTION MD_String8
MD_S8ChopWhitespace(MD_String8 string)
{
    for(MD_u64 i = string.size-1; i < string.size; i -= 1)
    {
        if(!MD_CharIsSpace(string.str[i]))
        {
            string = MD_S8Prefix(string, i+1);
            break;
        }
    }
    return string;
}

//~ Numeric Strings

MD_GLOBAL MD_u8 md_char_to_value[] = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
    0xFF,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0xFF,
    0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
};

MD_GLOBAL MD_u8 md_char_is_integer[] = {
    0,0,0,0,0,0,1,1,
    1,0,0,0,1,0,0,0,
    0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,
};

MD_FUNCTION MD_b32
MD_StringIsU64(MD_String8 string, MD_u32 radix)
{
    MD_b32 result = 0;
    if (string.size > 0)
    {
        result = 1;
        for (MD_u8 *ptr = string.str, *opl = string.str + string.size;
             ptr < opl;
             ptr += 1)
        {
            MD_u8 c = *ptr;
            if (!md_char_is_integer[c >> 3])
            {
                result = 0;
                break;
            }
            if (md_char_to_value[(c - 0x30)&0x1F] >= radix)
            {
                result = 0;
                break;
            }
        }
    }
    return(result);
}

MD_FUNCTION MD_b32
MD_StringIsCStyleInt(MD_String8 string)
{
    MD_u8 *ptr = string.str;
    MD_u8 *opl = string.str + string.size;
    
    // consume sign
    for (;ptr < opl && (*ptr == '+' || *ptr == '-'); ptr += 1);
    
    // radix from prefix
    MD_u32 radix = 10;
    if (ptr < opl)
    {
        MD_u8 c0 = *ptr;
        if (c0 == '0')
        {
            ptr += 1;
            radix = 8;
            if (ptr < opl)
            {
                MD_u8 c1 = *ptr;
                if (c1 == 'x')
                {
                    ptr += 1;
                    radix = 0x10;
                }
                else if (c1 == 'b')
                {
                    ptr += 1;
                    radix = 2;
                }
            }
        }
    }
    
    // check integer "digits"
    MD_String8 digits_substr = MD_S8Range(ptr, opl);
    MD_b32 result = MD_StringIsU64(digits_substr, radix);
    
    return(result);
}

MD_FUNCTION MD_u64
MD_U64FromString(MD_String8 string, MD_u32 radix)
{
    MD_Assert(2 <= radix && radix <= 16);
    MD_u64 value = 0;
    for (MD_u64 i = 0; i < string.size; i += 1)
    {
        value *= radix;
        MD_u8 c = string.str[i];
        value += md_char_to_value[(c - 0x30)&0x1F];
    }
    return(value);
}

MD_FUNCTION MD_i64
MD_CStyleIntFromString(MD_String8 string)
{
    MD_u64 p = 0;
    
    // consume sign
    MD_i64 sign = +1;
    if (p < string.size)
    {
        MD_u8 c = string.str[p];
        if (c == '-')
        {
            sign = -1;
            p += 1;
        }
        else if (c == '+')
        {
            p += 1;
        }
    }
    
    // radix from prefix
    MD_u32 radix = 10;
    if (p < string.size)
    {
        MD_u8 c0 = string.str[p];
        if (c0 == '0')
        {
            p += 1;
            radix = 8;
            if (p < string.size)
            {
                MD_u8 c1 = string.str[p];
                if (c1 == 'x')
                {
                    p += 1;
                    radix = 16;
                }
                else if (c1 == 'b')
                {
                    p += 1;
                    radix = 2;
                }
            }
        }
    }
    
    // consume integer "digits"
    MD_String8 digits_substr = MD_S8Skip(string, p);
    MD_u64 n = MD_U64FromString(digits_substr, radix);
    
    // combine result
    MD_i64 result = sign*n;
    return(result);
}

MD_FUNCTION MD_f64
MD_F64FromString(MD_String8 string)
{
    char str[64];
    MD_u64 str_size = string.size;
    if (str_size > sizeof(str) - 1)
    {
        str_size = sizeof(str) - 1;
    }
    MD_MemoryCopy(str, string.str, str_size);
    str[str_size] = 0;
    return(atof(str));
}


MD_FUNCTION MD_String8
MD_CStyleHexStringFromU64(MD_Arena *arena, MD_u64 x, MD_b32 caps)
{
    static char md_int_value_to_char[] = "0123456789abcdef";
    MD_u8 buffer[10];
    MD_u8 *opl = buffer + 10;
    MD_u8 *ptr = opl;
    if (x == 0)
    {
        ptr -= 1;
        *ptr = '0';
    }
    else
    {
        for (;;)
        {
            MD_u32 val = x%16;
            x /= 16;
            MD_u8 c = (MD_u8)md_int_value_to_char[val];
            if (caps)
            {
                c = MD_CharToUpper(c);
            }
            ptr -= 1;
            *ptr = c;
            if (x == 0)
            {
                break;
            }
        }
    }
    ptr -= 1;
    *ptr = 'x';
    ptr -= 1;
    *ptr = '0';
    
    MD_String8 result = MD_ZERO_STRUCT;
    result.size = (MD_u64)(ptr - buffer);
    result.str = MD_PushArray(arena, MD_u8, result.size + 1);
    MD_MemoryCopy(result.str, buffer, result.size);
    result.str[result.size] =0;
    return(result);
}

//~ Enum/Flag Strings

MD_FUNCTION MD_String8
MD_StringFromNodeKind(MD_NodeKind kind)
{
    // NOTE(rjf): @maintenance Must be kept in sync with MD_NodeKind enum.
    static char *cstrs[MD_NodeKind_COUNT] =
    {
        "Nil",
        
        "File",
        "ErrorMarker",
        
        "Main",
        "Tag",
        
        "List",
        "Reference",
    };
    return MD_S8CString(cstrs[kind]);
}

MD_FUNCTION MD_String8List
MD_StringListFromNodeFlags(MD_Arena *arena, MD_NodeFlags flags)
{
    // NOTE(rjf): @maintenance Must be kept in sync with MD_NodeFlags enum.
    static char *flag_cstrs[] =
    {
        "HasParenLeft",
        "HasParenRight",
        "HasBracketLeft",
        "HasBracketRight",
        "HasBraceLeft",
        "HasBraceRight",
        
        "IsBeforeSemicolon",
        "IsAfterSemicolon",
        
        "IsBeforeComma",
        "IsAfterComma",
        
        "StringSingleQuote",
        "StringDoubleQuote",
        "StringTick",
        "StringTriplet",
        
        "Numeric",
        "Identifier",
        "StringLiteral",
        "Symbol",
    };
    
    MD_String8List list = MD_ZERO_STRUCT;
    MD_u64 bits = sizeof(flags) * 8;
    for(MD_u64 i = 0; i < bits && i < MD_ArrayCount(flag_cstrs); i += 1)
    {
        if(flags & (1ull << i))
        {
            MD_S8ListPush(arena, &list, MD_S8CString(flag_cstrs[i]));
        }
    }
    return list;
}

//~ Map Table Data Structure

MD_FUNCTION MD_u64
MD_HashStr(MD_String8 string)
{
    MD_u64 result = 5381;
    for(MD_u64 i = 0; i < string.size; i += 1)
    {
        result = ((result << 5) + result) + string.str[i];
    }
    return result;
}

// NOTE(mal): Generic 64-bit hash function (https://nullprogram.com/blog/2018/07/31/)
//            Reversible, so no collisions. Assumes all bits of the pointer matter.
MD_FUNCTION MD_u64 
MD_HashPtr(void *p)
{
    MD_u64 h = (MD_u64)p;
    h = (h ^ (h >> 30)) * 0xbf58476d1ce4e5b9;
    h = (h ^ (h >> 27)) * 0x94d049bb133111eb;
    h = h ^ (h >> 31);
    return h;
}

MD_FUNCTION MD_Map
MD_MapMakeBucketCount(MD_Arena *arena, MD_u64 bucket_count)
{
    MD_Map result = {0};
    result.bucket_count = bucket_count;
    result.buckets = MD_PushArrayZero(arena, MD_MapBucket, bucket_count);
    return(result);
}

MD_FUNCTION MD_Map
MD_MapMake(MD_Arena *arena)
{
    MD_Map result = MD_MapMakeBucketCount(arena, 4093);
    return(result);
}

MD_FUNCTION MD_MapKey
MD_MapKeyStr(MD_String8 string)
{
    MD_MapKey result = {0};
    if (string.size != 0)
    {
        result.hash = MD_HashStr(string);
        result.size = string.size;
        if (string.size > 0)
        {
            result.ptr = string.str;
        }
    }
    return(result);
}

MD_FUNCTION MD_MapKey
MD_MapKeyPtr(void *ptr)
{
    MD_MapKey result = {0};
    if (ptr != 0)
    {
        result.hash = MD_HashPtr(ptr);
        result.size = 0;
        result.ptr = ptr;
    }
    return(result);
}

MD_FUNCTION MD_MapSlot*
MD_MapLookup(MD_Map *map, MD_MapKey key)
{
    MD_MapSlot *result = 0;
    if (map->bucket_count > 0)
    {
        MD_u64 index = key.hash%map->bucket_count;
        result = MD_MapScan(map->buckets[index].first, key);
    }
    return(result);
}

MD_FUNCTION MD_MapSlot*
MD_MapScan(MD_MapSlot *first_slot, MD_MapKey key)
{
    MD_MapSlot *result = 0;
    if (first_slot != 0)
    {
        MD_b32 ptr_kind = (key.size == 0);
        MD_String8 key_string = MD_S8((MD_u8*)key.ptr, key.size);
        for (MD_MapSlot *slot = first_slot;
             slot != 0;
             slot = slot->next)
        {
            if (slot->key.hash == key.hash)
            {
                if (ptr_kind)
                {
                    if (slot->key.size == 0 && slot->key.ptr == key.ptr)
                    {
                        result = slot;
                        break;
                    }
                }
                else
                {
                    MD_String8 slot_string = MD_S8((MD_u8*)slot->key.ptr, slot->key.size);
                    if (MD_S8Match(slot_string, key_string, 0))
                    {
                        result = slot;
                        break;
                    }
                }
            }
        }
    }
    return(result);
}

MD_FUNCTION MD_MapSlot*
MD_MapInsert(MD_Arena *arena, MD_Map *map, MD_MapKey key, void *val)
{
    MD_MapSlot *result = 0;
    if (map->bucket_count > 0)
    {
        MD_u64 index = key.hash%map->bucket_count;
        MD_MapSlot *slot = MD_PushArrayZero(arena, MD_MapSlot, 1);
        MD_MapBucket *bucket = &map->buckets[index];
        MD_QueuePush(bucket->first, bucket->last, slot);
        slot->key = key;
        slot->val = val;
        result = slot;
    }
    return(result);
}

MD_FUNCTION MD_MapSlot*
MD_MapOverwrite(MD_Arena *arena, MD_Map *map, MD_MapKey key, void *val)
{
    MD_MapSlot *result = MD_MapLookup(map, key);
    if (result != 0)
    {
        result->val = val;
    }
    else
    {
        result = MD_MapInsert(arena, map, key, val);
    }
    return(result);
}

//~ Parsing

MD_FUNCTION MD_Token
MD_TokenFromString(MD_String8 string)
{
    MD_Token token = MD_ZERO_STRUCT;
    
    MD_u8 *one_past_last = string.str + string.size;
    MD_u8 *first = string.str;
    
    if(first < one_past_last)
    {
        MD_u8 *at = first;
        MD_u32 skip_n = 0;
        MD_u32 chop_n = 0;
        
#define MD_TokenizerScan(cond) for (; at < one_past_last && (cond); at += 1)
        
        switch (*at)
        {
            // NOTE(allen): Whitespace parsing
            case '\n':
            {
                token.kind = MD_TokenKind_Newline;
                at += 1;
            }break;
            
            case ' ': case '\r': case '\t': case '\f': case '\v':
            {
                token.kind = MD_TokenKind_Whitespace;
                at += 1;
                MD_TokenizerScan(*at == ' ' || *at == '\r' || *at == '\t' || *at == '\f' || *at == '\v');
            }break;
            
            // NOTE(allen): Comment parsing
            case '/':
            {
                if (at + 1 < one_past_last)
                {
                    if (at[1] == '/')
                    {
                        // trim off the first '//'
                        skip_n = 2;
                        at += 2;
                        token.kind = MD_TokenKind_Comment;
                        MD_TokenizerScan(*at != '\n' && *at != '\r');
                    }
                    else if (at[1] == '*')
                    {
                        // trim off the first '/*'
                        skip_n = 2;
                        at += 2;
                        token.kind = MD_TokenKind_BrokenComment;
                        int counter = 1;
                        for (;at < one_past_last && counter > 0; at += 1)
                        {
                            if (at + 1 < one_past_last)
                            {
                                if (at[0] == '*' && at[1] == '/')
                                {
                                    at += 1;
                                    counter -= 1;
                                }
                                else if (at[0] == '/' && at[1] == '*')
                                {
                                    at += 1;
                                    counter += 1;
                                }
                            }
                        }
                        if(counter == 0)
                        {
                            token.kind = MD_TokenKind_Comment;
                            chop_n = 2;
                        }
                    }
                }
                if (token.kind == 0) goto symbol_lex;
            }break;
            
            // NOTE(allen): Strings
            case '"':
            case '\'':
            case '`':
            {
                token.kind = MD_TokenKind_BrokenStringLiteral;
                
                // determine delimiter setup
                MD_u8 d = *at;
                MD_b32 is_triplet = (at + 2 < one_past_last && at[1] == d && at[2] == d);
                
                // lex triple-delimiter string
                if (is_triplet)
                {
                    skip_n = 3;
                    at += 3;
                    MD_u32 consecutive_d = 0;
                    for (;;)
                    {
                        // fail condition
                        if (at >= one_past_last)
                        {
                            break;
                        }
                        
                        if(at[0] == d)
                        {
                            consecutive_d += 1;
                            at += 1;
                            // close condition
                            if (consecutive_d == 3)
                            {
                                chop_n = 3;
                                token.kind = MD_TokenKind_StringLiteral;
                                break;
                            }
                        }
                        else
                        {
                            consecutive_d = 0;
                            
                            // escaping rule
                            if(at[0] == '\\')
                            {
                                at += 1;
                                if(at < one_past_last && (at[0] == d || at[0] == '\\'))
                                {
                                    at += 1;
                                }
                            }
                            else{
                                at += 1;
                            }
                        }
                    }
                }
                
                // lex single-delimiter string
                if (!is_triplet)
                {
                    skip_n = 1;
                    at += 1;
                    for (;at < one_past_last;)
                    {
                        // close condition
                        if (*at == d)
                        {
                            at += 1;
                            chop_n = 1;
                            token.kind = MD_TokenKind_StringLiteral;
                            break;
                        }
                        
                        // fail condition
                        if (*at == '\n')
                        {
                            break;
                        }
                        
                        // escaping rule
                        if (at[0] == '\\')
                        {
                            at += 1;
                            if (at < one_past_last && (at[0] == d || at[0] == '\\'))
                            {
                                at += 1;
                            }
                        }
                        else
                        {
                            at += 1;
                        }
                    }
                }
                
                //- rjf: set relevant node flags on token
                token.node_flags |= MD_NodeFlag_StringLiteral;
                switch(d)
                {
                    case '\'': token.node_flags |= MD_NodeFlag_StringSingleQuote; break;
                    case '"':  token.node_flags |= MD_NodeFlag_StringDoubleQuote; break;
                    case '`':  token.node_flags |= MD_NodeFlag_StringTick; break;
                    default: break;
                }
                if(is_triplet)
                {
                    token.node_flags |= MD_NodeFlag_StringTriplet;
                }
                
            }break;
            
            // NOTE(allen): Identifiers, Numbers, Symbols
            default:
            {
                if (MD_CharIsAlpha(*at) || *at == '_')
                {
                    token.node_flags |= MD_NodeFlag_Identifier;
                    token.kind = MD_TokenKind_Identifier;
                    at += 1;
                    MD_TokenizerScan(MD_CharIsAlpha(*at) || MD_CharIsDigit(*at) || *at == '_');
                }
                
                else if (MD_CharIsDigit(*at))
                {
                    token.node_flags |= MD_NodeFlag_Numeric;
                    token.kind = MD_TokenKind_Numeric;
                    at += 1;
                    
                    for (; at < one_past_last;)
                    {
                        MD_b32 good = 0;
                        if (*at == 'e' || *at == 'E')
                        {
                            good = 1;
                            at += 1;
                            if (at < one_past_last && (*at == '+' || *at == '-'))
                            {
                                at += 1;
                            }
                        }
                        else if (MD_CharIsAlpha(*at) || MD_CharIsDigit(*at) || *at == '.' || *at == '_')
                        {
                            good = 1;
                            at += 1;
                        }
                        if (!good)
                        {
                            break;
                        }
                    }
                }
                
                else if (MD_CharIsUnreservedSymbol(*at))
                {
                    symbol_lex:
                    
                    token.node_flags |= MD_NodeFlag_Symbol;
                    token.kind = MD_TokenKind_Symbol;
                    at += 1;
                    MD_TokenizerScan(MD_CharIsUnreservedSymbol(*at));
                }
                
                else if (MD_CharIsReservedSymbol(*at))
                {
                    token.kind = MD_TokenKind_Reserved;
                    at += 1;
                }
                
                else
                {
                    token.kind = MD_TokenKind_BadCharacter;
                    at += 1;
                }
            }break;
        }
        
        token.raw_string = MD_S8Range(first, at);
        token.string = MD_S8Substring(token.raw_string, skip_n, token.raw_string.size - chop_n);
        
#undef MD_TokenizerScan
        
    }
    
    return token;
}

MD_FUNCTION MD_u64
MD_LexAdvanceFromSkips(MD_String8 string, MD_TokenKind skip_kinds)
{
    MD_u64 result = string.size;
    MD_u64 p = 0;
    for (;;)
    {
        MD_Token token = MD_TokenFromString(MD_S8Skip(string, p));
        if ((skip_kinds & token.kind) == 0)
        {
            result = p;
            break;
        }
        p += token.raw_string.size;
    }
    return(result);
}

MD_FUNCTION MD_ParseResult
MD_ParseResultZero(void)
{
    MD_ParseResult result = MD_ZERO_STRUCT;
    result.node = MD_NilNode();
    return result;
}

MD_FUNCTION MD_ParseResult
MD_ParseNodeSet(MD_Arena *arena, MD_String8 string, MD_u64 offset, MD_Node *parent,
                MD_ParseSetRule rule)
{
    MD_ParseResult result = MD_ParseResultZero();
    MD_u64 off = offset;
    
    //- rjf: fill data from set opener
    MD_Token initial_token = MD_TokenFromString(MD_S8Skip(string, offset));
    MD_u8 set_opener = 0;
    MD_NodeFlags set_opener_flags = 0;
    MD_b32 close_with_brace = 0;
    MD_b32 close_with_paren = 0;
    MD_b32 close_with_separator = 0;
    MD_b32 parse_all = 0;
    switch(rule)
    {
        default: break;
        
        case MD_ParseSetRule_EndOnDelimiter:
        {
            MD_u64 opener_check_off = off;
            opener_check_off += MD_LexAdvanceFromSkips(MD_S8Skip(string, opener_check_off), MD_TokenGroup_Irregular);
            initial_token = MD_TokenFromString(MD_S8Skip(string, opener_check_off));
            if(initial_token.kind == MD_TokenKind_Reserved)
            {
                MD_u8 c = initial_token.raw_string.str[0];
                if(c == '{')
                {
                    set_opener = '{';
                    set_opener_flags |= MD_NodeFlag_HasBraceLeft;
                    opener_check_off += initial_token.raw_string.size;
                    off = opener_check_off;
                    close_with_brace = 1;
                }
                else if(c == '(')
                {
                    set_opener = '(';
                    set_opener_flags |= MD_NodeFlag_HasParenLeft;
                    opener_check_off += initial_token.raw_string.size;
                    off = opener_check_off;
                    close_with_paren = 1;
                }
                else if(c == '[')
                {
                    set_opener = '[';
                    set_opener_flags |= MD_NodeFlag_HasBracketLeft;
                    opener_check_off += initial_token.raw_string.size;
                    off = opener_check_off;
                    close_with_paren = 1;
                }
                else
                {
                    close_with_separator = 1;
                }
            }
            else
            {
                close_with_separator = 1;
            }
        }break;
        
        case MD_ParseSetRule_Global:
        {
            parse_all = 1;
        }break;
    }
    
    //- rjf: fill parent data from opener
    parent->flags |= set_opener_flags;
    
    //- rjf: parse children
    MD_b32 got_closer = 0;
    MD_u64 parsed_child_count = 0;
    if(set_opener != 0 || close_with_separator || parse_all)
    {
        MD_NodeFlags next_child_flags = 0;
        for(;off < string.size;)
        {
            
            //- rjf: check for separator closers
            if(close_with_separator)
            {
                MD_u64 closer_check_off = off;
                
                //- rjf: check newlines
                {
                    MD_Token potential_closer = MD_TokenFromString(MD_S8Skip(string, closer_check_off));
                    if(potential_closer.kind == MD_TokenKind_Newline)
                    {
                        closer_check_off += potential_closer.raw_string.size;
                        // TODO(rjf): As far as I can tell, we can't actually do this,
                        // because higher-level unscoped sets may depend on this newline
                        // so they can be terminated.
                        // off = closer_check_off;
                        
                        // NOTE(rjf): always terminate with a newline if we have >0 children
                        if(parsed_child_count > 0)
                        {
                            // TODO(rjf): As far as I can tell, we can't actually do this,
                            // because higher-level unscoped sets may depend on this newline
                            // so they can be terminated.
                            // off = closer_check_off;
                            got_closer = 1;
                            break;
                        }
                        
                        // NOTE(rjf): terminate after double newline if we have 0 children
                        MD_Token next_closer = MD_TokenFromString(MD_S8Skip(string, closer_check_off));
                        if(next_closer.kind == MD_TokenKind_Newline)
                        {
                            closer_check_off += next_closer.raw_string.size;
                            off = closer_check_off;
                            got_closer = 1;
                            break;
                        }
                    }
                }
                
                //- rjf: check separators and possible braces from higher parents
                {
                    closer_check_off += MD_LexAdvanceFromSkips(MD_S8Skip(string, off), MD_TokenGroup_Irregular);
                    MD_Token potential_closer = MD_TokenFromString(MD_S8Skip(string, closer_check_off));
                    if(potential_closer.kind == MD_TokenKind_Reserved)
                    {
                        MD_u8 c = potential_closer.raw_string.str[0];
                        if(c == ',' || c == ';')
                        {
                            off = closer_check_off;
                            closer_check_off += potential_closer.raw_string.size;
                            break;
                        }
                        else if(c == '}' || c == ']'|| c == ')')
                        {
                            goto end_parse;
                        }
                    }
                }
                
            }
            
            //- rjf: check for non-separator closers
            if(!close_with_separator && !parse_all)
            {
                MD_u64 closer_check_off = off;
                closer_check_off += MD_LexAdvanceFromSkips(MD_S8Skip(string, off), MD_TokenGroup_Irregular);
                MD_Token potential_closer = MD_TokenFromString(MD_S8Skip(string, closer_check_off));
                if(potential_closer.kind == MD_TokenKind_Reserved)
                {
                    MD_u8 c = potential_closer.raw_string.str[0];
                    if(close_with_brace && c == '}')
                    {
                        closer_check_off += potential_closer.raw_string.size;
                        off = closer_check_off;
                        parent->flags |= MD_NodeFlag_HasBraceRight;
                        got_closer = 1;
                        break;
                    }
                    else if(close_with_paren && c == ']')
                    {
                        closer_check_off += potential_closer.raw_string.size;
                        off = closer_check_off;
                        parent->flags |= MD_NodeFlag_HasBracketRight;
                        got_closer = 1;
                        break;
                    }
                    else if(close_with_paren && c == ')')
                    {
                        closer_check_off += potential_closer.raw_string.size;
                        off = closer_check_off;
                        parent->flags |= MD_NodeFlag_HasParenRight;
                        got_closer = 1;
                        break;
                    }
                }
            }
            
            //- rjf: parse next child
            MD_ParseResult child_parse = MD_ParseOneNode(arena, string, off);
            MD_MessageListConcat(&result.errors, &child_parse.errors);
            off += child_parse.string_advance;
            
            //- rjf: hook child into parent
            if(!MD_NodeIsNil(child_parse.node))
            {
                // NOTE(rjf): @error No unnamed set children of implicitly-delimited sets
                if(close_with_separator &&
                   child_parse.node->string.size == 0 &&
                   child_parse.node->flags & (MD_NodeFlag_HasParenLeft    |
                                              MD_NodeFlag_HasParenRight   |
                                              MD_NodeFlag_HasBracketLeft  |
                                              MD_NodeFlag_HasBracketRight |
                                              MD_NodeFlag_HasBraceLeft    |
                                              MD_NodeFlag_HasBraceRight   ))
                {
                    MD_String8 error_str = MD_S8Lit("Unnamed set children of implicitly-delimited sets are not legal.");
                    MD_Message *error = MD_MakeNodeError(arena, child_parse.node, MD_MessageKind_Warning,
                                                         error_str);
                    MD_MessageListPush(&result.errors, error);
                }
                
                MD_PushChild(parent, child_parse.node);
                parsed_child_count += 1;
            }
            
            //- rjf: check trailing separator
            MD_NodeFlags trailing_separator_flags = 0;
            if(!close_with_separator)
            {
                off += MD_LexAdvanceFromSkips(MD_S8Skip(string, off), MD_TokenGroup_Irregular);
                MD_Token trailing_separator = MD_TokenFromString(MD_S8Skip(string, off));
                if (trailing_separator.kind == MD_TokenKind_Reserved)
                {
                    MD_u8 c = trailing_separator.string.str[0];
                    if(c == ',')
                    {
                        trailing_separator_flags |= MD_NodeFlag_IsBeforeComma;
                        off += trailing_separator.raw_string.size;
                    }
                    else if(c == ';')
                    {
                        trailing_separator_flags |= MD_NodeFlag_IsBeforeSemicolon;
                        off += trailing_separator.raw_string.size;
                    }
                }
            }
            
            //- rjf: fill child flags
            child_parse.node->flags |= next_child_flags | trailing_separator_flags;
            
            //- rjf: setup next_child_flags
            next_child_flags = MD_NodeFlag_AfterFromBefore(trailing_separator_flags);
        }
    }
    end_parse:;
    
    //- rjf: push missing closer error, if we have one
    if(set_opener != 0 && got_closer == 0)
    {
        // NOTE(rjf): @error We didn't get a closer for the set
        MD_String8 error_str = MD_S8Fmt(arena, "Unbalanced \"%c\"", set_opener);
        MD_Message *error = MD_MakeTokenError(arena, string, initial_token,
                                              MD_MessageKind_FatalError, error_str);
        MD_MessageListPush(&result.errors, error);
    }
    
    //- rjf: push empty implicit set error,
    if(close_with_separator && parsed_child_count == 0)
    {
        // NOTE(rjf): @error No empty implicitly-delimited sets
        MD_Message *error = MD_MakeTokenError(arena, string, initial_token, MD_MessageKind_Error,
                                              MD_S8Lit("Empty implicitly-delimited node list"));
        MD_MessageListPush(&result.errors, error);
    }
    
    //- rjf: fill result info
    result.node = parent;
    result.string_advance = off - offset;
    
    return result;
}

MD_FUNCTION MD_ParseResult
MD_ParseOneNode(MD_Arena *arena, MD_String8 string, MD_u64 offset)
{
    MD_ParseResult result = MD_ParseResultZero();
    MD_u64 off = offset;
    
    //- rjf: parse pre-comment
    MD_String8 prev_comment = MD_ZERO_STRUCT;
    {
        MD_Token comment_token = MD_ZERO_STRUCT;
        for(;off < string.size;)
        {
            MD_Token token = MD_TokenFromString(MD_S8Skip(string, off));
            if(token.kind == MD_TokenKind_Comment)
            {
                off += token.raw_string.size;
                comment_token = token;
            }
            else if(token.kind == MD_TokenKind_Newline)
            {
                off += token.raw_string.size;
                MD_Token next_token = MD_TokenFromString(MD_S8Skip(string, off));
                if(next_token.kind == MD_TokenKind_Comment)
                {
                    // NOTE(mal): If more than one comment, use the last comment
                    comment_token = next_token;
                }
                else if(next_token.kind == MD_TokenKind_Newline)
                {
                    MD_MemoryZeroStruct(&comment_token);
                }
            }
            else if((token.kind & MD_TokenGroup_Whitespace) != 0)
            {
                off += token.raw_string.size;
            }
            else
            {
                break;
            }
            prev_comment = comment_token.string;
        }
    }
    
    //- rjf: parse tag list
    MD_Node *first_tag = MD_NilNode();
    MD_Node *last_tag = MD_NilNode();
    {
        for(;off < string.size;)
        {
            //- rjf: parse @ symbol, signifying start of tag
            off += MD_LexAdvanceFromSkips(MD_S8Skip(string, off), MD_TokenGroup_Irregular);
            MD_Token next_token = MD_TokenFromString(MD_S8Skip(string, off));
            if(next_token.kind != MD_TokenKind_Reserved ||
               next_token.string.str[0] != '@')
            {
                break;
            }
            off += next_token.raw_string.size;
            
            //- rjf: parse string of tag node
            MD_Token name = MD_TokenFromString(MD_S8Skip(string, off));
            MD_u64 name_off = off;
            if((name.kind & MD_TokenGroup_Label) == 0)
            {
                // NOTE(rjf): @error Improper token for tag string
                MD_String8 error_str = MD_S8Fmt(arena, "\"%.*s\" is not a proper tag label",
                                                MD_S8VArg(name.raw_string));
                MD_Message *error = MD_MakeTokenError(arena, string, name, MD_MessageKind_Error, error_str);
                MD_MessageListPush(&result.errors, error);
                break;
            }
            off += name.raw_string.size;
            
            //- rjf: build tag
            MD_Node *tag = MD_MakeNode(arena, MD_NodeKind_Tag, name.string, name.raw_string, name_off);
            
            //- rjf: parse tag arguments
            MD_Token open_paren = MD_TokenFromString(MD_S8Skip(string, off));
            MD_ParseResult args_parse = MD_ParseResultZero();
            if(open_paren.kind == MD_TokenKind_Reserved &&
               open_paren.string.str[0] == '(')
            {
                args_parse = MD_ParseNodeSet(arena, string, off, tag, MD_ParseSetRule_EndOnDelimiter);
                MD_MessageListConcat(&result.errors, &args_parse.errors);
            }
            off += args_parse.string_advance;
            
            //- rjf: push tag to result
            MD_NodeDblPushBack(first_tag, last_tag, tag);
        }
    }
    
    //- rjf: parse node
    MD_Node *parsed_node = MD_NilNode();
    MD_ParseResult children_parse = MD_ParseResultZero();
    retry:;
    {
        //- rjf: try to parse an unnamed set
        off += MD_LexAdvanceFromSkips(MD_S8Skip(string, off), MD_TokenGroup_Irregular);
        MD_Token unnamed_set_opener = MD_TokenFromString(MD_S8Skip(string, off));
        if(unnamed_set_opener.kind == MD_TokenKind_Reserved)
        {
            MD_u8 c = unnamed_set_opener.string.str[0];
            if (c == '(' || c == '{' || c == '[')
            {
                parsed_node = MD_MakeNode(arena, MD_NodeKind_Main, MD_S8Lit(""), MD_S8Lit(""),
                                          unnamed_set_opener.raw_string.str - string.str);
                children_parse = MD_ParseNodeSet(arena, string, off, parsed_node,
                                                 MD_ParseSetRule_EndOnDelimiter);
                off += children_parse.string_advance;
                MD_MessageListConcat(&result.errors, &children_parse.errors);
            }
            else if (c == ')' || c == '}' || c == ']')
            {
                // NOTE(rjf): @error Unexpected set closing symbol
                MD_String8 error_str = MD_S8Fmt(arena, "Unbalanced \"%c\"", c);
                MD_Message *error = MD_MakeTokenError(arena, string, unnamed_set_opener,
                                                      MD_MessageKind_FatalError, error_str);
                MD_MessageListPush(&result.errors, error);
                off += unnamed_set_opener.raw_string.size;
            }
            else
            {
                // NOTE(rjf): @error Unexpected reserved symbol
                MD_String8 error_str = MD_S8Fmt(arena, "Unexpected reserved symbol \"%c\"", c);
                MD_Message *error = MD_MakeTokenError(arena, string, unnamed_set_opener,
                                                      MD_MessageKind_Error, error_str);
                MD_MessageListPush(&result.errors, error);
                off += unnamed_set_opener.raw_string.size;
            }
            goto end_parse;
            
        }
        
        //- rjf: try to parse regular node, with/without children
        off += MD_LexAdvanceFromSkips(MD_S8Skip(string, off), MD_TokenGroup_Irregular);
        MD_Token label_name = MD_TokenFromString(MD_S8Skip(string, off));
        if((label_name.kind & MD_TokenGroup_Label) != 0)
        {
            off += label_name.raw_string.size;
            parsed_node = MD_MakeNode(arena, MD_NodeKind_Main, label_name.string, label_name.raw_string,
                                      label_name.raw_string.str - string.str);
            parsed_node->flags |= label_name.node_flags;
            
            //- rjf: try to parse children for this node
            MD_u64 colon_check_off = off;
            colon_check_off += MD_LexAdvanceFromSkips(MD_S8Skip(string, colon_check_off), MD_TokenGroup_Irregular);
            MD_Token colon = MD_TokenFromString(MD_S8Skip(string, colon_check_off));
            if(colon.kind == MD_TokenKind_Reserved &&
               colon.string.str[0] == ':')
            {
                colon_check_off += colon.raw_string.size;
                off = colon_check_off;
                
                children_parse = MD_ParseNodeSet(arena, string, off, parsed_node,
                                                 MD_ParseSetRule_EndOnDelimiter);
                off += children_parse.string_advance;
                MD_MessageListConcat(&result.errors, &children_parse.errors);
            }
            goto end_parse;
        }
        
        //- rjf: collect bad token
        MD_Token bad_token = MD_TokenFromString(MD_S8Skip(string, off));
        if(bad_token.kind & MD_TokenGroup_Error)
        {
            off += bad_token.raw_string.size;
            
            switch (bad_token.kind)
            {
                case MD_TokenKind_BadCharacter:
                {
                    MD_String8List bytes = {0};
                    for(int i_byte = 0; i_byte < bad_token.raw_string.size; ++i_byte)
                    {
                        MD_u8 b = bad_token.raw_string.str[i_byte];
                        MD_S8ListPush(arena, &bytes, MD_CStyleHexStringFromU64(arena, b, 1));
                    }
                    
                    MD_StringJoin join = MD_ZERO_STRUCT;
                    join.mid = MD_S8Lit(" ");
                    MD_String8 byte_string = MD_S8ListJoin(arena, bytes, &join);
                    
                    // NOTE(rjf): @error Bad character
                    MD_String8 error_str = MD_S8Fmt(arena, "Non-ASCII character \"%.*s\"",
                                                    MD_S8VArg(byte_string));
                    MD_Message *error = MD_MakeTokenError(arena, string, bad_token, MD_MessageKind_Error,
                                                          error_str);
                    MD_MessageListPush(&result.errors, error);
                }break;
                
                case MD_TokenKind_BrokenComment:
                {
                    // NOTE(rjf): @error Broken Comments
                    MD_Message *error = MD_MakeTokenError(arena, string, bad_token, MD_MessageKind_Error,
                                                          MD_S8Lit("Unterminated comment"));
                    MD_MessageListPush(&result.errors, error);
                }break;
                
                case MD_TokenKind_BrokenStringLiteral:
                {
                    // NOTE(rjf): @error Broken String Literals
                    MD_Message *error = MD_MakeTokenError(arena, string, bad_token, MD_MessageKind_Error,
                                                          MD_S8Lit("Unterminated string literal"));
                    MD_MessageListPush(&result.errors, error);
                }break;
            }
            goto retry;
        }
    }
    
    end_parse:;
    
    //- rjf: parse comments after nodes.
    MD_String8 next_comment = MD_ZERO_STRUCT;
    {
        MD_Token comment_token = MD_ZERO_STRUCT;
        for(;;)
        {
            MD_Token token = MD_TokenFromString(MD_S8Skip(string, off));
            if(token.kind == MD_TokenKind_Comment)
            {
                comment_token = token;
                off += token.raw_string.size;
                break;
            }
            
            else if(token.kind == MD_TokenKind_Newline)
            {
                break;
            }
            else if((token.kind & MD_TokenGroup_Whitespace) != 0)
            {
                off += token.raw_string.size;
            }
            else
            {
                break;
            }
        }
        next_comment = comment_token.string;
    }
    
    //- rjf: fill result
    parsed_node->prev_comment = prev_comment;
    parsed_node->next_comment = next_comment;
    result.node = parsed_node;
    if(!MD_NodeIsNil(result.node))
    {
        result.node->first_tag = first_tag;
        result.node->last_tag = last_tag;
        for(MD_Node *tag = first_tag; !MD_NodeIsNil(tag); tag = tag->next)
        {
            tag->parent = result.node;
        }
    }
    result.string_advance = off - offset;
    
    return result;
}

MD_FUNCTION MD_ParseResult
MD_ParseWholeString(MD_Arena *arena, MD_String8 filename, MD_String8 contents)
{
    MD_Node *root = MD_MakeNode(arena, MD_NodeKind_File, filename, contents, 0);
    MD_ParseResult result = MD_ParseNodeSet(arena, contents, 0, root, MD_ParseSetRule_Global);
    result.node = root;
    for(MD_Message *error = result.errors.first; error != 0; error = error->next)
    {
        if(MD_NodeIsNil(error->node->parent))
        {
            error->node->parent = root;
        }
    }
    return result;
}

MD_FUNCTION MD_ParseResult
MD_ParseWholeFile(MD_Arena *arena, MD_String8 filename)
{
    MD_String8 file_contents = MD_LoadEntireFile(arena, filename);
    MD_ParseResult parse = MD_ParseWholeString(arena, filename, file_contents);
    if(file_contents.str == 0)
    {
        // NOTE(rjf): @error File failing to load
        MD_String8 error_str = MD_S8Fmt(arena, "Could not read file \"%.*s\"", MD_S8VArg(filename));
        MD_Message *error = MD_MakeNodeError(arena, parse.node, MD_MessageKind_FatalError,
                                             error_str);
        MD_MessageListPush(&parse.errors, error);
    }
    return parse;
}

//~ Messages (Errors/Warnings)

MD_FUNCTION MD_Node*
MD_MakeErrorMarkerNode(MD_Arena *arena, MD_String8 parse_contents, MD_u64 offset)
{
    MD_Node *result = MD_MakeNode(arena, MD_NodeKind_ErrorMarker, MD_S8Lit(""), parse_contents,
                                  offset);
    return(result);
}

MD_FUNCTION MD_Message*
MD_MakeNodeError(MD_Arena *arena, MD_Node *node, MD_MessageKind kind, MD_String8 str)
{
    MD_Message *error = MD_PushArrayZero(arena, MD_Message, 1);
    error->node = node;
    error->kind = kind;
    error->string = str;
    return error;
}

MD_FUNCTION MD_Message *
MD_MakeTokenError(MD_Arena *arena, MD_String8 parse_contents, MD_Token token,
                  MD_MessageKind kind, MD_String8 str)
{
    MD_u64 offset = token.raw_string.str - parse_contents.str;
    MD_Node *err_node = MD_MakeErrorMarkerNode(arena, parse_contents, offset);
    return MD_MakeNodeError(arena, err_node, kind, str);
}

MD_FUNCTION void
MD_MessageListPush(MD_MessageList *list, MD_Message *message)
{
    MD_QueuePush(list->first, list->last, message);
    if(message->kind > list->max_message_kind)
    {
        list->max_message_kind = message->kind;
    }
    list->node_count += 1;
}

MD_FUNCTION void
MD_MessageListConcat(MD_MessageList *list, MD_MessageList *to_push)
{
    if(to_push->node_count != 0)
    {
        if(list->last != 0)
        {
            list->last->next = to_push->first;
            list->last = to_push->last;
            list->node_count += to_push->node_count;
            if(to_push->max_message_kind > list->max_message_kind)
            {
                list->max_message_kind = to_push->max_message_kind;
            }
        }
        else
        {
            *list = *to_push;
        }
        MD_MemoryZeroStruct(to_push);
    }
}

//~ Location Conversions

MD_FUNCTION MD_CodeLoc
MD_CodeLocFromFileOffset(MD_String8 filename, MD_u8 *base, MD_u64 offset)
{
    MD_CodeLoc loc;
    loc.filename = filename;
    loc.line = 1;
    loc.column = 1;
    if(base != 0)
    {
        MD_u8 *at = base + offset;
        for(MD_u64 i = 0; base+i < at && base[i]; i += 1)
        {
            if(base[i] == '\n')
            {
                loc.line += 1;
                loc.column = 1;
            }
            else
            {
                loc.column += 1;
            }
        }
    }
    return loc;
}

MD_FUNCTION MD_CodeLoc
MD_CodeLocFromNode(MD_Node *node)
{
    MD_Node *file_root = MD_NilNode();
    for(MD_Node *parent = node->parent; !MD_NodeIsNil(parent); parent = parent->parent)
    {
        if(parent->kind == MD_NodeKind_File)
        {
            file_root = parent;
            break;
        }
    }
    MD_Node *first_tag = file_root->first_tag;
    MD_CodeLoc loc = {0};
    if(MD_NodeIsNil(first_tag))
    {
        loc = MD_CodeLocFromFileOffset(file_root->string, file_root->raw_string.str, node->offset);
    }
    else
    {
        loc = MD_CodeLocFromFileOffset(file_root->string, first_tag->raw_string.str, node->offset);
    }
    return loc;
}

//~ Tree/List Building

MD_FUNCTION MD_b32
MD_NodeIsNil(MD_Node *node)
{
    return(node == 0 || node == &_md_nil_node || node->kind == MD_NodeKind_Nil);
}

MD_FUNCTION MD_Node *
MD_NilNode(void) { return &_md_nil_node; }

MD_FUNCTION MD_Node *
MD_MakeNode(MD_Arena *arena, MD_NodeKind kind, MD_String8 string, MD_String8 raw_string,
            MD_u64 offset)
{
    MD_Node *node = MD_PushArrayZero(arena, MD_Node, 1);
    node->kind = kind;
    node->string = string;
    node->raw_string = raw_string;
    node->next = node->prev = node->parent =
        node->first_child = node->last_child =
        node->first_tag = node->last_tag = node->ref_target = MD_NilNode();
    node->offset = offset;
    return node;
}

MD_FUNCTION void
MD_PushChild(MD_Node *parent, MD_Node *new_child)
{
    if (!MD_NodeIsNil(new_child))
    {
        MD_NodeDblPushBack(parent->first_child, parent->last_child, new_child);
        new_child->parent = parent;
    }
}

MD_FUNCTION void
MD_PushTag(MD_Node *node, MD_Node *tag)
{
    if (!MD_NodeIsNil(tag))
    {
        MD_NodeDblPushBack(node->first_tag, node->last_tag, tag);
        tag->parent = node;
    }
}

MD_FUNCTION MD_Node*
MD_MakeList(MD_Arena *arena)
{
    MD_String8 empty = {0};
    MD_Node *result = MD_MakeNode(arena, MD_NodeKind_List, empty, empty, 0);
    return(result);
}

MD_FUNCTION void
MD_ListConcatInPlace(MD_Node *list, MD_Node *to_push)
{
    if (!MD_NodeIsNil(to_push->first_child))
    {
        if (!MD_NodeIsNil(list->first_child))
        {
            list->last_child->next = to_push->first_child;
            list->last_child = to_push->last_child;
        }
        else
        {
            list->first_child = to_push->first_child;
            list->last_child = to_push->last_child;
        }
        to_push->first_child = to_push->last_child = MD_NilNode();
    }
}

MD_FUNCTION MD_Node*
MD_PushNewReference(MD_Arena *arena, MD_Node *list, MD_Node *target)
{
    MD_Node *n = MD_MakeNode(arena, MD_NodeKind_Reference, target->string, target->raw_string,
                             target->offset);
    n->ref_target = target;
    MD_PushChild(list, n);
    return(n);
}

//~ Introspection Helpers

MD_FUNCTION MD_Node *
MD_FirstNodeWithString(MD_Node *first, MD_String8 string, MD_MatchFlags flags)
{
    MD_Node *result = MD_NilNode();
    for(MD_Node *node = first; !MD_NodeIsNil(node); node = node->next)
    {
        if(MD_S8Match(string, node->string, flags))
        {
            result = node;
            break;
        }
    }
    return result;
}

MD_FUNCTION MD_Node *
MD_NodeAtIndex(MD_Node *first, int n)
{
    MD_Node *result = MD_NilNode();
    if(n >= 0)
    {
        int idx = 0;
        for(MD_Node *node = first; !MD_NodeIsNil(node); node = node->next, idx += 1)
        {
            if(idx == n)
            {
                result = node;
                break;
            }
        }
    }
    return result;
}

MD_FUNCTION MD_Node *
MD_FirstNodeWithFlags(MD_Node *first, MD_NodeFlags flags)
{
    MD_Node *result = MD_NilNode();
    for(MD_Node *n = first; !MD_NodeIsNil(n); n = n->next)
    {
        if(n->flags & flags)
        {
            result = n;
            break;
        }
    }
    return result;
}

MD_FUNCTION int
MD_IndexFromNode(MD_Node *node)
{
    int idx = 0;
    for(MD_Node *last = node->prev; !MD_NodeIsNil(last); last = last->prev, idx += 1);
    return idx;
}

MD_FUNCTION MD_Node *
MD_RootFromNode(MD_Node *node)
{
    MD_Node *parent = node;
    for(MD_Node *p = parent; !MD_NodeIsNil(p); p = p->parent)
    {
        parent = p;
    }
    return parent;
}

MD_FUNCTION MD_Node *
MD_ChildFromString(MD_Node *node, MD_String8 child_string, MD_MatchFlags flags)
{
    return MD_FirstNodeWithString(node->first_child, child_string, flags);
}

MD_FUNCTION MD_Node *
MD_TagFromString(MD_Node *node, MD_String8 tag_string, MD_MatchFlags flags)
{
    return MD_FirstNodeWithString(node->first_tag, tag_string, flags);
}

MD_FUNCTION MD_Node *
MD_ChildFromIndex(MD_Node *node, int n)
{
    return MD_NodeAtIndex(node->first_child, n);
}

MD_FUNCTION MD_Node *
MD_TagFromIndex(MD_Node *node, int n)
{
    return MD_NodeAtIndex(node->first_tag, n);
}

MD_FUNCTION MD_Node *
MD_TagArgFromIndex(MD_Node *node, MD_String8 tag_string, MD_MatchFlags flags, int n)
{
    MD_Node *tag = MD_TagFromString(node, tag_string, flags);
    return MD_ChildFromIndex(tag, n);
}

MD_FUNCTION MD_Node *
MD_TagArgFromString(MD_Node *node, MD_String8 tag_string, MD_MatchFlags tag_str_flags, MD_String8 arg_string, MD_MatchFlags arg_str_flags)
{
    MD_Node *tag = MD_TagFromString(node, tag_string, tag_str_flags);
    MD_Node *arg = MD_ChildFromString(tag, arg_string, arg_str_flags);
    return arg;
}

MD_FUNCTION MD_b32
MD_NodeHasChild(MD_Node *node, MD_String8 string, MD_MatchFlags flags)
{
    return !MD_NodeIsNil(MD_ChildFromString(node, string, flags));
}

MD_FUNCTION MD_b32
MD_NodeHasTag(MD_Node *node, MD_String8 string, MD_MatchFlags flags)
{
    return !MD_NodeIsNil(MD_TagFromString(node, string, flags));
}

MD_FUNCTION MD_i64
MD_ChildCountFromNode(MD_Node *node)
{
    MD_i64 result = 0;
    for(MD_EachNode(child, node->first_child))
    {
        result += 1;
    }
    return result;
}

MD_FUNCTION MD_i64
MD_TagCountFromNode(MD_Node *node)
{
    MD_i64 result = 0;
    for(MD_EachNode(tag, node->first_tag))
    {
        result += 1;
    }
    return result;
}

MD_FUNCTION MD_Node *
MD_ResolveNodeFromReference(MD_Node *node)
{
    MD_u64 safety = 100;
    for(; safety > 0 && node->kind == MD_NodeKind_Reference;
        safety -= 1, node = node->ref_target);
    MD_Node *result = node;
    return(result);
}

MD_FUNCTION MD_Node*
MD_NodeNextWithLimit(MD_Node *node, MD_Node *opl)
{
    node = node->next;
    if (node == opl)
    {
        node = MD_NilNode();
    }
    return(node);
}

MD_FUNCTION MD_String8
MD_PrevCommentFromNode(MD_Node *node)
{
    return(node->prev_comment);
}

MD_FUNCTION MD_String8
MD_NextCommentFromNode(MD_Node *node)
{
    return(node->next_comment);
}

//~ Error/Warning Helpers

MD_FUNCTION MD_String8
MD_StringFromMessageKind(MD_MessageKind kind)
{
    MD_String8 result = MD_ZERO_STRUCT;
    switch (kind)
    {
        default: break;
        case MD_MessageKind_Note:       result = MD_S8Lit("note");        break;
        case MD_MessageKind_Warning:    result = MD_S8Lit("warning");     break;
        case MD_MessageKind_Error:      result = MD_S8Lit("error");       break;
        case MD_MessageKind_FatalError: result = MD_S8Lit("fatal error"); break;
    }
    return(result);
}

MD_FUNCTION MD_String8
MD_FormatMessage(MD_Arena *arena, MD_CodeLoc loc, MD_MessageKind kind, MD_String8 string)
{
    MD_String8 kind_string = MD_StringFromMessageKind(kind);
    MD_String8 result = MD_S8Fmt(arena, "" MD_FmtCodeLoc " %.*s: %.*s\n",
                                 MD_CodeLocVArg(loc), MD_S8VArg(kind_string), MD_S8VArg(string));
    return(result);
}

#if !MD_DISABLE_PRINT_HELPERS

MD_FUNCTION void
MD_PrintMessage(FILE *file, MD_CodeLoc code_loc, MD_MessageKind kind, MD_String8 string)
{
    MD_ArenaTemp scratch = MD_GetScratch(0, 0);
    MD_String8 message = MD_FormatMessage(scratch.arena, code_loc, kind, string);
    fwrite(message.str, message.size, 1, file);
    MD_ReleaseScratch(scratch);
}

MD_FUNCTION void
MD_PrintMessageFmt(FILE *file, MD_CodeLoc code_loc, MD_MessageKind kind, char *fmt, ...)
{
    MD_ArenaTemp scratch = MD_GetScratch(0, 0);
    va_list args;
    va_start(args, fmt);
    MD_String8 string = MD_S8FmtV(scratch.arena, fmt, args);
    va_end(args);
    MD_String8 message = MD_FormatMessage(scratch.arena, code_loc, kind, string);
    fwrite(message.str, message.size, 1, file);
    MD_ReleaseScratch(scratch);
}

#endif

//~ Tree Comparison/Verification

MD_FUNCTION MD_b32
MD_NodeMatch(MD_Node *a, MD_Node *b, MD_MatchFlags flags)
{
    MD_b32 result = 0;
    if(a->kind == b->kind && MD_S8Match(a->string, b->string, flags))
    {
        result = 1;
        if(result && flags & MD_NodeMatchFlag_NodeFlags)
        {
            result = result && a->flags == b->flags;
        }
        if(result && a->kind != MD_NodeKind_Tag && (flags & MD_NodeMatchFlag_Tags))
        {
            for(MD_Node *a_tag = a->first_tag, *b_tag = b->first_tag;
                !MD_NodeIsNil(a_tag) || !MD_NodeIsNil(b_tag);
                a_tag = a_tag->next, b_tag = b_tag->next)
            {
                if(MD_NodeMatch(a_tag, b_tag, flags))
                {
                    if(flags & MD_NodeMatchFlag_TagArguments)
                    {
                        for(MD_Node *a_tag_arg = a_tag->first_child, *b_tag_arg = b_tag->first_child;
                            !MD_NodeIsNil(a_tag_arg) || !MD_NodeIsNil(b_tag_arg);
                            a_tag_arg = a_tag_arg->next, b_tag_arg = b_tag_arg->next)
                        {
                            if(!MD_NodeDeepMatch(a_tag_arg, b_tag_arg, flags))
                            {
                                result = 0;
                                goto end;
                            }
                        }
                    }
                }
                else
                {
                    result = 0;
                    goto end;
                }
            }
        }
    }
    end:;
    return result;
}

MD_FUNCTION MD_b32
MD_NodeDeepMatch(MD_Node *a, MD_Node *b, MD_MatchFlags flags)
{
    MD_b32 result = MD_NodeMatch(a, b, flags);
    if(result)
    {
        for(MD_Node *a_child = a->first_child, *b_child = b->first_child;
            !MD_NodeIsNil(a_child) || !MD_NodeIsNil(b_child);
            a_child = a_child->next, b_child = b_child->next)
        {
            if(!MD_NodeDeepMatch(a_child, b_child, flags))
            {
                result = 0;
                goto end;
            }
        }
    }
    end:;
    return result;
}

//~ Expression Parsing

MD_FUNCTION void
MD_ExprOprPush(MD_Arena *arena, MD_ExprOprList *list,
               MD_ExprOprKind kind, MD_u32 precedence, MD_String8 string,
               MD_u32 op_id, void *op_ptr)
{
    MD_ExprOpr *op = MD_PushArrayZero(arena, MD_ExprOpr, 1);
    MD_QueuePush(list->first, list->last, op);
    list->count += 1;
    op->op_id = op_id;
    op->kind = kind;
    op->precedence = precedence;
    op->string = string;
    op->op_ptr = op_ptr;
}

MD_GLOBAL MD_BakeOperatorErrorHandler md_bake_operator_error_handler = 0;

MD_FUNCTION MD_BakeOperatorErrorHandler
MD_ExprSetBakeOperatorErrorHandler(MD_BakeOperatorErrorHandler handler){
    MD_BakeOperatorErrorHandler old_handler = md_bake_operator_error_handler;
    md_bake_operator_error_handler = handler;
    return old_handler;
}

MD_FUNCTION MD_ExprOprTable
MD_ExprBakeOprTableFromList(MD_Arena *arena, MD_ExprOprList *list)
{
    MD_ExprOprTable result = MD_ZERO_STRUCT;
    
    // TODO(allen): @upgrade_potential(minor)
    
    for(MD_ExprOpr *op = list->first;
        op != 0;
        op = op->next)
    {
        MD_ExprOprKind op_kind = op->kind;
        MD_String8 op_s = op->string;
        
        // error checking
        MD_String8 error_str = MD_ZERO_STRUCT;
        
        MD_Token op_token = MD_TokenFromString(op_s);
        MD_b32 is_setlike_op =
        (op_s.size == 2 &&
         (MD_S8Match(op_s, MD_S8Lit("[]"), 0) || MD_S8Match(op_s, MD_S8Lit("()"), 0) ||
          MD_S8Match(op_s, MD_S8Lit("[)"), 0) || MD_S8Match(op_s, MD_S8Lit("(]"), 0) ||
          MD_S8Match(op_s, MD_S8Lit("{}"), 0)));
        
        if(op_kind != MD_ExprOprKind_Prefix && op_kind != MD_ExprOprKind_Postfix &&
           op_kind != MD_ExprOprKind_Binary && op_kind != MD_ExprOprKind_BinaryRightAssociative)
        {
            error_str = MD_S8Fmt(arena, "Ignored operator \"%.*s\" because its kind value (%d) does not match "
                                 "any valid operator kind", MD_S8VArg(op_s), op_kind);
        }
        else if(is_setlike_op && op_kind != MD_ExprOprKind_Postfix)
        {
            error_str =
                MD_S8Fmt(arena, "Ignored operator \"%.*s\". \"%.*s\" is only allowed as unary postfix",
                         MD_S8VArg(op_s), MD_S8VArg(op_s));
        }
        else if(!is_setlike_op &&
                (op_token.kind != MD_TokenKind_Identifier && op_token.kind != MD_TokenKind_Symbol))
        {
            error_str = MD_S8Fmt(arena, "Ignored operator \"%.*s\" because it is neither a symbol "
                                 "nor an identifier token", MD_S8VArg(op_s));
        }
        else if(!is_setlike_op && op_token.string.size < op_s.size)
        {
            error_str = MD_S8Fmt(arena, "Ignored operator \"%.*s\" because its prefix \"%.*s\" "
                                 "constitutes a standalone operator",
                                 MD_S8VArg(op_s), MD_S8VArg(op_token.string));
        }
        else
        {
            for(MD_ExprOpr *op2 = list->first;
                op2 != op;
                op2 = op2->next)
            {   // NOTE(mal): O(n^2)
                MD_ExprOprKind op2_kind = op2->kind;
                MD_String8 op2_s = op2->string;
                if(op->precedence == op2->precedence && 
                   ((op_kind == MD_ExprOprKind_Binary &&
                     op2_kind == MD_ExprOprKind_BinaryRightAssociative) ||
                    (op_kind == MD_ExprOprKind_BinaryRightAssociative &&
                     op2_kind == MD_ExprOprKind_Binary)))
                {
                    error_str =
                        MD_S8Fmt(arena, "Ignored binary operator \"%.*s\" because another binary operator"
                                 "has the same precedence and different associativity", MD_S8VArg(op_s));
                }
                else if(MD_S8Match(op_s, op2_s, 0))
                {
                    if(op_kind == op2_kind)
                    {
                        error_str = MD_S8Fmt(arena, "Ignored repeat operator \"%.*s\"", MD_S8VArg(op_s));
                    }
                    else if(op_kind != MD_ExprOprKind_Prefix && op2_kind != MD_ExprOprKind_Prefix)
                    {
                        error_str =
                            MD_S8Fmt(arena, "Ignored conflicting repeat operator \"%.*s\". There can't be"
                                     "more than one posfix/binary operator associated to the same token", 
                                     MD_S8VArg(op_s));
                    }
                }
            }
        }
        
        // save error
        if(error_str.size != 0 && md_bake_operator_error_handler)
        {
            md_bake_operator_error_handler(MD_MessageKind_Warning, error_str);
        }
        
        // save list
        else
        {
            MD_ExprOprList *saved_list = result.table + op_kind;
            MD_ExprOpr *op_node_copy = MD_PushArray(arena, MD_ExprOpr, 1);
            *op_node_copy = *op;
            MD_QueuePush(saved_list->first, saved_list->last, op_node_copy);
            saved_list->count += 1;
        }
    }
    
    return(result);
}

MD_FUNCTION MD_ExprOpr*
MD_ExprOprFromKindString(MD_ExprOprTable *table, MD_ExprOprKind kind, MD_String8 s)
{
    // TODO(allen): @upgrade_potential
    
    // NOTE(mal): Look for operator on one or all (kind == MD_ExprOprKind_Null) tables
    MD_ExprOpr *result = 0;
    for(MD_ExprOprKind cur_kind = (MD_ExprOprKind)(MD_ExprOprKind_Null + 1);
        cur_kind < MD_ExprOprKind_COUNT;
        cur_kind = (MD_ExprOprKind)(cur_kind + 1))
    {
        if(kind == MD_ExprOprKind_Null || kind == cur_kind)
        {
            MD_ExprOprList *op_list = table->table+cur_kind;
            for(MD_ExprOpr *op = op_list->first;
                op != 0;
                op = op->next)
            {
                if(MD_S8Match(op->string, s, 0))
                {
                    result = op;
                    goto dbl_break;
                }
            }
        }
    }
    dbl_break:;
    return result;
}

MD_FUNCTION MD_ExprParseResult
MD_ExprParse(MD_Arena *arena, MD_ExprOprTable *op_table, MD_Node *first, MD_Node *opl)
{
    // setup a context
    MD_ExprParseCtx ctx = MD_ExprParse_MakeContext(op_table);
    
    // parse the top level
    MD_Expr *expr = MD_ExprParse_TopLevel(arena, &ctx, first, opl);
    
    // fill result
    MD_ExprParseResult result = {0};
    result.expr = expr;
    result.errors = ctx.errors;
    return(result);
}

MD_FUNCTION MD_Expr*
MD_Expr_NewLeaf(MD_Arena *arena, MD_Node *node)
{
    MD_Expr *result = MD_PushArrayZero(arena, MD_Expr, 1);
    result->md_node = node;
    return(result);
}

MD_FUNCTION MD_Expr*
MD_Expr_NewOpr(MD_Arena *arena, MD_ExprOpr *op, MD_Node *op_node, MD_Expr *l, MD_Expr *r)
{
    MD_Expr *result = MD_PushArrayZero(arena, MD_Expr, 1);
    result->op = op;
    result->md_node = op_node;
    result->parent = 0;
    result->left = l;
    result->right = r;
    if (l != 0)
    {
        MD_Assert(l->parent == 0);
        l->parent = result;
    }
    if(r != 0)
    {
        MD_Assert(r->parent == 0);
        r->parent = result;
    }
    return(result);
}

MD_FUNCTION MD_ExprParseCtx
MD_ExprParse_MakeContext(MD_ExprOprTable *op_table)
{
    MD_ExprParseCtx result = MD_ZERO_STRUCT;
    result.op_table = op_table;
    
    result.accel.postfix_set_ops[0] = MD_ExprOprFromKindString(op_table, MD_ExprOprKind_Postfix, MD_S8Lit("()"));
    result.accel.postfix_set_flags[0] = MD_NodeFlag_HasParenLeft | MD_NodeFlag_HasParenRight;
    
    result.accel.postfix_set_ops[1] = MD_ExprOprFromKindString(op_table, MD_ExprOprKind_Postfix, MD_S8Lit("[]"));
    result.accel.postfix_set_flags[1] = MD_NodeFlag_HasBracketLeft | MD_NodeFlag_HasBracketRight;
    
    result.accel.postfix_set_ops[2] = MD_ExprOprFromKindString(op_table, MD_ExprOprKind_Postfix, MD_S8Lit("{}"));
    result.accel.postfix_set_flags[2] = MD_NodeFlag_HasBraceLeft | MD_NodeFlag_HasBraceRight;
    
    result.accel.postfix_set_ops[3] = MD_ExprOprFromKindString(op_table, MD_ExprOprKind_Postfix, MD_S8Lit("[)"));
    result.accel.postfix_set_flags[3] = MD_NodeFlag_HasBracketLeft | MD_NodeFlag_HasParenRight;
    
    result.accel.postfix_set_ops[4] = MD_ExprOprFromKindString(op_table, MD_ExprOprKind_Postfix, MD_S8Lit("(]"));
    result.accel.postfix_set_flags[4] = MD_NodeFlag_HasParenLeft | MD_NodeFlag_HasBracketRight;
    
    return(result);
}

MD_FUNCTION MD_Expr*
MD_ExprParse_TopLevel(MD_Arena *arena, MD_ExprParseCtx *ctx, MD_Node *first, MD_Node *opl)
{
    // parse the node range
    MD_Node *iter = first;
    MD_Expr *expr = MD_ExprParse_MinPrecedence(arena, ctx, &iter, first, opl, 0);
    
    // check for failed-to-reach-end error
    if(ctx->errors.max_message_kind == MD_MessageKind_Null)
    {
        MD_Node *stop_node = iter;
        if(!MD_NodeIsNil(stop_node))
        {
            MD_String8 error_str = MD_S8Lit("Expected binary or unary postfix operator.");
            MD_Message *error = MD_MakeNodeError(arena,stop_node,MD_MessageKind_FatalError,error_str);
            MD_MessageListPush(&ctx->errors, error);
        }
    }
    
    return(expr);
}

MD_FUNCTION MD_b32
MD_ExprParse_OprConsume(MD_ExprParseCtx *ctx, MD_Node **iter, MD_Node *opl,
                        MD_ExprOprKind kind, MD_u32 min_precedence, MD_ExprOpr **op_out)
{
    MD_b32 result = 0;
    MD_Node *node = *iter;
    if(!MD_NodeIsNil(node))
    {
        MD_ExprOpr *op = MD_ExprOprFromKindString(ctx->op_table, kind, node->string);
        if(op != 0 && op->precedence >= min_precedence)
        {
            result = 1;
            *op_out = op;
            *iter= MD_NodeNextWithLimit(*iter, opl);
        }
    }
    return result;
}

MD_FUNCTION MD_Expr*
MD_ExprParse_Atom(MD_Arena *arena, MD_ExprParseCtx *ctx, MD_Node **iter,
                  MD_Node *first, MD_Node *opl)
{
    // TODO(allen): nil
    MD_Expr* result = 0;
    
    MD_Node *node = *iter;
    MD_ExprOpr *op = 0;
    
    if(MD_NodeIsNil(node))
    {
        MD_Node *last = first;
        for (;last->next != opl; last = last->next);
        
        MD_Node *error_node = last->next;
        if (MD_NodeIsNil(error_node))
        {
            MD_Node *root = MD_RootFromNode(node);
            MD_String8 parse_contents = root->raw_string;
            MD_u64 offset = last->offset + last->raw_string.size;
            error_node = MD_MakeErrorMarkerNode(arena, parse_contents, offset);
        }
        
        MD_String8 error_str = MD_S8Lit("Unexpected end of expression.");
        MD_Message *error = MD_MakeNodeError(arena, error_node, MD_MessageKind_FatalError,
                                             error_str);
        MD_MessageListPush(&ctx->errors, error);
    }
    else if((node->flags & MD_NodeFlag_HasParenLeft) &&
            (node->flags & MD_NodeFlag_HasParenRight))
    { // NOTE(mal): Parens
        *iter = MD_NodeNextWithLimit(*iter, opl);
        result = MD_ExprParse_TopLevel(arena, ctx, node->first_child, MD_NilNode());
    }
    else if(((node->flags & MD_NodeFlag_HasBraceLeft)   && (node->flags & MD_NodeFlag_HasBraceRight))   ||
            ((node->flags & MD_NodeFlag_HasBracketLeft) && (node->flags & MD_NodeFlag_HasBracketRight)) ||
            ((node->flags & MD_NodeFlag_HasBracketLeft) && (node->flags & MD_NodeFlag_HasParenRight))   ||
            ((node->flags & MD_NodeFlag_HasParenLeft)   && (node->flags & MD_NodeFlag_HasBracketRight)))
    { // NOTE(mal): Unparsed leaf sets ({...}, [...], [...), (...])
        *iter = MD_NodeNextWithLimit(*iter, opl);
        result = MD_Expr_NewLeaf(arena, node);
    }
    else if(MD_ExprParse_OprConsume(ctx, iter, opl, MD_ExprOprKind_Prefix, 1, &op))
    {
        MD_u32 min_precedence = op->precedence + 1;
        MD_Expr *sub_expr =
            MD_ExprParse_MinPrecedence(arena, ctx, iter, first, opl, min_precedence);
        if(ctx->errors.max_message_kind == MD_MessageKind_Null)
        {
            result = MD_Expr_NewOpr(arena, op, node, sub_expr, 0);
        }
    }
    else if(MD_ExprParse_OprConsume(ctx, iter, opl, MD_ExprOprKind_Null, 1, &op))
    {
        MD_String8 error_str = MD_S8Fmt(arena, "Expected leaf. Got operator \"%.*s\".", MD_S8VArg(node->string));
        
        MD_Message *error = MD_MakeNodeError(arena, node, MD_MessageKind_FatalError, error_str);
        MD_MessageListPush(&ctx->errors, error);
    }
    else if(node->flags &
            (MD_NodeFlag_HasParenLeft|MD_NodeFlag_HasParenRight|MD_NodeFlag_HasBracketLeft|
             MD_NodeFlag_HasBracketRight|MD_NodeFlag_HasBraceLeft|MD_NodeFlag_HasBraceRight))
    {
        MD_String8 error_str = MD_S8Fmt(arena, "Unexpected set.", MD_S8VArg(node->string));
        MD_Message *error = MD_MakeNodeError(arena, node, MD_MessageKind_FatalError, error_str);
        MD_MessageListPush(&ctx->errors, error);
    }
    else{   // NOTE(mal): leaf
        *iter = MD_NodeNextWithLimit(*iter, opl);
        result = MD_Expr_NewLeaf(arena, node);
    }
    
    return(result);
}

MD_FUNCTION MD_Expr*
MD_ExprParse_MinPrecedence(MD_Arena *arena, MD_ExprParseCtx *ctx,
                           MD_Node **iter, MD_Node *first, MD_Node *opl,
                           MD_u32 min_precedence)
{
    // TODO(allen): nil
    MD_Expr* result = 0;
    
    result = MD_ExprParse_Atom(arena, ctx, iter, first, opl);
    if(ctx->errors.max_message_kind == MD_MessageKind_Null)
    {
        for (;!MD_NodeIsNil(*iter);)
        {
            MD_Node *node = *iter;
            MD_ExprOpr *op = 0;
            
            if(MD_ExprParse_OprConsume(ctx, iter, opl, MD_ExprOprKind_Binary,
                                       min_precedence, &op) ||
               MD_ExprParse_OprConsume(ctx, iter, opl, MD_ExprOprKind_BinaryRightAssociative,
                                       min_precedence, &op))
            {
                MD_u32 next_min_precedence = op->precedence + (op->kind == MD_ExprOprKind_Binary);
                MD_Expr *sub_expr =
                    MD_ExprParse_MinPrecedence(arena, ctx, iter, first, opl, next_min_precedence);
                if(ctx->errors.max_message_kind == MD_MessageKind_Null)
                {
                    result = MD_Expr_NewOpr(arena, op, node, result, sub_expr);
                }
                else{
                    break;
                }
            }
            
            else
            {
                MD_b32 found_postfix_setlike_operator = 0;
                for(MD_u32 i_op = 0;
                    i_op < MD_ArrayCount(ctx->accel.postfix_set_ops);
                    ++i_op)
                {
                    MD_ExprOpr *op2 = ctx->accel.postfix_set_ops[i_op];
                    if(op2 && op2->precedence >= min_precedence &&
                       node->flags == ctx->accel.postfix_set_flags[i_op])
                    {
                        *iter = MD_NodeNextWithLimit(*iter, opl);
                        result = MD_Expr_NewOpr(arena, op2, node, result, 0);
                        found_postfix_setlike_operator = 1;
                        break;
                    }
                }
                
                if(!found_postfix_setlike_operator)
                {
                    if(MD_ExprParse_OprConsume(ctx, iter, opl, MD_ExprOprKind_Postfix,
                                               min_precedence, &op))
                    {
                        result = MD_Expr_NewOpr(arena, op, node, result, 0);
                    }
                    else
                    {
                        break;  // NOTE: Due to lack of progress
                    }
                }
                
            }
            
        }
    }
    
    return(result);
}




//~ String Generation

MD_FUNCTION void
MD_DebugDumpFromNode(MD_Arena *arena, MD_String8List *out, MD_Node *node,
                     int indent, MD_String8 indent_string, MD_GenerateFlags flags)
{
#define MD_PrintIndent(_indent_level) do\
{\
for(int i = 0; i < (_indent_level); i += 1)\
{\
MD_S8ListPush(arena, out, indent_string);\
}\
}while(0)
    
    //- rjf: prev-comment
    if(flags & MD_GenerateFlag_Comments && node->prev_comment.size != 0)
    {
        MD_PrintIndent(indent);
        MD_S8ListPush(arena, out, MD_S8Lit("/*\n"));
        MD_PrintIndent(indent);
        MD_S8ListPush(arena, out, node->prev_comment);
        MD_PrintIndent(indent);
        MD_S8ListPush(arena, out, MD_S8Lit("\n"));
        MD_PrintIndent(indent);
        MD_S8ListPush(arena, out, MD_S8Lit("*/\n"));
    }
    
    //- rjf: tags of node
    if(flags & MD_GenerateFlag_Tags)
    {
        for(MD_EachNode(tag, node->first_tag))
        {
            MD_PrintIndent(indent);
            MD_S8ListPush(arena, out, MD_S8Lit("@"));
            MD_S8ListPush(arena, out, tag->string);
            if(flags & MD_GenerateFlag_TagArguments && !MD_NodeIsNil(tag->first_child))
            {
                int tag_arg_indent = (int)(indent + 1 + tag->string.size + 1);
                MD_S8ListPush(arena, out, MD_S8Lit("("));
                for(MD_EachNode(child, tag->first_child))
                {
                    int child_indent = tag_arg_indent;
                    if(MD_NodeIsNil(child->prev))
                    {
                        child_indent = 0;
                    }
                    MD_DebugDumpFromNode(arena, out, child, child_indent, MD_S8Lit(" "), flags);
                    if(!MD_NodeIsNil(child->next))
                    {
                        MD_S8ListPush(arena, out, MD_S8Lit(",\n"));
                    }
                }
                MD_S8ListPush(arena, out, MD_S8Lit(")\n"));
            }
            else
            {
                MD_S8ListPush(arena, out, MD_S8Lit("\n"));
            }
        }
    }
    
    //- rjf: node kind
    if(flags & MD_GenerateFlag_NodeKind)
    {
        MD_PrintIndent(indent);
        MD_S8ListPush(arena, out, MD_S8Lit("// kind: \""));
        MD_S8ListPush(arena, out, MD_StringFromNodeKind(node->kind));
        MD_S8ListPush(arena, out, MD_S8Lit("\"\n"));
    }
    
    //- rjf: node flags
    if(flags & MD_GenerateFlag_NodeFlags)
    {
        MD_PrintIndent(indent);
        MD_ArenaTemp scratch = MD_GetScratch(&arena, 1);
        MD_String8List flag_strs = MD_StringListFromNodeFlags(scratch.arena, node->flags);
        MD_StringJoin join = { MD_S8LitComp(""), MD_S8LitComp("|"), MD_S8LitComp("") };
        MD_String8 flag_str = MD_S8ListJoin(arena, flag_strs, &join);
        MD_S8ListPush(arena, out, MD_S8Lit("// flags: \""));
        MD_S8ListPush(arena, out, flag_str);
        MD_S8ListPush(arena, out, MD_S8Lit("\"\n"));
        MD_ReleaseScratch(scratch);
    }
    
    //- rjf: location
    if(flags & MD_GenerateFlag_Location)
    {
        MD_PrintIndent(indent);
        MD_CodeLoc loc = MD_CodeLocFromNode(node);
        MD_String8 string = MD_S8Fmt(arena, "// location: %.*s:%i:%i\n", MD_S8VArg(loc.filename), (int)loc.line, (int)loc.column);
        MD_S8ListPush(arena, out, string);
    }
    
    //- rjf: name of node
    if(node->string.size != 0)
    {
        MD_PrintIndent(indent);
        if(node->kind == MD_NodeKind_File)
        {
            MD_S8ListPush(arena, out, MD_S8Lit("`"));
            MD_S8ListPush(arena, out, node->string);
            MD_S8ListPush(arena, out, MD_S8Lit("`"));
        }
        else
        {
            MD_S8ListPush(arena, out, node->raw_string);
        }
    }
    
    //- rjf: children list
    if(flags & MD_GenerateFlag_Children && !MD_NodeIsNil(node->first_child))
    {
        if(node->string.size != 0)
        {
            MD_S8ListPush(arena, out, MD_S8Lit(":\n"));
        }
        MD_PrintIndent(indent);
        MD_S8ListPush(arena, out, MD_S8Lit("{\n"));
        for(MD_EachNode(child, node->first_child))
        {
            MD_DebugDumpFromNode(arena, out, child, indent+1, indent_string, flags);
            MD_S8ListPush(arena, out, MD_S8Lit(",\n"));
        }
        MD_PrintIndent(indent);
        MD_S8ListPush(arena, out, MD_S8Lit("}"));
    }
    
    //- rjf: next-comment
    if(flags & MD_GenerateFlag_Comments && node->next_comment.size != 0)
    {
        MD_PrintIndent(indent);
        MD_S8ListPush(arena, out, MD_S8Lit("\n/*\n"));
        MD_PrintIndent(indent);
        MD_S8ListPush(arena, out, node->next_comment);
        MD_PrintIndent(indent);
        MD_S8ListPush(arena, out, MD_S8Lit("\n"));
        MD_PrintIndent(indent);
        MD_S8ListPush(arena, out, MD_S8Lit("*/\n"));
    }
    
#undef MD_PrintIndent
}

MD_FUNCTION void
MD_ReconstructionFromNode(MD_Arena *arena, MD_String8List *out, MD_Node *node,
                          int indent, MD_String8 indent_string)
{
    MD_CodeLoc code_loc = MD_CodeLocFromNode(node);
    
#define MD_PrintIndent(_indent_level) do\
{\
for(int i = 0; i < (_indent_level); i += 1)\
{\
MD_S8ListPush(arena, out, indent_string);\
}\
}while(0)
    
    //- rjf: prev-comment
    if(node->prev_comment.size != 0)
    {
        MD_String8 comment = MD_S8SkipWhitespace(MD_S8ChopWhitespace(node->prev_comment));
        MD_b32 requires_multiline = MD_S8FindSubstring(comment, MD_S8Lit("\n"), 0, 0) < comment.size;
        MD_PrintIndent(indent);
        if(requires_multiline)
        {
            MD_S8ListPush(arena, out, MD_S8Lit("/*\n"));
        }
        else
        {
            MD_S8ListPush(arena, out, MD_S8Lit("// "));
        }
        MD_S8ListPush(arena, out, comment);
        if(requires_multiline)
        {
            MD_S8ListPush(arena, out, MD_S8Lit("\n*/\n"));
        }
        else
        {
            MD_S8ListPush(arena, out, MD_S8Lit("\n"));
        }
    }
    
    //- rjf: tags of node
    MD_u32 tag_first_line = MD_CodeLocFromNode(node->first_tag).line;
    MD_u32 tag_last_line = tag_first_line;
    {
        for(MD_EachNode(tag, node->first_tag))
        {
            MD_u32 tag_line = MD_CodeLocFromNode(tag).line;
            if(tag_line != tag_last_line)
            {
                MD_S8ListPush(arena, out, MD_S8Lit("\n"));
                tag_last_line = tag_line;
            }
            else if(!MD_NodeIsNil(tag->prev))
            {
                MD_S8ListPush(arena, out, MD_S8Lit(" "));
            }
            
            MD_PrintIndent(indent);
            MD_S8ListPush(arena, out, MD_S8Lit("@"));
            MD_S8ListPush(arena, out, tag->string);
            if(!MD_NodeIsNil(tag->first_child))
            {
                int tag_arg_indent = (int)(indent + 1 + tag->string.size + 1);
                MD_S8ListPush(arena, out, MD_S8Lit("("));
                MD_u32 last_line = MD_CodeLocFromNode(tag).line;
                for(MD_EachNode(child, tag->first_child))
                {
                    MD_CodeLoc child_loc = MD_CodeLocFromNode(child);
                    if(child_loc.line != last_line)
                    {
                        MD_S8ListPush(arena, out, MD_S8Lit("\n"));
                        MD_PrintIndent(indent);
                    }
                    last_line = child_loc.line;
                    
                    int child_indent = tag_arg_indent;
                    if(MD_NodeIsNil(child->prev))
                    {
                        child_indent = 0;
                    }
                    MD_ReconstructionFromNode(arena, out, child, child_indent, MD_S8Lit(" "));
                    if(!MD_NodeIsNil(child->next))
                    {
                        MD_S8ListPush(arena, out, MD_S8Lit(",\n"));
                    }
                }
                MD_S8ListPush(arena, out, MD_S8Lit(")"));
            }
        }
    }
    
    //- rjf: name of node
    if(node->string.size != 0)
    {
        if(tag_first_line != tag_last_line)
        {
            MD_S8ListPush(arena, out, MD_S8Lit("\n"));
            MD_PrintIndent(indent);
        }
        else if(!MD_NodeIsNil(node->first_tag) || !MD_NodeIsNil(node->prev))
        {
            MD_S8ListPush(arena, out, MD_S8Lit(" "));
        }
        if(node->kind == MD_NodeKind_File)
        {
            MD_S8ListPush(arena, out, MD_S8Lit("`"));
            MD_S8ListPush(arena, out, node->string);
            MD_S8ListPush(arena, out, MD_S8Lit("`"));
        }
        else
        {
            MD_S8ListPush(arena, out, node->raw_string);
        }
    }
    
    //- rjf: children list
    if(!MD_NodeIsNil(node->first_child))
    {
        if(node->string.size != 0)
        {
            MD_S8ListPush(arena, out, MD_S8Lit(":"));
        }
        
        // rjf: figure out opener/closer symbols
        MD_u8 opener_char = 0;
        MD_u8 closer_char = 0;
        if(node->flags & MD_NodeFlag_HasParenLeft)        { opener_char = '('; }
        else if(node->flags & MD_NodeFlag_HasBracketLeft) { opener_char = '['; }
        else if(node->flags & MD_NodeFlag_HasBraceLeft)   { opener_char = '{'; }
        if(node->flags & MD_NodeFlag_HasParenRight)       { closer_char = ')'; }
        else if(node->flags & MD_NodeFlag_HasBracketRight){ closer_char = ']'; }
        else if(node->flags & MD_NodeFlag_HasBraceRight)  { closer_char = '}'; }
        
        MD_b32 multiline = 0;
        for(MD_EachNode(child, node->first_child))
        {
            MD_CodeLoc child_loc = MD_CodeLocFromNode(child);
            if(child_loc.line != code_loc.line)
            {
                multiline = 1;
                break;
            }
        }
        
        if(opener_char != 0)
        {
            if(multiline)
            {
                MD_S8ListPush(arena, out, MD_S8Lit("\n"));
                MD_PrintIndent(indent);
            }
            else
            {
                MD_S8ListPush(arena, out, MD_S8Lit(" "));
            }
            MD_S8ListPush(arena, out, MD_S8(&opener_char, 1));
            if(multiline)
            {
                MD_S8ListPush(arena, out, MD_S8Lit("\n"));
                MD_PrintIndent(indent+1);
            }
        }
        MD_u32 last_line = MD_CodeLocFromNode(node->first_child).line;
        for(MD_EachNode(child, node->first_child))
        {
            int child_indent = 0;
            MD_CodeLoc child_loc = MD_CodeLocFromNode(child);
            if(child_loc.line != last_line)
            {
                MD_S8ListPush(arena, out, MD_S8Lit("\n"));
                MD_PrintIndent(indent);
                child_indent = indent+1;
            }
            last_line = child_loc.line;
            MD_ReconstructionFromNode(arena, out, child, child_indent, indent_string);
        }
        MD_PrintIndent(indent);
        if(closer_char != 0)
        {
            if(last_line != code_loc.line)
            {
                MD_S8ListPush(arena, out, MD_S8Lit("\n"));
                MD_PrintIndent(indent);
            }
            else
            {
                MD_S8ListPush(arena, out, MD_S8Lit(" "));
            }
            MD_S8ListPush(arena, out, MD_S8(&closer_char, 1));
        }
    }
    
    //- rjf: trailing separator symbols
    if(node->flags & MD_NodeFlag_IsBeforeSemicolon)
    {
        MD_S8ListPush(arena, out, MD_S8Lit(";"));
    }
    else if(node->flags & MD_NodeFlag_IsBeforeComma)
    {
        MD_S8ListPush(arena, out, MD_S8Lit(","));
    }
    
    //- rjf: next-comment
    // TODO(rjf): @node_comments
    if(node->next_comment.size != 0)
    {
        MD_String8 comment = MD_S8SkipWhitespace(MD_S8ChopWhitespace(node->next_comment));
        MD_b32 requires_multiline = MD_S8FindSubstring(comment, MD_S8Lit("\n"), 0, 0) < comment.size;
        MD_PrintIndent(indent);
        if(requires_multiline)
        {
            MD_S8ListPush(arena, out, MD_S8Lit("/*\n"));
        }
        else
        {
            MD_S8ListPush(arena, out, MD_S8Lit("// "));
        }
        MD_S8ListPush(arena, out, comment);
        if(requires_multiline)
        {
            MD_S8ListPush(arena, out, MD_S8Lit("\n*/\n"));
        }
        else
        {
            MD_S8ListPush(arena, out, MD_S8Lit("\n"));
        }
    }
    
#undef MD_PrintIndent
}


#if !MD_DISABLE_PRINT_HELPERS
MD_FUNCTION void
MD_PrintDebugDumpFromNode(FILE *file, MD_Node *node, MD_GenerateFlags flags)
{
    MD_ArenaTemp scratch = MD_GetScratch(0, 0);
    MD_String8List list = {0};
    MD_DebugDumpFromNode(scratch.arena, &list, node,
                         0, MD_S8Lit(" "), flags);
    MD_String8 string = MD_S8ListJoin(scratch.arena, list, 0);
    fwrite(string.str, string.size, 1, file);
    MD_ReleaseScratch(scratch);
}
#endif


//~ Command Line Argument Helper

MD_FUNCTION MD_String8List
MD_StringListFromArgCV(MD_Arena *arena, int argument_count, char **arguments)
{
    MD_String8List options = MD_ZERO_STRUCT;
    for(int i = 1; i < argument_count; i += 1)
    {
        MD_S8ListPush(arena, &options, MD_S8CString(arguments[i]));
    }
    return options;
}

MD_FUNCTION MD_CmdLine
MD_MakeCmdLineFromOptions(MD_Arena *arena, MD_String8List options)
{
    MD_CmdLine cmdln = MD_ZERO_STRUCT;
    MD_b32 parsing_only_inputs = 0;
    
    for(MD_String8Node *n = options.first, *next = 0;
        n; n = next)
    {
        next = n->next;
        
        //- rjf: figure out whether or not this is an option by checking for `-` or `--`
        // from the beginning of the string
        MD_String8 option_name = MD_ZERO_STRUCT;
        if(MD_S8Match(n->string, MD_S8Lit("--"), 0))
        {
            parsing_only_inputs = 1;
        }
        else if(MD_S8Match(MD_S8Prefix(n->string, 2), MD_S8Lit("--"), 0))
        {
            option_name = MD_S8Skip(n->string, 2);
        }
        else if(MD_S8Match(MD_S8Prefix(n->string, 1), MD_S8Lit("-"), 0))
        {
            option_name = MD_S8Skip(n->string, 1);
        }
        
        //- rjf: trim off anything after a `:` or `=`, use that as the first value string
        MD_String8 first_value = MD_ZERO_STRUCT;
        MD_b32 has_many_values = 0;
        if(option_name.size != 0)
        {
            MD_u64 colon_signifier_pos = MD_S8FindSubstring(option_name, MD_S8Lit(":"), 0, 0);
            MD_u64 equal_signifier_pos = MD_S8FindSubstring(option_name, MD_S8Lit("="), 0, 0);
            MD_u64 signifier_pos = MD_Min(colon_signifier_pos, equal_signifier_pos);
            if(signifier_pos < option_name.size)
            {
                first_value = MD_S8Skip(option_name, signifier_pos+1);
                option_name = MD_S8Prefix(option_name, signifier_pos);
                if(MD_S8Match(MD_S8Suffix(first_value, 1), MD_S8Lit(","), 0))
                {
                    has_many_values = 1;
                }
            }
        }
        
        //- rjf: gather arguments
        if(option_name.size != 0 && !parsing_only_inputs)
        {
            MD_String8List option_values = MD_ZERO_STRUCT;
            
            //- rjf: push first value
            if(first_value.size != 0)
            {
                MD_S8ListPush(arena, &option_values, first_value);
            }
            
            //- rjf: scan next string values, add them to option values until we hit a lack
            // of a ',' between values
            if(has_many_values)
            {
                for(MD_String8Node *v = next; v; v = v->next, next = v)
                {
                    MD_String8 value_str = v->string;
                    MD_b32 next_has_arguments = MD_S8Match(MD_S8Suffix(value_str, 1), MD_S8Lit(","), 0);
                    MD_b32 in_quotes = 0;
                    MD_u64 start = 0;
                    for(MD_u64 i = 0; i <= value_str.size; i += 1)
                    {
                        if(i == value_str.size || (value_str.str[i] == ',' && in_quotes == 0))
                        {
                            if(start != i)
                            {
                                MD_S8ListPush(arena, &option_values, MD_S8Substring(value_str, start, i));
                            }
                            start = i+1;
                        }
                        else if(value_str.str[i] == '"')
                        {
                            in_quotes = !in_quotes;
                        }
                    }
                    if(next_has_arguments == 0)
                    {
                        break;
                    }
                }
            }
            
            //- rjf: insert the fully parsed option
            {
                MD_CmdLineOption *opt = MD_PushArrayZero(arena, MD_CmdLineOption, 1);
                opt->name = option_name;
                opt->values = option_values;
                if(cmdln.last_option == 0)
                {
                    cmdln.first_option = cmdln.last_option = opt;
                }
                else
                {
                    cmdln.last_option->next = opt;
                    cmdln.last_option = cmdln.last_option->next;
                }
            }
        }
        
        //- rjf: this argument is not an option, push it to regular inputs list.
        else
        {
            MD_S8ListPush(arena, &cmdln.inputs, n->string);
        }
    }
    
    return cmdln;
}

MD_FUNCTION MD_String8List
MD_CmdLineValuesFromString(MD_CmdLine cmdln, MD_String8 name)
{
    MD_String8List values = MD_ZERO_STRUCT;
    for(MD_CmdLineOption *opt = cmdln.first_option; opt; opt = opt->next)
    {
        if(MD_S8Match(opt->name, name, 0))
        {
            values = opt->values;
            break;
        }
    }
    return values;
}

MD_FUNCTION MD_b32
MD_CmdLineB32FromString(MD_CmdLine cmdln, MD_String8 name)
{
    MD_b32 result = 0;
    for(MD_CmdLineOption *opt = cmdln.first_option; opt; opt = opt->next)
    {
        if(MD_S8Match(opt->name, name, 0))
        {
            result = 1;
            break;
        }
    }
    return result;
}

MD_FUNCTION MD_i64
MD_CmdLineI64FromString(MD_CmdLine cmdln, MD_String8 name)
{
    MD_String8List values = MD_CmdLineValuesFromString(cmdln, name);
    MD_ArenaTemp scratch = MD_GetScratch(0, 0);
    MD_String8 value_str = MD_S8ListJoin(scratch.arena, values, 0);
    MD_i64 result = MD_CStyleIntFromString(value_str);
    MD_ReleaseScratch(scratch);
    return(result);
}

//~ File System

MD_FUNCTION MD_String8
MD_LoadEntireFile(MD_Arena *arena, MD_String8 filename)
{
    MD_String8 result = MD_ZERO_STRUCT;
#if defined(MD_IMPL_LoadEntireFile)
    result = MD_IMPL_LoadEntireFile(arena, filename);
#endif
    return(result);
}

MD_FUNCTION MD_b32
MD_FileIterBegin(MD_FileIter *it, MD_String8 path)
{
#if !defined(MD_IMPL_FileIterBegin)
    return(0);
#else
    return(MD_IMPL_FileIterBegin(it, path));
#endif
}

MD_FUNCTION MD_FileInfo
MD_FileIterNext(MD_Arena *arena, MD_FileIter *it)
{
#if !defined(MD_IMPL_FileIterNext)
    MD_FileInfo result = {0};
    return(result);
#else
    return(MD_IMPL_FileIterNext(arena, it));
#endif
}

MD_FUNCTION void
MD_FileIterEnd(MD_FileIter *it)
{
#if defined(MD_IMPL_FileIterEnd)
    MD_IMPL_FileIterEnd(it);
#endif
}

#endif // MD_C

/*
Copyright 2021 Dion Systems LLC

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
