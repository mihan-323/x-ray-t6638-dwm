#include "common.h"

uniform float3x4 m_xform;
uniform float4 consts;

struct v2p_smap
{
	float2 tc : TEXCOORD0;
	float4 hpos : SV_Position;
};

v2p_smap main(v_rsmap_aref I)
{
	float3 w_pos = mul(m_xform , I.P);

	v2p_smap O;
	O.tc = (I.tc * consts).xy;
	O.hpos = mul(m_VP, float4(w_pos, 1));

 	return	O;
}