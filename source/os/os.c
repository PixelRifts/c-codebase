#include "defines.h"

#include "os.h"

#if defined(PLATFORM_WIN)
#include "impl/win32_os.c"
#elif defined(PLATFORM_LINUX)
#include "impl/linux_os.c"
#endif
