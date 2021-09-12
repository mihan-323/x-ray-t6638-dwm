#ifndef RSM_H
#define RSM_H
	#include "common.h"
	#include "lmodel.h"
	#include "shadow.h"

	/*
		Reflective Shadow Maps
	*/

	/*
		Accumulate RSM for current light
	*/

	uniform Texture2D<float3> s_positionil;
	uniform Texture2D<float3> s_normalil;
	uniform Texture2D<float3> s_coloril;
	// uniform Texture2D<float3> s_accumulatoril;

	// r__sun_il_params_0 = { 5, 3, 50, 1 };
	// r__sun_il_params_1 = { 3.25, 0.02, 50, 0.015 };
	// r__sun_il_params_2 = { 200, 0.01, 0.5, 20 };

	uniform float4 c_rsm_generate_params_0; // r__sun_il_params_0 = (r__sun_gi_samples, 	r__sun_gi_saturation, 	r__sun_gi_brightness, 	r__sun_gi_intensity)
	uniform float4 c_rsm_generate_params_1; // r__sun_il_params_1 = (r__sun_gi_fade_power, 	r__sun_gi_fade_min, 	r__sun_gi_fade_max, 	r__sun_gi_size)
	uniform float4 c_rsm_generate_params_2; // r__sun_il_params_2 = (r__sun_gi_far_plane, 	r__sun_gi_near_plane, 	r__sun_gi_normal_bias, 	r__sun_gi_jitter_size)

	uniform int dwframe; // current frame id

	static const float golden_angle = 2.4;
	static const float hash_angle = 6.1258;

	// make offset for the 2d tex coordinates
	// by the vogel disk with randomized radius and angle using 3d noise
	// radius noise makes offset in the radius of one sample
	float2 rsm_make_offset(int points_num, int point_id, float3 noise_3d)
	{
		float r = sqrt((float)point_id + 0.5) / sqrt((float)points_num);
		float theta = (float)point_id * golden_angle;
		float2 sincon_new;
		sincos(theta + noise_3d.z * hash_angle, sincon_new.x, sincon_new.y);
		float2 offset = r * sincon_new * (1 - noise_3d.xy);
		return offset;
	}

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

	// accumulate the reflective shadow map for current dynamic light
	float3 rsm_accum_hashed(float2 tc, float2 pos2d)
	{
		G_BUFFER::GBD gbd = G_BUFFER::load_P_N_hemi_mtl_mask(tc, pos2d);

		float dist = length(gbd.P);

		if(dist <= rsm_near_plane || dist > rsm_far_plane || gbd.mask) 
			return 0;

		float3 normalw = G_BUFFER::vs_ws(gbd.N);

		#ifdef ACCUM_DIRECT
			float3 dirw = L_sun_dir_w;
		#else
			float3 dirw = G_BUFFER::vs_ws(Ldynamic_dir.xyz);
		#endif

		float cant_light = saturate(dot(dirw, normalw));
		gbd.P += gbd.N * cant_light * rsm_normal_bias;

		float3 positionw = G_BUFFER::vs_ws(gbd.P, 1);

		float4 PS = mul(m_shadow, float4(gbd.P, 1));
		float3 PSproj = PS.xyz / PS.w;

		float3 hash = noise::hash33(float3(pos2d, dwframe));

		float3 accum = 0;

		for (int i = 0; i < (int)rsm_samples; i++)
		{
			float2 PSproj_current = PSproj.xy + rsm_make_offset(rsm_samples, i, hash) * rsm_size;

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

	/*
		Spatial filter
	*/

	uniform Texture2D s_rsm;

	// r__sun_il_params_3 = { 0.05, 0.075, 0.1, 0.93 };

	// r__sun_il_params_3 = (r__sun_gi_spatial_filter_depth_threshold, 
	//						 r__sun_gi_spatial_filter_normal_threshold, 
	//						 r__sun_gi_temporal_filter_depth_threshold, 	
	//						 r__sun_gi_temporal_filter_expectation)
	uniform float4 c_rsm_generate_params_3; 

	// static const float edge_dist = 0.05;
	// static const float normal_dist = 0.075;

	// 1 - halton sequence (2, 3) discretized to 7x7 region
	// 2 - sample region 3x3
	// 3 - sample region 2x2
	#define SPATIAL_FILTER_TYPE 2

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
		return s_rsm.SampleLevel(smp_rtlinear, tc, 0);
	}

	// spatial filter simulates fill in lost info, does it by bilinear filtering with current offsets
	float4 rsm_spatial_filter(float2 tc, float2 pos2d)
	{
		// return rsm_sample_bi_0(tc);

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

	uniform Texture2D s_rsm_prev;

	// r__sun_il_params_3 = { 0.05, 0.075, 0.1, 0.93 };

	// r__sun_il_params_3 = (r__sun_gi_spatial_filter_depth_threshold, 
	//						 r__sun_gi_spatial_filter_normal_threshold, 
	//						 r__sun_gi_temporal_filter_depth_threshold, 	
	//						 r__sun_gi_temporal_filter_expectation)
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

	// temporal reprojected recursive filter, remove spatial filter flickering and fill other lost info
	float4 rsm_temporal_filter(float2 tc, float2 pos2d)
	{
		float4 rsm = s_rsm.SampleLevel(smp_rtlinear, tc, 0);

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
