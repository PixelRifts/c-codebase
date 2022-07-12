#if defined(BACKEND_GL46) || defined(BACKEND_GL33)
#  include <glad/glad.h>
#endif

#include "backend.h"

#if defined(BACKEND_GL46)
#  include "impl/gl46_backend.c"
#elif defined(BACKEND_GL33)
#  include "impl/gl33_backend.c"
#endif
