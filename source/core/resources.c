#include "resources.h"
#include "base/base.h"
#include "os/os.h"

#if defined(BACKEND_GL46)
#  include "impl/gl46_resources.c"
#elif defined(BACKEND_GL33)
#  include "impl/gl33_resources.c"
#endif
