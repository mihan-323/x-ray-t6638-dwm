#include "common.h"
#include "planar_ligth.h"

uniform float4 planar_env;
uniform float4 planar_amb;
uniform float4 planar_bias;
// uniform float4x4 planar_vp_camera;

v2p_planar main(v_static I)
{
	float3 w_pos = mul(m_W, I.P);

	I.Nh = unpack_D3DCOLOR(I.Nh);
	I.T = unpack_D3DCOLOR(I.T);
	I.B = unpack_D3DCOLOR(I.B);

	float4 Pp = mul(m_VP, float4(w_pos, 1));
	update_taa_vertex(Pp);

	float3 Nw = mul((float3x3)m_W, unpack_bx2(I.Nh.xyz));

	float3 Ne = mul(m_V, Nw);

	float3 Pe_b = mul(m_V, float4(w_pos, 1)) + Ne * planar_bias.x;

	float2 base_tc = unpack_tc_base(I.tc, I.T.w, I.B.w);

	// float2 screen_tc;
	// planar_vertex_proj_camera_space(w_pos, planar_vp_camera, screen_tc);

	float4 planar_pos = float4(Pe_b, w_pos.y);

	float3 light_sun, light_spot, diffuse;
	planar_vertex_ligth(Nw, I.Nh.w, planar_env, planar_amb, light_sun, light_spot, diffuse);

	v2p_planar O = {base_tc, planar_pos, light_sun, light_spot, diffuse, Pp};

	return O;
}
