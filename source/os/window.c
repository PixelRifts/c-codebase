#include "defines.h"
#include "base/base.h"

#include "window.h"

#if defined(PLATFORM_WIN)
#include "impl/win32_window.c"
#elif defined(PLATFORM_LINUX)
#include "impl/x11_window.c"
#endif
