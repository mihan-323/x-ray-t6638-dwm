#ifndef RSM_H
#define RSM_H
	#include "common.h"
	#include "lmodel.h"
	#include "shadow.h"

	/*
		Reflective Shadow Maps
	*/

	// 0 - disabled
	// 1 - halton sequence (2, 3) discretized to 7x7 region
	// 2 - sample region 3x3
	// 3 - sample region 2x2
	#define SPATIAL_FILTER_TYPE 2 // 2

	// #define RSM_HALFRES
	// #define RSM_DISABLE_SPATIAL
	// #define RSM_DISABLE_TEMPORAL

	/*
		Accumulate RSM for current light
	*/

	uniform Texture2D<float3> s_positionil;
	uniform Texture2D<float3> s_normalil;
	uniform Texture2D<float3> s_coloril;

	uniform Texture2D s_rsm_prev;
	uniform Texture2D s_rsm;

	// uniform Texture2D<float3> s_accumulatoril;

	uniform float4 c_rsm_generate_params_0; // r__gi_samples, 		r__gi_saturation, 		r__gi_brightness, 		r__gi_intensity
	uniform float4 c_rsm_generate_params_1; // r__gi_fade_power, 	r__gi_fade_min, 		r__gi_fade_max, 		r__gi_size
	uniform float4 c_rsm_generate_params_2; // r__gi_far_plane, 	r__gi_near_plane, 		r__gi_normal_bias, 		r__gi_jitter_size
	uniform float4 c_rsm_generate_params_3; // r__gi_spatial_depth,	r__gi_spatial_normal,	r__gi_temporal_depth,	r__gi_temporal_expectation

	uniform float4x4 m_tVP;

	#ifndef dwframe_used
	#define dwframe_used
		uniform int dwframe; // current frame id
	#endif

	#define rsm_samples	  	c_rsm_generate_params_0.x
	#define rsm_saturation  c_rsm_generate_params_0.y
	#define rsm_brightness  c_rsm_generate_params_0.z
	#define rsm_intensity   c_rsm_generate_params_0.w
	#define rsm_fade_power  c_rsm_generate_params_1.x
	#define rsm_fade_min 	c_rsm_generate_params_1.y
	#define rsm_fade_max 	c_rsm_generate_params_1.z
	#define rsm_size 		c_rsm_generate_params_1.w
	#define rsm_far_plane   c_rsm_generate_params_2.x
	#define rsm_near_plane  c_rsm_generate_params_2.y
	#define rsm_normal_bias c_rsm_generate_params_2.z
	#define rsm_jitter_size c_rsm_generate_params_2.w

	// accumulate the reflective shadow map for a dynamic light
	float3 rsm_accum_hashed(float2 tc, float2 pos2d)
	{
		#ifdef RSM_HALFRES
			tc *= 2;
			pos2d *= 2;
		#endif
		
		G_BUFFER::GBD gbd = G_BUFFER::load_P_N_hemi_mtl_mask(tc, pos2d);

		float dist = length(gbd.P);

		if(dist <= rsm_near_plane || dist > rsm_far_plane || 
		   gbd.mask || !is_in_quad(tc)) 
			return 0;

		float3 normalw = G_BUFFER::vs_ws(gbd.N);

		#ifdef ACCUM_DIRECT
			float3 dirw = L_sun_dir_w;
		#else
			float3 dirw = G_BUFFER::vs_ws(Ldynamic_dir.xyz);
		#endif

		float cant_light = saturate(dot(dirw, normalw));
		gbd.P += gbd.N * cant_light * rsm_normal_bias;
		// gbd.P += gbd.N * 0.025;

		float3 positionw = G_BUFFER::vs_ws(gbd.P, 1);

		float4 PS = mul(m_shadow, float4(gbd.P, 1));
		float3 PSproj = PS.xyz / PS.w;

		float3 hash_tc;
		hash_tc.xy = pos2d;
		// hash_tc.z = dwframe % (int)(1.0 / (1 - c_rsm_generate_params_3.w) + 0.5);
		hash_tc.z = dwframe % 16;

		float3 hash = noise::hash33(hash_tc);

		float3 accum = 0;

		// float3 normalil = s_normalil.SampleLevel(smp_rtlinear, PSproj, 0);
		// float3 positionil = s_positionil.SampleLevel(smp_rtlinear, PSproj, 0);
		// float fresnelil = abs(dot(-normalil, normalize(positionil)));

		float sector_full = 6.2831853*8;
		float sector_tap = sector_full / rsm_samples;
		float sector_start = sector_full * hash.x;

		float2 direction;
		sincos(sector_start, direction.y, direction.x);
		direction *= rsm_size / rsm_samples;

		// float falloff_factor = -1.0 / (rsm_size * rsm_size);

		float2 rotation;
		sincos(sector_tap, rotation.y, rotation.x);

		float2x2 rot = float2x2(rotation.x, -rotation.y, rotation.y, rotation.x);

		for (int i = 0; i < (int)rsm_samples; i++)
		{
			float2 PSproj_current = PSproj.xy + direction * (i + hash.y);
			direction = mul(direction, rot);

			if(!is_in_quad(PSproj_current))
				continue;

			float3 positionil = s_positionil.SampleLevel(smp_rtlinear, PSproj_current, 0);
			float fade = pow(length(positionw - positionil), rsm_fade_power);

			if(!is_in_range(fade, rsm_fade_min, rsm_fade_max))
				continue;

			float m1 = dot(normalw, positionil - positionw);

			if(m1 <= 0)
				continue;

			float3 normalil = s_normalil.SampleLevel(smp_rtlinear, PSproj_current, 0);
			float m2 = dot(normalil, positionw - positionil);

			if(m2 <= 0)
				continue;

			float3 coloril = s_coloril.SampleLevel(smp_rtlinear, PSproj_current, 0);
			accum += coloril * saturate(m1 * m2 / fade);
		}

		accum = accum * rsm_brightness / rsm_samples;
		accum = accum / (1 + accum);

		float gray = dot(accum, LUMINANCE_VECTOR);
		accum = lerp(gray, accum, rsm_saturation);

		accum = accum * rsm_intensity * Ldynamic_color.xyz;

		return accum;
	}

	// accumulate the reflective shadow map for a dynamic light
	float3 rsm_accum_hashed_new(float2 tc, float2 pos2d)
	{
		G_BUFFER::GBD gbd = G_BUFFER::load_P_N_hemi_mtl_mask(tc, pos2d);

		float3 pos = gbd.P;
		float3 wpos = mul(m_v2w, float4(pos, 1));

		float3 norm = gbd.N;
		float3 wnorm = mul(m_v2w, float4(norm, 0));

		float3 positionw = G_BUFFER::vs_ws(gbd.P, 1);

		float4 PS = mul(m_shadow, float4(gbd.P, 1));
		float3 PSproj = PS.xyz / PS.w;
		float2 PSpix = ddx(PSproj.xy) + ddy(PSproj.xy);
		float2 PSres = 1.0 / PSpix;

		float3 lwpos_gen = s_positionil.SampleLevel(smp_rtlinear, PSproj.xy, 0);
		float3 lpos_gen = mul(m_V, float4(lwpos_gen, 1));
		
		float4 lPS = mul(m_shadow, float4(lpos_gen, 1));
		float3 lPSproj = lPS.xyz / lPS.w;

		float2 ltc = lPSproj.xy;

		// if(lPSproj.z + 0.00017 < PSproj.z)
			// return 0;

		float3 lwpos = s_positionil.SampleLevel(smp_rtlinear, ltc, 0);
		float3 lwnorm = s_normalil.SampleLevel(smp_rtlinear, ltc, 0);

		lwpos += lwnorm * 0.18;

		float3 lpos = mul(m_V, float4(lwpos, 1));
		float3 lnorm = mul(m_V, float4(lwnorm, 0));

		float3 hash_tc;
		hash_tc.xy = pos2d % 4;
		hash_tc.z = 0;

		float2 hash = noise::hash23(hash_tc);

		int rays = 25;

		float circle = 6.2831853*8;
		float sector = circle / rays;
		float angle = circle * hash.x;

		float2 direction;
		sincos(angle, direction.y, direction.x);

		float2 rotation;
		sincos(sector, rotation.y, rotation.x);

		float2x2 rot = float2x2(rotation.x, -rotation.y, rotation.y, rotation.x);

		float3 up 		= abs(lwnorm.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
		float3 tangent 	= normalize(cross(up, lwnorm));
		float3 binormal = cross(lwnorm, tangent);

		float3x3 tbn = float3x3(tangent, binormal, lwnorm);

		float gi = 0;

		for (int i = 0; i < rays; i++)
		{
			float st = (i + hash.y) / rays;
			float ct = sqrt(1 - st*st);

			float2 uv_dir = direction * ct;
			direction = mul(direction, rot);

			float3 sph = float3(uv_dir, st);
			sph = mul(sph, tbn);

			float3 lwpos_tap = lwpos + sph;
			float3 lpos_tap = mul(m_V, float4(lwpos_tap, 1));

			float4 lPS_tap = mul(m_shadow, float4(lpos_tap, 1));
			float3 lPSproj_tap = lPS_tap.xyz / lPS_tap.w;

			float2 ltc_tap = lPSproj_tap.xy;

			float3 lwpos_hit = s_positionil.SampleLevel(smp_rtlinear, ltc_tap, 0);
			float3 lpos_hit = mul(m_V, float4(lwpos_hit, 1));

			float dist = lpos_tap.z - lpos_hit.z;
			float distinvsqr = 1 - dot(dist, dist);

			int exist = is_in_range(dist, 0, 1);
			gi += distinvsqr * exist;
		}

		gi = 1 - gi / rays;

		// accum = accum * rsm_brightness / rsm_samples;
		// accum = accum / (1 + accum);

		// float gray = dot(accum, LUMINANCE_VECTOR);
		// accum = lerp(gray, accum, rsm_saturation);

		// accum = accum * rsm_intensity * Ldynamic_color.xyz;

		return gi;
	}

	/*
		Spatial filter
	*/

	// uniform Texture2D s_rsm;

	// r__gi_spatial_filter_depth_threshold,  r__gi_spatial_filter_normal_threshold
	// r__gi_temporal_filter_depth_threshold, r__gi_temporal_filter_expectation
	// uniform float4 c_rsm_generate_params_3; 

	// static const float edge_dist = 0.05;
	// static const float normal_dist = 0.075;

	#if SPATIAL_FILTER_TYPE == 1
		#define filter_size 15
		#define get_offset(i) offsets[i]
		// halton sequence (2, 3) discretized to 7x7 region
		static const int2 offsets[filter_size] =
		{
			-3,  0,	-3,  3,	-2, -2,
			-2,  1,	-1, -2,	-1, -1,
			 0, -3,	 0,  1,	 0,  2,
			 1,  0,	 1,  2,	 2, -3,
			 2,  1,	 3,  0,	 3, -1,
		};
	#elif SPATIAL_FILTER_TYPE == 2
		#define filter_size 9
		#define get_offset(i) offsets[i]
		// sample region 3x3
		static const int2 offsets[filter_size] =
		{
			-1, -1,	-1,  0,	-1,  1,
			 0, -1,	 0,  0,	 0,  1,
			 1, -1,	 1,  0,	 1,  1,
		};
	#elif SPATIAL_FILTER_TYPE == 3
		#define filter_size 4
		#define get_offset(i) offsets[i]
		// sample region 2x2
		static const int2 offsets[filter_size] =
		{
			-1, -1,	-1,  0,
			 0, -1,	 0,  0,
		};
	#else
		#define filter_size 0
		#define get_offset(i) 0
	#endif

	// scene info for spatial filter
	struct RSM_SCENE
	{
		float3 Nw;
		float depth;
	};

	RSM_SCENE rsm_sample_scene(float2 tc)
	{
		G_BUFFER::set_bilinear_f(true);
		G_BUFFER::GBD gbd = G_BUFFER::load_P_N(tc);
		float3 Nw = G_BUFFER::vs_ws(gbd.N);
		float depth = length(gbd.P);
		RSM_SCENE scene = {Nw, depth};
		return scene;
	}

	int rsm_detect_plane(float2 tc, RSM_SCENE scene, float depthsqr)
	{
		if(!is_in_quad(tc)) return 0;
		RSM_SCENE scene_tap = rsm_sample_scene(tc);
		int plane = abs(scene.depth - scene_tap.depth) < c_rsm_generate_params_3.x * depthsqr;
		plane &= abs(scene.Nw - scene_tap.Nw) < c_rsm_generate_params_3.yyy;
		return plane;
	}

	float4 rsm_sample_bi_0(float2 tc)
	{
		#ifdef RSM_HALFRES
			tc *= 0.5;
		#endif
		return s_rsm.SampleLevel(smp_rtlinear, tc, 0);
	}

	// spatial filter simulates fill in lost info, does it by bilinear filtering with current offsets
	float4 rsm_spatial_filter(float2 tc, float2 pos2d)
	{
		#ifdef RSM_DISABLE_SPATIAL
			return rsm_sample_bi_0(tc);
		#endif
		
		float2 tc_b = tc + float2(0.5, 0.5) * screen_res.zw;
		float2 pixel_b = screen_res.zw * 2;

		RSM_SCENE scene = rsm_sample_scene(tc_b);

		// float depthsqr = sqrt(scene.depth);
		float depthsqr = scene.depth;

		float4 accum = rsm_sample_bi_0(tc);

		for(int i = 0; i < filter_size; i++)
		{
			float2 bias = pixel_b * get_offset(i);
			int plane_curr = rsm_detect_plane(tc_b + bias, scene, depthsqr);
			float4 curr = rsm_sample_bi_0(tc_b + bias * plane_curr);
			accum = max(accum, curr);
		}

		accum.w = dot(accum.xyz, LUMINANCE_VECTOR);
		accum.w = sqrt(accum.w);

		return accum;
	}

	/*
		Temporal filter
	*/

	// uniform Texture2D s_rsm;

	// uniform Texture2D s_rsm_prev;

	// r__gi_spatial_filter_depth_threshold,  r__gi_spatial_filter_normal_threshold
	// r__gi_temporal_filter_depth_threshold, r__gi_temporal_filter_expectation
	// uniform float4 c_rsm_generate_params_3; 

	// static const float edge_dist = 0.1;
	// static const float expectation = 0.93;

	uint rsm_need_reprojection(float4 rsm, float2 tc, float2 tc_next, float2 pos2d, float depth)
	{
		if(!is_in_quad(tc_next))
			return 0;

		float Pz_prev_reprojected = G_BUFFER::load_history_packed(tc_next).z;

		float depth_threshold = c_rsm_generate_params_3.z * depth;

		return abs(Pz_prev_reprojected - depth) < depth_threshold;
	}

	// uniform float4x4 m_tVP;

	// temporal reprojected recursive filter, remove spatial filter flickering and fill other lost info
	float4 rsm_temporal_filter(float2 tc, float2 pos2d)
	{
		float4 rsm = s_rsm.SampleLevel(smp_rtlinear, tc, 0);

		#ifdef RSM_DISABLE_TEMPORAL
			return rsm;
		#endif

		G_BUFFER::GBD gbd = G_BUFFER::load_P_mask(tc, pos2d);

		float3 Pworld = G_BUFFER::vs_ws(gbd.P, 1);

		float4 proj_next = mul(m_tVP, float4(Pworld, 1));

		float2 tc_next = proj_next.xy / proj_next.ww * float2(0.5, -0.5) + 0.5;

		uint test = rsm_need_reprojection(rsm, tc, tc_next, pos2d, gbd.P.z);

		if(test && !gbd.mask)
		{
			float4 rsm_prev = s_rsm_prev.SampleLevel(smp_rtlinear, tc_next, 0);

			rsm = lerp(rsm, rsm_prev, c_rsm_generate_params_3.w);
		}

		return rsm;
	}
#endif
