#define USE_TDETAIL
#undef DX11_STATIC_DEFFERED_RENDERER
#if defined(DX11_STATIC_DISABLE_BUMP_MAPPING)
	#include "deffer_base_flat.vs"
#else
	#include "deffer_base_bump.vs"
#endif
