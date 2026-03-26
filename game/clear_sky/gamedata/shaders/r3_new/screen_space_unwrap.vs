#include "common.h"

// DWM Team
// Обновление 23.09.2020

void main
(
	in		float4 iP	: POSITIONT,
	inout	float2 uv	: TEXCOORD0,
	out		float4 HPos	: SV_Position
)
{
	HPos = float4((iP.xy + float2(0.5, 0.5)) * screen_res.zw, iP.zw);
	HPos.xy = (HPos.xy * 2 - 1) * float2(1, -1);
}