#include "common.h"

struct 	a2v
{
	float4 P:	 	POSITION;	// Object-space position
};

v2p_shadow_direct_aref main ( a2v I )
{
	v2p_shadow_direct_aref	O=init_v2p_shadow_direct_aref();

	O.hpos 	= mul		(m_WVP,	I.P	);

	#ifdef NEED_SHADOW_MULTISAMPLING
		update_taa_vertex(O.hpos);
		O.depth = mul( m_WV, I.P ).z;
	#endif

 	return	O;
}