#include "common.h"

#if (DX11_STATIC_DEFFERED_RENDERER == 1) && !defined(USE_LM_HEMI)
	#define	v_in v_static_color	
#else
	#define	v_in v_static
#endif

void main(in v_in I, out v2p_bumped O)
{
	float4 w_pos = I.P;
	w_pos.xyz = mul(m_W, w_pos);
	w_pos.w = 1;

	// w_pos.y = -(w_pos.y - 2 * -7.2);

	float2 tc = unpack_tc_base(I.tc, I.T.w, I.B.w); // copy tc

	float hemi = I.Nh.w;

	float3 Pe = mul(m_V, w_pos);

	// Eye-space pos/normal
	O.hpos 		= mul(m_VP, w_pos);
	O.position	= float4(Pe, hemi);
	
	update_taa_vertex(O.hpos);

	O.tcdh 		= float4(tc.xyyy);

	#if (DX11_STATIC_DEFFERED_RENDERER == 1) && !defined(USE_LM_HEMI)
		O.tcdh.w = I.color.w; // (r,g,b,dir-occlusion)
	#endif

	// Calculate the 3x3 transform from tangent space to eye-space
	// TangentToEyeSpace = object2eye * tangent2object
	//		     = object2eye * transpose(object2tangent) (since the inverse of a rotation is its transpose)
	I.Nh			= unpack_D3DCOLOR(I.Nh);
	I.T				= unpack_D3DCOLOR(I.T);
	I.B				= unpack_D3DCOLOR(I.B);
	float3 	N 	= unpack_bx4(I.Nh.xyz);	// just scale (assume normal in the -.5f, .5f)
	float3 	T 	= unpack_bx4(I.T.xyz);	// 
	float3 	B 	= unpack_bx4(I.B.xyz);	// 
	float3x3 xform	= mul	((float3x3)m_WV, float3x3(
						T.x,B.x,N.x,
						T.y,B.y,N.y,
						T.z,B.z,N.z
				));
	// The pixel shader operates on the bump-map in [0..1] range
	// Remap this range in the matrix, anyway we are pixel-shader limited :)
	// ...... [ 2  0  0  0]
	// ...... [ 0  2  0  0]
	// ...... [ 0  0  2  0]
	// ...... [-1 -1 -1  1]
	// issue: strange, but it's slower :(
	// issue: interpolators? dp4? VS limited? black magic? 

	// Feed this transform to pixel shader
	O.M1 			= xform[0]; 
	O.M2 			= xform[1]; 
	O.M3 			= xform[2]; 

	// O.M1 = -O.M1;
    // O.M2 = -O.M2;
    // O.M3 = -O.M3;

#ifdef 	USE_TDETAIL
	O.tcdbump		= O.tcdh.xy * dt_params.xy;		// dt tc
#endif

#ifdef	USE_LM_HEMI
	O.lmh 			= unpack_tc_lmap	(I.lmh);
#endif
}
