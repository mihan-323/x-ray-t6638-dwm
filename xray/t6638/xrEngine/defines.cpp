#include "stdafx.h"

#ifdef DEBUG
	ECORE_API BOOL bDebug	= FALSE;
#endif



u32			psCurrentVidMode[2] = {1024,768};
u32			psCurrentBPP		= 32;

Flags32		psDeviceFlags		= { rsFullscreen|rsDetails|mtPhysics|mtSound|mtNetwork|rsDrawStatic|rsDrawDynamic|rsRefresh60hz};

int			psTextureLOD		= 1;
