#if defined(BACKEND_GL33) || defined(BACKEND_GL46)
#  include "core/impl/gl_functions.c"
#elif defined(BACKEND_D3D11)
#  include "core/impl/d3d11_functions.c"
#endif
