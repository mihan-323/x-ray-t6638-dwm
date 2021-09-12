#include "common.h"

uniform float3x4 m_xform;
uniform float4 consts;

v2p_rsmap main(v_rsmap_aref I)
{
	v2p_rsmap O;

	float3 w_pos = mul(m_xform , I.P);

	I.Nh = unpack_D3DCOLOR(I.Nh);

	O.P = w_pos;
	O.N = mul(m_xform, unpack_bx2(I.Nh.xyz));
	O.tc0 = (I.tc * consts).xy;
	O.hpos = mul(m_VP, float4(w_pos, 1));

 	return	O;
}