#include "d3d11_functions.h"

#if defined(DXGI_FUNCTIONS)

//#  define X(Name, Return, Args) DXGI_##Name##_Func* Name;
//DXGI_FUNCTIONS
//#  undef X
//
void __LoadDXGIFunctions(void) {
	//#define X(Name, Return, Args) \
	//if (!Name) Name = (DXGI_##Name##_Func*) GetProcAddress(#Name);
	//
	//DXGI_FUNCTIONS
	//
	//#undef X
}

#endif // defined(DXGI_FUNCTIONS)