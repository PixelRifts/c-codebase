#include "backend.h"

#if defined(BACKEND_GL46)
#  if defined(PLATFORM_WIN)
#    include "impl/win32_gl46_backend.c"
#  endif // WINDOWS

#elif defined(BACKEND_GL33)
#  if defined(PLATFORM_WIN)
#    include "impl/win32_gl33_backend.c"
#  endif // WINDOWS

#endif // Backends
