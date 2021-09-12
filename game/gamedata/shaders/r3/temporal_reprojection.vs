#include "common.h"

void main
(
	in float3 P : POSITION,
	inout float2 tc : TEXCOORD0,
	out float2 tc0 : TEXCOORD1,
	out float4 pos2d : SV_Position
)
{
	pos2d.xy = P.xy * screen_res.zw * 2 - 1;
	pos2d = float4(pos2d.x, -pos2d.y, 0, 1);
	tc0 = tc;
	update_taa_tc(tc0);
}