#include "resources.h"
#include "base/base.h"
#include "os/os.h"


#if defined(BACKEND_GL46)
#  include "impl/gl46_resources.c"

// @testing
void R_Texture2DWhite(R_Texture2D* texture) {
	R_Texture2DAlloc(texture, TextureFormat_RGBA, 1, 1, TextureResize_Nearest, TextureResize_Nearest, TextureWrap_ClampToEdge, TextureWrap_ClampToEdge);
	u8 data[] = { 255, 255, 255, 255 };
	R_Texture2DData(texture, data);
}

#elif defined(BACKEND_GL33)
#  include "impl/gl33_resources.c"

// @testing
void R_Texture2DWhite(R_Texture2D* texture) {
	R_Texture2DAlloc(texture, TextureFormat_RGBA, 1, 1, TextureResize_Nearest, TextureResize_Nearest, TextureWrap_ClampToEdge, TextureWrap_ClampToEdge);
	u8 data[] = { 255, 255, 255, 255 };
	R_Texture2DData(texture, data);
}

#elif defined(BACKEND_D3D11)
#  include "impl/d3d11_resources.c"

#endif
