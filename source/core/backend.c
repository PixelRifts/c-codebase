#include "backend.h"

#if defined(BACKEND_GL46)

#  if defined(PLATFORM_WIN)
#    include "impl/win32_gl46_backend.c"
#  elif defined(PLATFORM_LINUX)
#    include "impl/linux_gl46_backend.c"
#  endif // WINDOWS

#elif defined(BACKEND_GL33)

#  if defined(PLATFORM_WIN)
#    include "impl/win32_gl33_backend.c"
#  elif defined(PLATFORM_LINUX)
#    include "impl/linux_gl33_backend.c"
#  endif // WINDOWS

#elif defined(BACKEND_D3D11)

#  if defined(PLATFORM_WIN)
#    include "impl/win32_d3d11_backend.c"
#  elif defined(PLATFORM_LINUX)
#    error The D3D11 Backend is not supported for linux
#  endif // WINDOWS

#endif // Backends
