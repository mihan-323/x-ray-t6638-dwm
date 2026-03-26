#include "common.h"

v2p_rsmap main(v_static I)
{
	v2p_rsmap O;

#ifndef USE_VSM
	I.Nh = unpack_D3DCOLOR(I.Nh);
	O.N = mul(m_W, unpack_bx2(I.Nh.xyz));
#endif

	O.P = mul(m_W, I.P);
	O.tc0 = unpack_tc_base(I.tc, I.T.w, I.B.w);
	O.hpos = mul(m_WVP,	I.P);

 	return O;
}