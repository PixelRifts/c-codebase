/* date = July 3rd 2022 11:22 am */

#ifndef LOG_H
#define LOG_H

#include "defines.h"

#define Log(format, ...) Statement(\
printf("Info: ");\
printf(format, __VA_ARGS__);\
printf("\n");\
)
#define LogError(format, ...) Statement(\
printf("%s:%d: Error: ", FILE_NAME, __LINE__);\
printf(format, __VA_ARGS__);\
printf("\n");\
)
#define LogReturn(ret, format, ...) Statement(\
printf("%s:%d: Error: ", FILE_NAME, __LINE__);\
printf(format, __VA_ARGS__);\
printf("\n");\
return ret;\
)
#define LogFatal(format, ...) Statement(\
printf("%s:%d: Error: ", FILE_NAME, __LINE__);\
printf(format, __VA_ARGS__);\
printf("\n");\
exit(-1);\
)

#endif //LOG_H
