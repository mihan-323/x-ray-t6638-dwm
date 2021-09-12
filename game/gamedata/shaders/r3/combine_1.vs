#include "common.h"

struct 	v2p
{
	float4	tc0	: TEXCOORD0;	// tc.xy, tc.w = tonemap scale
	float4	hpos: SV_Position;
};

v2p main(float4 P : POSITIONT)
{
	v2p O;

	float	scale	= s_tonemap.Load( int3(0,0,0) );
	O.tc0 = float4(P.zw, scale, scale);

	O.hpos = float4(P.x, -P.y, 0, 1);

 	return	O;
}