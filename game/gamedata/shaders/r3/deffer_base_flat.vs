#include "common.h"

#if defined(DX11_STATIC_DEFFERED_RENDERER) && !defined(USE_LM_HEMI)
#define	v_in	v_static_color
#else
#define	v_in	v_static
#endif

v2p_flat main ( v_in I )
{
	float4 w_pos = I.P;
	w_pos.xyz = mul(m_W, w_pos);
	w_pos.w = 1;

	I.Nh			= unpack_D3DCOLOR(I.Nh);
	I.T				= unpack_D3DCOLOR(I.T);
	I.B				= unpack_D3DCOLOR(I.B);

	v2p_flat 		O;
	
	// Eye-space pos/normal
	float4	Pp 	= mul( m_VP, w_pos );
	O.hpos 		= Pp;
	O.N 		= mul( (float3x3)m_WV, unpack_bx2(I.Nh.xyz) );
	float3	Pe	= mul( m_V, w_pos );

	float2	tc 	= unpack_tc_base( I.tc, I.T.w, I.B.w);	// copy tc
	O.tcdh		= float4( tc.xyyy );
	O.position	= float4( Pe, I.Nh.w );
	
	update_taa_vertex(O.hpos);

#if defined(DX11_STATIC_DEFFERED_RENDERER) && !defined(USE_LM_HEMI)
	float 	s	= I.color.w	;							// (r,g,b,dir-occlusion)
	O.tcdh.w	= s;
#endif

#ifdef	USE_TDETAIL
	O.tcdbump	= O.tcdh * dt_params.xy;					// dt tc
#endif

#ifdef	USE_LM_HEMI
	O.lmh 		= unpack_tc_lmap( I.lmh );
#endif

	return	O;
}
