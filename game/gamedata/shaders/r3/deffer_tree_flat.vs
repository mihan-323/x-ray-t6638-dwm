#include "common.h"

uniform float3x4	m_xform;
uniform float3x4	m_xform_v;
uniform float4 		consts; 	// {1/quant,1/quant,???,???}
uniform float4 		c_scale,c_bias,wind,wave;
uniform float2 		c_sun;		// x=*, y=+

v2p_flat main (v_tree I)
{
	I.Nh	=	unpack_D3DCOLOR(I.Nh);
	I.T		=	unpack_D3DCOLOR(I.T);
	I.B		=	unpack_D3DCOLOR(I.B);

	v2p_flat 		O;

	// Transform to world coords
	float3 	pos		= mul		(m_xform, I.P);

	//
	float 	base 	= m_xform._24;			// take base height from matrix
	float 	dp		= calc_cyclic  (wave.w+dot(pos,(float3)wave));
	float 	H 		= pos.y - base;			// height of vertex (scaled, rotated, etc.)
	float 	frac 	= I.tc.z*consts.x;		// fractional (or rigidity)
	float 	inten 	= H * dp;			// intensity
	float2 	result	= calc_xz_wave	(wind.xz*inten, frac);
	
	#ifdef		USE_TREEWAVE
				result	= 0;
	#endif
	
	float4 	f_pos 	= float4(pos.x+result.x, pos.y, pos.z+result.y, 1);

	O.w_pos = f_pos;

	// Final xform(s)
	// Final xform
	float3	Pe		= mul		(m_V,  f_pos				);

	#if USE_MORE_LIGHT_HEMI == 1
		float 	hemi 	= I.Nh.w + saturate(1 - c_bias.w) * 0.15;
	#else
		float 	hemi 	= I.Nh.w * c_scale.w + c_bias.w;
	#endif

//	float 	hemi 	= I.Nh.w;

	O.hpos			= mul		(m_VP, f_pos				); 
	O.N 			= mul		((float3x3)m_xform_v, unpack_bx2(I.Nh.xyz)	);
	O.tcdh 			= float4	((I.tc * consts).xyyy		);
	O.position		= float4	(Pe, hemi					);
		
	update_taa_vertex(O.hpos);

	#if (DX11_STATIC_DEFFERED_RENDERER == 1) && !defined(USE_LM_HEMI)
		float 	suno 	= I.Nh.w/* * c_sun.x*/ + c_sun.y	 * developer_float4.y;
		O.tcdh.w		= hemi;//suno;					// (,,,dir-occlusion)
	#endif

	#ifdef USE_TDETAIL
		O.tcdbump	= O.tcdh*dt_params;					// dt tc
	#endif

	return O;
}