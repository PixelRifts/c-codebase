/* date = July 3rd 2022 11:22 am */

#ifndef LOG_H
#define LOG_H

#include "defines.h"

#if defined(_DEBUG)
#  define Log(format, ...) Statement(\
printf("Info: ");\
printf(format, __VA_ARGS__);\
printf("\n");\
)
#  define LogError(format, ...) Statement(\
printf("%s:%d: Error: ", FILE_NAME, __LINE__);\
printf(format, __VA_ARGS__);\
printf("\n");\
)
#  define LogReturn(ret, format, ...) Statement(\
printf("%s:%d: Error: ", FILE_NAME, __LINE__);\
printf(format, __VA_ARGS__);\
printf("\n");\
return ret;\
)
#  define LogFatal(format, ...) Statement(\
printf("%s:%d: Error: ", FILE_NAME, __LINE__);\
printf(format, __VA_ARGS__);\
printf("\n");\
exit(-1);\
)
#  define AssertTrue(c, format, ...) Statement(\
if (!(c)) {\
printf("%s:%d: Error: ", FILE_NAME, __LINE__);\
printf("Assertion Failure: ");\
printf(format, __VA_ARGS__);\
printf("\n");\
}\
)
#else
#  define Log(format, ...) Statement()
#  define LogError(format, ...) Statement()
#  define LogReturn(ret, format, ...) Statement()
#  define LogFatal(format, ...) Statement()
#  define AssertTrue(c, format, ...) Statement()
#endif

#endif //LOG_H
