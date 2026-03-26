#include "common.h"
#include "skin.h"
#include "planar_ligth.h"

uniform float4 planar_env;
uniform float4 planar_amb;
uniform float4 planar_bias;
// uniform float4x4 planar_vp_camera;

v2p_planar _main(v_model I)
{
	float3 w_pos = mul(m_W, I.P);

	float3 Nw = mul((float3x3)m_W, (float3)I.N);

	float4 Pp = mul(m_VP, float4(w_pos, 1));
	update_taa_vertex(Pp);

	float3 Ne = mul(m_V, Nw);

	float3 Pe_b = mul(m_V, float4(w_pos, 1)) + Ne * planar_bias.x;

	float2 base_tc = I.tc.xy;

	// float2 screen_tc;
	// planar_vertex_proj_camera_space(w_pos, planar_vp_camera, screen_tc);

	// Hemi cube lighting
	float3 hc_pos = (float3)hemi_cube_pos_faces;
	float3 hc_neg = (float3)hemi_cube_neg_faces;
	float3 hc_mixed = (Nw < 0) ? hc_neg : hc_pos;
	float hemi_val = dot(hc_mixed, abs(Nw));
	hemi_val = saturate(hemi_val);

	float4 planar_pos = float4(Pe_b, w_pos.y);

	float3 light_sun, light_spot, diffuse;
	planar_vertex_ligth(Nw, hemi_val, planar_env, planar_amb, light_sun, light_spot, diffuse);

	v2p_planar O = {base_tc, planar_pos, light_sun, light_spot, diffuse, Pp};

	return O;
}

/////////////////////////////////////////////////////////////////////////
#ifdef 	SKIN_NONE
v2p_planar	main(v_model v) 		{ return _main(v); 		}
#endif

#ifdef 	SKIN_0
v2p_planar	main(v_model_skinned_0 v) 	{ return _main(skinning_0(v)); }
#endif

#ifdef	SKIN_1
v2p_planar	main(v_model_skinned_1 v) 	{ return _main(skinning_1(v)); }
#endif

#ifdef	SKIN_2
v2p_planar	main(v_model_skinned_2 v) 	{ return _main(skinning_2(v)); }
#endif

#ifdef	SKIN_3
v2p_planar	main(v_model_skinned_3 v) 	{ return _main(skinning_3(v)); }
#endif

#ifdef	SKIN_4
v2p_planar	main(v_model_skinned_4 v) 	{ return _main(skinning_4(v)); }
#endif
