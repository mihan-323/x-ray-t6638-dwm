#include "common.h"

struct vi
{
	float4	p		: POSITION;
	float4	c		: COLOR0;
	float3	tc0		: TEXCOORD0;
	float3	tc1		: TEXCOORD1;
};

struct v2p
{
	float4	c		: COLOR0;
	float3	tc0		: TEXCOORD0;
	float3	tc1		: TEXCOORD1;
	float4	hpos	: SV_Position;
};

v2p main (vi v)
{
	v2p o;

	float4 tpos = mul(G_BUFFER::sky, v.p);

	o.hpos = mul(m_WVP, tpos); // xform, input in world coords, G_BUFFER::sky - magic number
	o.hpos.z = o.hpos.w;

	o.tc0 = v.tc0; // copy tc
	o.tc1 = v.tc1; // copy tc

	o.c = v.c;

	float	scale	= s_tonemap.Load( int3(0,0,0) );
	o.c.rgb *= scale;
	
	return o;
}