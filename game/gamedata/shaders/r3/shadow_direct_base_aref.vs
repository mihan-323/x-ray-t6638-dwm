#include "common.h"

struct 	a2v
{
	float4	P	: POSITION;		// Object-space position
 	int2	tc0	: TEXCOORD0;	// Texture coordinates
};

v2p_shadow_direct_aref main ( v_static I )
{
	v2p_shadow_direct_aref 		O;
	O.hpos 	= mul				(m_WVP,	I.P		);
	O.tc0 	= unpack_tc_base	(I.tc,I.T.w,I.B.w	);	// copy tc

	#ifdef NEED_SHADOW_MULTISAMPLING
		update_taa_vertex(O.hpos);
		O.depth = mul( m_WV, I.P ).z;
	#endif

 	return	O;
}