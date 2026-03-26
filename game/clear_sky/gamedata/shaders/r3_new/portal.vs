#include "common.h"

struct	v_vert
{
	float4 	pos	: POSITION;	// (float,float,float,1)
	float4	color	: COLOR0;	// (r,g,b,dir-occlusion)
};
struct 	v2p
{
	float4 c	: COLOR0;
	float  fog	: FOG;
	float4 hpos	: SV_Position;
};

v2p main (v_vert v)
{
	v2p 		o;

	o.hpos		= mul(m_VP, v.pos);				// xform, input in world coords
	o.c			= v.color;
	o.fog 		= calc_fogging(v.pos);			// fog, input in world coords
	o.fog 		= saturate(o.fog);
	o.c.rgb 	= lerp(fog_color.xyz, o.c.xyz, o.fog);

	o.c.a		= o.fog;

	float scale = s_tonemap.Load(int3(0,0,0));
	o.c.rgb *= scale;
	
	return o;
	
}
