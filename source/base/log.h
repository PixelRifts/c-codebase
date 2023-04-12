/* date = July 3rd 2022 11:22 am */

#ifndef LOG_H
#define LOG_H

#include "defines.h"

#if defined(_DEBUG)
#  define Log(format, ...) Statement(\
printf("Info: ");\
printf(format, ##__VA_ARGS__);\
printf("\n");\
flush;\
)
#  define LogError(format, ...) Statement(\
printf("%s:%d: Error: ", FILE_NAME, __LINE__);\
printf(format, ##__VA_ARGS__);\
printf("\n");\
flush;\
)
#  define LogReturn(ret, format, ...) Statement(\
printf("%s:%d: Error: ", FILE_NAME, __LINE__);\
printf(format, ##__VA_ARGS__);\
printf("\n");\
flush;\
return ret;\
)
#  define LogFatal(format, ...) Statement(\
printf("%s:%d: Error: ", FILE_NAME, __LINE__);\
printf(format, ##__VA_ARGS__);\
printf("\n");\
flush;\
exit(-1);\
)
// TODO(voxel): assert should actually do a debug break.
#  define AssertTrue(c, format, ...) Statement(\
if (!(c)) {\
printf("%s:%d: Error: ", FILE_NAME, __LINE__);\
printf("Assertion Failure: ");\
printf(format, ##__VA_ARGS__);\
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
