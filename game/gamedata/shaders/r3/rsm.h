#ifndef RSM_H
#define RSM_H
	#include "common.h"
	#include "lmodel.h"
	#include "shadow.h"

	/*
		Reflective Shadow Maps
	*/

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

	uniform float4x4 m_tVP;

	#ifndef dwframe_used
	#define dwframe_used
		uniform int dwframe; // current frame id
	#endif

	#ifdef ACCUM_DIRECT
		#define rsm_mip_level	0
		#define rsm_samples	  	5
		#define rsm_size 		0.015f
		#define rsm_fade_power  3.25f
		#define rsm_brightness  5.0f
		#define rsm_saturation  2.0f
		#define rsm_fade_min 	0.02f
		#define rsm_fade_max 	50.0f
		#define rsm_far_plane   200.0f
		#define rsm_near_plane  0.01f
		#define rsm_normal_bias 0.5f
	#else
		#define rsm_mip_level	0
		#define rsm_samples	  	1
		#define rsm_size 		0.15f
		#define rsm_fade_power  3.25f
		#define rsm_brightness  1.5f
		#define rsm_saturation  0.5f
		#define rsm_fade_min 	0.02f
		#define rsm_fade_max 	15.0f
		#define rsm_far_plane   15.0f
		#define rsm_near_plane  0.01f
		#define rsm_normal_bias 2.0f
	#endif
	
	#define rsm_sf_type 	2
	#define rsm_sf_depth	0.05f
	#define rsm_sf_normal	0.075f
	#define rsm_tf_depth	0.1f
	#define rsm_tf_exp		0.9f
		
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

		float3 positionw = G_BUFFER::vs_ws(gbd.P, 1);

		float4 PS = mul(m_shadow, float4(gbd.P, 1));
		float3 PSproj = PS.xyz / PS.w;

		float3 hash_tc;
		hash_tc.xy = pos2d;
		hash_tc.z = dwframe % 16;

		float3 hash = noise::hash33(hash_tc);

		float3 accum = 0;

		float sector_full = 6.2831853*8;
		float sector_tap = sector_full / rsm_samples;
		float sector_start = sector_full * hash.x;

		float2 direction;
		sincos(sector_start, direction.y, direction.x);
		direction *= rsm_size / rsm_samples;

		float2 rotation;
		sincos(sector_tap, rotation.y, rotation.x);

		float2x2 rot = float2x2(rotation.x, -rotation.y, rotation.y, rotation.x);

		for (int i = 0; i < (int)rsm_samples; i++)
		{
			float2 PSproj_current = PSproj.xy + direction * (i + hash.y);
			direction = mul(direction, rot);

			if(!is_in_quad(PSproj_current))
				continue;

			float3 positionil = s_positionil.SampleLevel(smp_rtlinear, PSproj_current, rsm_mip_level);
			float fade = pow(length(positionw - positionil), rsm_fade_power);
			
			if(!is_in_range(fade, rsm_fade_min, rsm_fade_max))
				continue;

			float m1 = dot(normalw, positionil - positionw);

			if(m1 <= 0)
				continue;

			float3 normalil = s_normalil.SampleLevel(smp_rtlinear, PSproj_current, rsm_mip_level);
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

		accum = accum * Ldynamic_color.xyz;

		return accum;
	}

	// accumulate the reflective shadow map for a dynamic light
	float3 rsm_accum_hashed_advanced(float2 tc, float2 pos2d)
	{
		if(DEVX)return 0;
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

		float3 positionw = G_BUFFER::vs_ws(gbd.P, 1);

		float4 PS = mul(m_shadow, float4(gbd.P, 1));
		float3 PSproj = PS.xyz / PS.w;

		float3 hash_tc;
		hash_tc.xy = pos2d;
		hash_tc.z = dwframe % 16;

		float3 hash = noise::hash33(hash_tc);

		float3 accum = 0;

		float sector_full = 6.2831853*8;
		float sector_tap = sector_full / rsm_samples;
		float sector_start = sector_full * hash.x;

		float2 direction;
		sincos(sector_start, direction.y, direction.x);
		direction *= rsm_size / rsm_samples;

		float2 rotation;
		sincos(sector_tap, rotation.y, rotation.x);

		float2x2 rot = float2x2(rotation.x, -rotation.y, rotation.y, rotation.x);

		float small_max_dist = 1.5f;
		float big_max_dist = 4.0f;
		
		float specular_gloss = 16;
		float specular_power = 0.1;
			
		bool use_specular = 1;
		bool use_small = 1;
			
		bool use_diffuse = 1;
			
		float3 view_dirw = normalize(eye_position - positionw);
				
		for (int i = 0; i < (int)rsm_samples; i++)
		{
			float2 PSproj_current = PSproj.xy + direction * (i + hash.y);
			direction = mul(direction, rot);

			if(!is_in_quad(PSproj_current))
				continue;

			float depth_light = s_smap.SampleLevel(smp_nofilter, PSproj_current, 0);
			if(PSproj.z > depth_light)
				continue;
			
			float3 positionil = s_positionil.SampleLevel(smp_rtlinear, PSproj_current, rsm_mip_level);
			
			float3 w_light = positionil - positionw; // fragment to shadow map
			float3 light_dir_sample_w = normalize(w_light);
			float m1 = dot(normalw, w_light);

			if(m1 <= 0)
				continue;

			float3 normalil = s_normalil.SampleLevel(smp_rtlinear, PSproj_current, rsm_mip_level);
			float m2 = dot(normalil, positionw - positionil);

			if(m2 <= 0)
				continue;

			float3 coloril = s_coloril.SampleLevel(smp_rtlinear, PSproj_current, 0);
			
			float weight = 0.0f;
			
			float dist = length(positionw - positionil);
			
			float weight_small = 0.0f;
			float weight_big = 0.0f;
			
			// small
			if(use_small &&
			   dist >= 0.0f && dist < small_max_dist)
			{
				float f = 1.0f / pow(dist, rsm_fade_power);
				weight_small += f;
			}
			
			// big
			if(dist >= small_max_dist
			   || !use_small)
			{
				float f = saturate(1.0f - dist / big_max_dist);
				weight_big += f*f;
			}
			
			// diffuse
			if(use_diffuse)
			{
				float weight_difuse = saturate(m1 * m2 * (weight_small+weight_big));
				accum += coloril * weight_difuse;
			}
			
			// specular
			if(use_specular)
			{
				float3 reflect_dir = reflect(-light_dir_sample_w, normalw);
				float spec = pow(saturate(dot(reflect_dir, view_dirw)), specular_gloss);
				float weight_specular = spec * m2 * specular_power;
				accum += coloril * weight_specular;
				// надо учитывать веса !!
			}
		}

		accum = accum * rsm_brightness / rsm_samples;
		accum = accum / (1 + accum);

		float gray = dot(accum, LUMINANCE_VECTOR);
		accum = lerp(gray, accum, rsm_saturation);

		accum = accum * Ldynamic_color.xyz;

		return accum;
	}

	/*
		Spatial filter
	*/

	// 0 - disabled
	// 1 - halton sequence (2, 3) discretized to 7x7 region
	// 2 - sample region 3x3
	// 3 - sample region 2x2
	
	#if rsm_sf_type == 1
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
	#elif rsm_sf_type == 2
		#define filter_size 9
		#define get_offset(i) offsets[i]
		// sample region 3x3
		static const int2 offsets[filter_size] =
		{
			-1, -1,	-1,  0,	-1,  1,
			 0, -1,	 0,  0,	 0,  1,
			 1, -1,	 1,  0,	 1,  1,
		};
	#elif rsm_sf_type == 3
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
		int plane = abs(scene.depth - scene_tap.depth) < rsm_sf_depth * depthsqr;
		plane &= abs(scene.Nw - scene_tap.Nw) < rsm_sf_normal;
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

	uint rsm_need_reprojection(float4 rsm, float2 tc, float2 tc_next, float2 pos2d, float depth)
	{
		if(!is_in_quad(tc_next))
			return 0;

		float Pz_prev_reprojected = G_BUFFER::load_history_packed(tc_next).z;

		float depth_threshold = rsm_tf_depth * depth;

		return abs(Pz_prev_reprojected - depth) < depth_threshold;
	}

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

			rsm = lerp(rsm, rsm_prev, rsm_tf_exp);
		}

		return rsm;
	}
#endif
