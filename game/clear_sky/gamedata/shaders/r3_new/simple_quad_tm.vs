#include "common.h"

void main
(
	in float3 P : POSITION,
	in float2 tc : TEXCOORD0,
	out float4 tc_tm : TEXCOORD0,
	out float4 pos2d : SV_Position
)
{
	float	scale	= s_tonemap.Load( int3(0,0,0) );
	tc_tm = float4(tc, scale, scale);
	pos2d.xy = P.xy * screen_res.zw * 2 - 1;
	pos2d = float4(pos2d.x, -pos2d.y, 0, 1);
}