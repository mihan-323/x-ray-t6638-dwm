#include "common.h"

uniform float3x4 m_xform;
uniform float3x4 m_xform_v;
uniform float4 consts; // {1/quant,1/quant,???,???}
uniform float4 c_scale, c_bias, wind, wave;

#ifdef USE_AREF
	v2p_shadow_direct_aref main ( v_shadow_direct_aref I )
#else
	v2p_shadow_direct main ( v_shadow_direct I )
#endif
	{
	#ifdef USE_AREF
		v2p_shadow_direct_aref 	O;
	#else
		v2p_shadow_direct 		O;
	#endif

	// Transform to world coords
	float3 	pos	= mul		(m_xform , I.P);

	// 
	float 	base 	= m_xform._24;			// take base height from matrix
	float 	dp		= calc_cyclic  (wave.w+dot(pos,(float3)wave));
	float 	H 		= pos.y - base;			// height of vertex (scaled, rotated, etc.)
	float 	inten 	= H * dp;			// intensity
	float2 	result;

	#ifdef USE_TREEWAVE
		result	= 0;
	#else
		#ifdef USE_AREF
			float 	frac 	= I.tc.z*consts.x;		// fractional (or rigidity)
		#else
			float 	frac 	= 0;
		#endif

		result	= calc_xz_wave	(wind.xz*inten, frac);
	#endif

	float4 	f_pos 	= float4(pos.x+result.x, pos.y, pos.z+result.y, 1);

	O.hpos 	= mul		(m_VP,	f_pos	);

	#ifdef	USE_AREF
		O.tc0 	= (I.tc * consts).xy;
	#endif

	#ifdef NEED_SHADOW_MULTISAMPLING
		update_taa_vertex(O.hpos);
		O.depth = mul( m_V, f_pos ).z;
	#endif

 	return	O;
}