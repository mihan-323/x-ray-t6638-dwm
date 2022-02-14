#include "common.h"

struct v2p_smap
{
	float2 tc : TEXCOORD0;
	float4 hpos : SV_Position;
};

v2p_smap main(v_static I)
{
	v2p_smap O;
	O.tc = unpack_tc_base(I.tc, I.T.w, I.B.w);
	O.hpos = mul(m_WVP,	I.P);
	
 	return O;
}