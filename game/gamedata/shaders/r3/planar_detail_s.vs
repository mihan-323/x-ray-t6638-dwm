#include "common.h"
#include "planar_ligth.h"

uniform float4 consts; // {1/quant,1/quant,diffusescale,ambient}
uniform float4 array[61 * 4];
uniform float4 planar_env;
uniform float4 planar_amb;
uniform float4 planar_bias;
// uniform float4x4 planar_vp_camera;

#ifdef USE_TREEWAVE
	uniform float4 wave; // cx,cy,cz,tm
	uniform float4 dir2D;
#endif

v2p_planar main(v_detail v)
{
	// index
	int 	i 	= v.misc.w;
	float4  m0 	= array[i+0];
	float4  m1 	= array[i+1];
	float4  m2 	= array[i+2];
	float4  c0 	= array[i+3];

	// Transform pos to world coords
	float4 pos;
 	pos.x = dot(m0, v.pos);
 	pos.y = dot(m1, v.pos);
 	pos.z = dot(m2, v.pos);
	pos.w = 1;

	#ifdef USE_TREEWAVE
		float base = m1.w;
		float dp = calc_cyclic(dot(pos,wave));
		float H = pos.y - base; // height of vertex (scaled)
		float frac = v.misc.z * consts.x; // fractional
		float inten = H * dp;
		float2 result = calc_xz_wave(dir2D.xz * inten,frac);
		pos = float4(pos.x + result.x, pos.y, pos.z + result.y, 1);
	#endif

	// Normal in world coords
	float3 norm;	
	norm.x = pos.x - m0.w;
	norm.y = pos.y - m1.w + 0.75f; // avoid zero
	norm.z = pos.z - m2.w;

	float3 w_pos = mul(m_W,	pos);

	float3 Nw = mul(m_W, normalize(norm));
	
	float4 Pp = mul(m_VP, float4(w_pos, 1));
	update_taa_vertex(Pp);

	float3 Ne = mul(m_WV, normalize(norm));

	float3 Pe_b = mul(m_V, float4(w_pos, 1)) + Ne * planar_bias.x;

	float2 base_tc = (v.misc * consts).xy;

	// float2 screen_tc;
	// planar_vertex_proj_camera_space(w_pos, planar_vp_camera, screen_tc);

	float4 planar_pos = float4(Pe_b, w_pos.y);

	float3 light_sun, light_spot, diffuse;
	planar_vertex_ligth(Nw, c0.w, planar_env, planar_amb, light_sun, light_spot, diffuse);

	v2p_planar O = {base_tc, planar_pos, light_sun, light_spot, diffuse, Pp};

	return O;
}