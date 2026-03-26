#pragma once

#ifdef CLEAR_SKY_BUILD
//#include "light.h"
class light;
#else
#include "light.h"
#endif

class	light_Package
{
public:
	xr_vector<light*>		v_point;
	xr_vector<light*>		v_spot;
	xr_vector<light*>		v_shadowed;
public:
	void					clear				();
	void					sort				();
};
