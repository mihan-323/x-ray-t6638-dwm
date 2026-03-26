#include "common.h"
#include "planar_ligth.h"

uniform float3x4 m_xform;
uniform float3x4 m_xform_v;
uniform float4 consts; // {1/quant,1/quant,???,???}
uniform float4 c_scale, c_bias, wind, wave;
uniform float2 c_sun; // x=*, y=+
uniform float4 planar_env;
uniform float4 planar_amb;
uniform float4 planar_bias;
// uniform float4x4 planar_vp_camera;

v2p_planar main(v_tree I)
{
	I.Nh = unpack_D3DCOLOR(I.Nh);
	I.T = unpack_D3DCOLOR(I.T);
	I.B = unpack_D3DCOLOR(I.B);

	// Transform to world coords
	float3 w_pos0 = mul(m_xform, I.P);

	#ifdef USE_TREEWAVE
		float4 w_pos = float4(w_pos0, 1);
	#else
		float 	base 	= m_xform._24; 									// take base height from matrix
		float 	dp		= calc_cyclic(wave.w + dot(w_pos0, (float3)wave));
		float 	H 		= w_pos0.y - base; 								// height of vertex (scaled, rotated, etc.)
		float 	frac 	= I.tc.z * consts.x; 							// fractional (or rigidity)
		float 	inten 	= H * dp; 										// intensity
		float2 	result	= calc_xz_wave(wind.xz * inten, frac);
		float4 w_pos = float4(w_pos0.x + result.x, w_pos0.y, w_pos0.z + result.y, 1);
	#endif

	float3 Nw = mul((float3x3)m_xform, unpack_bx2(I.Nh.xyz));

	float3 Ne = mul(m_V, Nw);

	float3 Pe_b = mul(m_V, w_pos) + Ne * planar_bias.x;

	float hemi = I.Nh.w * c_scale.w + c_bias.w;

	float4 Pp = mul(m_VP, w_pos); 
	update_taa_vertex(Pp);

	float2 base_tc = (I.tc * consts).xy;

	// float2 screen_tc;
	// planar_vertex_proj_camera_space(w_pos, planar_vp_camera, screen_tc);

	float4 planar_pos = float4(Pe_b, w_pos.y);

	float3 light_sun, light_spot, diffuse;
	planar_vertex_ligth(Nw, hemi, planar_env, planar_amb, light_sun, light_spot, diffuse);

	v2p_planar O = {base_tc, planar_pos, light_sun, light_spot, diffuse, Pp};

	return O;
}