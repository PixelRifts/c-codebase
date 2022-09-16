/* date = September 27th 2021 11:48 am */

#ifndef DEFINES_H
#define DEFINES_H

#include <stdio.h>

// Unsigned int types.
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types.
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

// Regular int types.
typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

// Floating point types
typedef float f32;
typedef double f64;

// Boolean types
typedef u8  b8;
typedef u32 b32;

// Void function type
typedef void void_func(void);
typedef void (*void_func_ptr)(void);

#define true 1
#define false 0

#define null 0
#define u32_max 4294967295

#ifndef __cplusplus
#define nullptr (void*)0
#endif

#if defined(__clang__)
#  define COMPILER_CLANG
#elif defined(_MSC_VER)
#  define COMPILER_CL
#elif defined(__GNUC__)
#  define COMPILER_GCC
#endif

#if defined(COMPILER_CLANG)
#  define FILE_NAME __FILE_NAME__
#else
#  define FILE_NAME __FILE__
#endif

#define Statement(s) do {\
s\
} while (0)

#define flush fflush(stdout)
#define trace Statement(printf("%s:%d: Trace\n", FILE_NAME, __LINE__); flush;)
#define unreachable Statement(\
printf("How did we get here? In %s on line %d\n", FILE_NAME, __LINE__);\
flush;\
)

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#  define PLATFORM_WIN
#elif defined(__linux__) || defined(__gnu_linux__)
#  define PLATFORM_LINUX
#else
#  error "The compiler only supports windows and linux for now"
#endif
#define PATH_MAX 4096

#ifdef PLATFORM_WIN
#  include <direct.h>
#  define get_cwd _getcwd
#elif defined(PLATFORM_LINUX)
#  include <unistd.h>
#  define get_cwd getcwd
#endif

// NOTE(voxel): Confirm gcc version works
#if defined(COMPILER_CL) || defined(COMPILER_CLANG)
#  define dll_export __declspec(dllexport)
#  define dll_import __declspec(dllimport)
#elif defined (COMPILER_GCC)
#  define dll_export __attribute__((dllexport))
#  define dll_import __attribute__((dllimport))
#else
#  error dll_export not defined for this compiler
#endif

#ifdef PLUGIN
#  define dll_plugin_api
#else
#  define dll_plugin_api dll_export
#endif

#define Gigabytes(count) (u64) (count * 1024 * 1024 * 1024)
#define Megabytes(count) (u64) (count * 1024 * 1024)
#define Kilobytes(count) (u64) (count * 1024)

#define Min(a,b) (((a)<(b))?(a):(b))
#define Max(a,b) (((a)>(b))?(a):(b))
#define Clamp(a,x,b) (((x)<(a))?(a):((b)<(x))?(b):(x))
#define ClampTop(a,b) Min(a,b)
#define ClampBot(a,b) Max(a,b)
#define ReverseClamp(a,x,b) (((x)<(a))?(b):((b)<(x))?(a):(x))
#define Wrap(a,x,b) ReverseClamp(a,x,b)

#define MemoryCopy(d,s,z) memmove((d), (s), (z))
#define MemoryCopyStruct(d,s) MemoryCopy((d),(s), Min(sizeof(*(d)) , sizeof(*(s))))
#define MemoryZero(d,z) memset((d), 0, (z))
#define MemoryZeroStruct(d,s) MemoryZero((d),sizeof(s))

#define ArrayCount(a) (sizeof(a) / sizeof(a[0]))

#define Slice_Prototype(type) typedef struct type##_slice { type* elems; u32 len; } type##_slice;
#define slice(type) type##_slice

#define Iterate(array, var) for (int var = 0; var < array.len; var++)
#define IteratePtr(array, var) for (int var = 0; var < array->len; var++)

#endif //DEFINES_H