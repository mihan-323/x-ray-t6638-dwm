#ifndef	NEW_SHADOW_H
#define NEW_SHADOW_H
	#include "common.h"

	/*
		SHADOW_FILTERING

		0 - HW 2x2
		1 - HW 2x2 + Jitter
		2 - HW 2x2 + PCF 7x7
		3 - HW 2x2 + PCSS
		
		USE_VSM - Variance Shadow Mapping
	*/

	uniform float3x4 m_sunmask;
	uniform float4x4 m_shadow;
	uniform float4x4 m_shadow0;

	#ifdef ACCUM_DIRECT
		#ifdef USE_VSM
			uniform int SHADOW_CASCEDE_SCALE;
		#else
			uniform float cascede_scale; // for casceded shadows
			#define SHADOW_CASCEDE_SCALE cascede_scale
		#endif
	#else
		#define SHADOW_CASCEDE_SCALE 1
	#endif

	#ifdef ACCUM_DIRECT
		#if SHADOW_FILTERING == 1
			#undef SHADOW_FILTERING
			#define SHADOW_FILTERING 1
		#endif
	#endif
	
	#define SHADOW_BIAS 0.99985
	
	SamplerComparisonState smp_smap;

	uniform Texture2D s_smap : register(ps, t0);

	#if defined(MINMAX_SM_UNLOCK)
		uniform Texture2D s_smap_minmax;
	#endif

	float sample_smap(float3 tcproj) { return s_smap.SampleCmpLevelZero(smp_smap, tcproj.xy, tcproj.z * SHADOW_BIAS).x; }
	float sample_smap_proj(float4 tc) { return sample_smap(tc.xyz / tc.w); }
	float sample_smap_offset(float3 tcproj, float2 offset) { return sample_smap(tcproj.xyz + float3(offset, 0)); }
	float sample_smap_proj_offset(float4 tc, float2 offset) { return sample_smap_offset(tc.xyz / tc.w, offset); }

	float sample_smap(float2 uv, float2 offset, float zreceiver, float zscale = 1.0f, float zbias = 0.0f) 
	{ 
		return s_smap.SampleCmpLevelZero(smp_smap, uv + offset, zreceiver * zscale - zbias).x;
	}
	
	float screen_space_shadow
	(
		float2 tc, 
		G_BUFFER::GBD gbd, 
		float3 vslightvec, 
		int samples,
		float size_scale,
		float normal_scale
	);
	
	#if SHADOW_FILTERING == 1

		float accum_shadow(float4 tc)
		{
			float3 tcproj = tc.xyz / tc.w;

			float jitter = noise::get_6(tcproj.xy * 2048, 1, true);

			float accum = 0;

			float2 new_sincon;

			float r = 0.001;

			sincos(0, new_sincon.x, new_sincon.y);
			accum += sample_smap_offset(tcproj, new_sincon * jitter * r);

			sincos(2.0944, new_sincon.x, new_sincon.y);
			accum += sample_smap_offset(tcproj, new_sincon * jitter * r);

			sincos(4.1888, new_sincon.x, new_sincon.y);
			accum += sample_smap_offset(tcproj, new_sincon * jitter * r);

			accum /= 3.0;

			return accum;
		}

	#elif SHADOW_FILTERING == 2

		float accum_shadow(in float4 tc)
		{
			float3 tcproj = tc.xyz / tc.w;

			float s = 0;
			
			#ifndef SM_4_0
				float2 stc = SMAP_size * tcproj.xy + float2(0.5, 0.5), tcs = floor(stc);

				float2 fc = stc - tcs;

				tcs /= SMAP_size;

				int row, col;

				// loop over the rows
				[unroll] for(row = -3; row <= 3; row += 2)
				[unroll] for(col = -3; col <= 3; col += 2)
				{
					float4 g = s_smap.Gather(smp_nofilter, tcs, int2(col, row));

					float4 v = (tcproj.zzzz <= g) ? (1) : (0); 

					float4 shadow_tap;

					if(row == -3) // top row
					{
						if		(col == -3) shadow_tap = float4(1.0 - fc.x, 1.0, 1.0 - fc.y, (1.0 - fc.x) * (1.0 - fc.y)); // left
						else if	(col ==  3) shadow_tap = float4(1.0f, fc.x, fc.x * (1.0 - fc.y), 1.0 - fc.y); // right
						else 				shadow_tap = float4(1.0, 1.0, 1.0 - fc.y, 1.0 - fc.y); // center
					}
					else if(row == 3)  // bottom row
					{
						if		(col == -3)	shadow_tap = float4((1.0 - fc.x) * fc.y, fc.y, 1.0, (1.0 - fc.x)); // left
						else if	(col ==  3)	shadow_tap = float4(fc.y, fc.x * fc.y, fc.x, 1.0); // right
						else				shadow_tap = float4(fc.y, fc.y, 1.0, 1.0); // center
					}
					else // center rows
					{
						if		(col == -3)	shadow_tap = float4((1.0 - fc.x), 1.0, 1.0, (1.0 - fc.x)); // left
						else if	(col ==  3)	shadow_tap = float4(1.0, fc.x, fc.x, 1.0); // right
						else				shadow_tap = float4(1.0, 1.0, 1.0, 1.0); // center
					}

					s += dot(shadow_tap, v);
				}
			#else
				for(int i = -3; i <= 3; ++i)
				for(int j = -3; j <= 3; ++j)
				{
					s += sample_smap_offset(tcproj, float2(i, j) / SMAP_size);
				}
			#endif

			return s / 49.0;
		}

	#elif SHADOW_FILTERING == 3
/*
		#define PCF_NUM_SAMPLES 16 
		#define BLOCKER_SEARCH_NUM_SAMPLES 16 
		#define NEAR_PLANE 0.02f 
		#define LIGHT_SIZE_UV 3
		
		float2 poissonDisk[16] = { 
			float2( -0.94201624, -0.39906216 ), 
			float2( 0.94558609, -0.76890725 ), 
			float2( -0.094184101, -0.92938870 ), 
			float2( 0.34495938, 0.29387760 ), 
			float2( -0.91588581, 0.45771432 ), 
			float2( -0.81544232, -0.87912464 ), 
			float2( -0.38277543, 0.27676845 ), 
			float2( 0.97484398, 0.75648379 ), 
			float2( 0.44323325, -0.97511554 ), 
			float2( 0.53742981, -0.47373420 ), 
			float2( -0.26496911, -0.41893023 ), 
			float2( 0.79197514, 0.19090188 ), 
			float2( -0.24188840, 0.99706507 ), 
			float2( -0.81409955, 0.91437590 ), 
			float2( 0.19984126, 0.78641367 ), 
			float2( 0.14383161, -0.14100790 ) 
		}; 
		
		//Parallel plane estimation 
		float PenumbraSize(float zReceiver, float zBlocker) 
		{ 
			return (zReceiver - zBlocker) / zBlocker; 
		} 
		
		//This uses similar triangles to compute what  
		//area of the shadow map we should search 
		void FindBlocker(out float avgBlockerDepth, out float numBlockers, float2 uv, float zReceiver) 
		{ 
			float searchWidth = LIGHT_SIZE_UV * (zReceiver - NEAR_PLANE) / zReceiver; 
			
			float blockerSum = 0; 
			numBlockers = 0; 
			
			for(int i = 0; i < BLOCKER_SEARCH_NUM_SAMPLES; ++i) 
			{ 
				float shadowMapDepth = s_smap.SampleLevel(smp_nofilter, uv + poissonDisk[i] * searchWidth, 0); 
				if (shadowMapDepth < zReceiver) 
				{ 
					blockerSum += shadowMapDepth; 
					numBlockers++; 
				} 
			} 
			
			avgBlockerDepth = blockerSum / numBlockers; 
		} 
		
		float PCF_Filter(float2 uv, float zReceiver, float filterRadiusUV) 
		{ 
			float sum = 0.0f; 
			for(int i = 0; i < PCF_NUM_SAMPLES; ++i) 
			{ 
				float2 offset = poissonDisk[i] * filterRadiusUV; 
				sum += s_smap.SampleCmpLevelZero(smp_smap, uv + offset, zReceiver); 
			}
			
			return sum / PCF_NUM_SAMPLES; 
		}
		
		float PCSS(float3 coords) 
		{ 
			float2 uv = coords.xy; 
			float zReceiver = coords.z; // Assumed to be eye-space z in this code 
			
			// STEP 1: blocker search 
			float avgBlockerDepth = 0; 
			float numBlockers = 0; 
			FindBlocker(avgBlockerDepth, numBlockers, uv, zReceiver); 
			
			//There are no occluders so early out (this saves filtering) 
			if(numBlockers < 1) 
				return 1.0f; 
			
			// STEP 2: penumbra size 
			float penumbraRatio = PenumbraSize(zReceiver, avgBlockerDepth);     
			float filterRadiusUV = penumbraRatio * LIGHT_SIZE_UV * NEAR_PLANE / coords.z; 
			
			// STEP 3: filtering 
			return PCF_Filter(uv, zReceiver, filterRadiusUV); 
		} 
		

		float accum_shadow(in float4 tc)
		{
			return PCSS(tc.xyz / tc.w);
		}

*/

		// #define PCSS_BLOCKER_SEARCH_SAMPLES 16
		// #define PCSS_BLOCKER_SEARCH_BIAS 0.0001f
		
		// #define PCSS_FILTER_BOKEH 0.2f

		// #define PCSS_LIGHT_SIZE_PIXELS 250.0f // pixels on shadow map
		
		#ifdef ACCUM_DIRECT
		#define PCSS_LIGHT_SIZE_PIXELS 250.0f // pixels on shadow map
		#else
		#define PCSS_LIGHT_SIZE_PIXELS 25.0f
		#endif

		#define PCSS_BLOCKER_SEARCH_SAMPLES 16
		#define PCSS_BLOCKER_SEARCH_BIAS 0.02f / SHADOW_CASCEDE_SCALE
		
		#define PCSS_FILTER_PIXELS_MIN 2.0f
		#define PCSS_FILTER_PIXELS_MAX 50.0f
		
		#define PCSS_FILTER_SAMPLES 36
		#define PCSS_FILTER_BIAS 0.045f / SHADOW_CASCEDE_SCALE
		
		#define FRUSTUM_UV_BORDER_SIZE 70.0f
		#define FRUSTUM_UV_BORDER_MUL 2.0f
		
		#define SHADOW_FILTER_BOKEH 0.25f
		
		// #define SHADOW_HARD_MIN_DISTANCE 0.035f
		// #define SHADOW_HARD_MAX_DISTANCE 0.25f
		
		void calc_golden_angle(in float i, in float samples, out float2 offset, out float dist)
		{
			float golden_angle = 2.4f;
			float samples_sqr = sqrt(0.5f + samples);
			float theta = golden_angle * i;
			sincos(theta, offset.y, offset.x);
			dist = sqrt(0.5f + i) / samples_sqr;
			offset *= dist;
		}

		float2 remap_uv(float2 uv, float2 offset, float max_size)
		{
			float2 uv_test = uv + offset;
		#ifdef ACCUM_DIRECT
			if(uv_test.x > max_size)
				uv_test.x = uv_test.x - (uv_test.x - max_size) * 2.0f;
			if(uv_test.y > max_size)
				uv_test.y = uv_test.y - (uv_test.y - max_size) * 2.0f;
			if(uv_test.x < 0.0f)
				uv_test.x = -uv_test.x;
			if(uv_test.y < 0.0f)
				uv_test.y = -uv_test.y;
		#endif
			return uv_test;
		}			

		float4 shadow_fetch4(float2 tc)
		{
		#if defined(SM_4_1) || defined(SM_5_0) 
			return s_smap.Gather(smp_nofilter, tc.xy);
		#else
			return float4(s_smap.SampleLevel(smp_nofilter, tc + float2(-0.5f,  0.5f) / SMAP_size, 0).x,
				          s_smap.SampleLevel(smp_nofilter, tc + float2( 0.5f,  0.5f) / SMAP_size, 0).x,
				          s_smap.SampleLevel(smp_nofilter, tc + float2( 0.5f, -0.5f) / SMAP_size, 0).x,
				          s_smap.SampleLevel(smp_nofilter, tc + float2(-0.5f, -0.5f) / SMAP_size, 0).x);
		#endif
		}

		float accum_shadow(in float4 proj)
		{
			float2 uv = proj.xy / proj.w;
			float2 pos2d = uv.xy * SMAP_size;
			float receiver = proj.z / proj.w;

			// float shadow_min = sample_smap(uv, 0.0f, receiver, 1.0f, SHADOW_HARD_MIN_DISTANCE / SMAP_size);
			// float shadow_max = sample_smap(uv, 0.0f, receiver, 1.0f, SHADOW_HARD_MAX_DISTANCE / SMAP_size);
			// float small_shadow = smoothstep(1.0f, 0.5f, saturate(shadow_max - shadow_min));
			
			float uv_weight = 1.0f;
			
		#ifdef ACCUM_DIRECT
			float4 uv_borders = float4(FRUSTUM_UV_BORDER_SIZE, SMAP_size - FRUSTUM_UV_BORDER_SIZE, 0.0f, SMAP_size);
			float4 uv_weights = smoothstep(uv_borders.xyxy, uv_borders.zwzw, pos2d.xxyy);
			uv_weight += max(max(uv_weights.z, uv_weights.w), max(uv_weights.x, uv_weights.y)) * FRUSTUM_UV_BORDER_MUL;
		#endif
		
			// Jitter
			float2 hash = noise::hash22(uv * SMAP_size);
			
			// Blocker
			float blocker_sum = 0.0f;
			float blocker_weight = 0.0f;
			for(int i = 0; i < PCSS_BLOCKER_SEARCH_SAMPLES; i++)
			{
				float2 offset; float dist;
				calc_golden_angle(i + hash, PCSS_FILTER_SAMPLES, offset, dist);
				float3(offset, dist) *= (float)min(PCSS_LIGHT_SIZE_PIXELS, PCSS_FILTER_PIXELS_MAX) / SMAP_size;
				dist *= PCSS_BLOCKER_SEARCH_BIAS;
				float2 uv_sample = remap_uv(uv, offset, 1.0f);
				float4 blocker_fetch = shadow_fetch4(uv_sample);
				float4 bocker_compare = step(blocker_fetch, receiver - dist);
				blocker_sum += dot(blocker_fetch, bocker_compare);
				blocker_weight += dot(bocker_compare, 1.0f);
			}

			if(blocker_weight < 1.0f)
				return 1.0f;
			blocker_sum /= blocker_weight;

			float penumbra = (receiver - blocker_sum) / blocker_sum;
			
			// Filter
			float shadow_sum = 0.0f;
			float shadow_bokeh = 0.0f;
			float shadow_weight = 0.0f;
			for(int i = 0; i < PCSS_FILTER_SAMPLES; i++)
			{
				float2 offset; float dist;
				calc_golden_angle(i + hash, PCSS_FILTER_SAMPLES, offset, dist);
				float filter_weight = penumbra * PCSS_LIGHT_SIZE_PIXELS * SHADOW_CASCEDE_SCALE * uv_weight;
				filter_weight = clamp(filter_weight, PCSS_FILTER_PIXELS_MIN, PCSS_FILTER_PIXELS_MAX);
				float3(offset, dist) *= filter_weight / SMAP_size;
				float2 uv_sample = remap_uv(uv, offset, 1.0f);
				dist *= PCSS_FILTER_BIAS;
				float shadow_sample = sample_smap(uv_sample, 0.0f, receiver, 1.0f, dist);
				shadow_sum += shadow_sample;
				shadow_bokeh = max(shadow_bokeh, shadow_sample);
				shadow_weight += 1.0f;
			}

			if(shadow_weight < 1.0f)
				return 1.0f;
			shadow_sum /= shadow_weight;
			shadow_sum = lerp(shadow_sum, shadow_bokeh, SHADOW_FILTER_BOKEH);

			return shadow_sum;
		}

	#else

		float accum_shadow(float4 tc)	
		{ 
			return sample_smap_proj(tc);
		}

	#endif

	#ifdef MINMAX_SM_UNLOCK
		float sample_smap_minmax(float3 tcproj) 
		{
			float minmax = s_smap_minmax.SampleLevel(smp_nofilter, tcproj.xy, 0).x;

			bool umbra = ((minmax.x < 0) && (minmax.x > -tcproj.z));

			[branch] if(umbra)
				return 0;
			else
				return sample_smap(tcproj); 
		}

		float sample_smap_minmax_proj(float4 tc) 
		{
			float3 tcproj = tc.xyz / tc.www;

			return sample_smap_minmax(tcproj);
		}

		float accum_shadow_minmax(float4 tc)
		{
			float3 tcproj = tc.xyz / tc.www;

			bool cheap_path = false, full_light = false;

			float4 plane0 = s_smap_minmax.Gather(smp_nofilter, tcproj.xy, int2(-1, -1));
			float4 plane1 = s_smap_minmax.Gather(smp_nofilter, tcproj.xy, int2( 1, -1));
			float4 plane2 = s_smap_minmax.Gather(smp_nofilter, tcproj.xy, int2(-1,  1));
			float4 plane3 = s_smap_minmax.Gather(smp_nofilter, tcproj.xy, int2( 1,  1));

			bool plane = all((plane0 >= (0).xxxx) * (plane1 >= (0).xxxx) * (plane2 >= (0).xxxx) * (plane3 >= (0).xxxx));

			[flatten] if(!plane) // if there are no proper plane equations in the support region
			{
				bool no_plane = all((plane0 < (0).xxxx) * (plane1 < (0).xxxx) * (plane2 < (0).xxxx) * (plane3 < (0).xxxx));

				float4 z = (tcproj.z - 0.0005).xxxx;

				bool reject = all((z > -plane0) * (z > -plane1) * (z > -plane2) * (z > -plane3)); 

				[flatten] if(no_plane && reject)
				{
					full_light = false;
					cheap_path = true;
				}
			}
			else // plane equation detected
			{
				// compute corrected z for texel pos
				static const float scale = float(SMAP_size / 4);

				float2 fc = frac(tcproj.xy * scale);

				float z = lerp(lerp(plane0.y, plane1.x, fc.x), lerp(plane2.z, plane3.w, fc.x), fc.y);

				// do minmax test with new z
				full_light = ((tcproj.z - 0.0001) <= z);

				cheap_path = true; 
			}

			[branch] if(cheap_path)
			{
				[branch] if(full_light == true)
				{
					return 1;
				}
				else
				{
					float shadow_simple = sample_smap(tcproj);

					if(shadow_simple > 0.7)
					{
						return 1;
					}
					else
					{
						return lerp(0, 1, shadow_simple / 0.7);
					}
				}
			}
			else
			{
				float shadow = accum_shadow(tc);

				if(shadow > 0.7)
				{
					return 1;
				}
				else
				{
					return lerp(0, 1, shadow / 0.7);
				}
			}
		}
	#endif

	#ifndef VOLUME_SHADOW_SAMPLES
		#define VOLUME_SHADOW_SAMPLES 50
	#endif

	#ifndef dwframe_used
	#define dwframe_used
		uniform int dwframe; // current frame id
	#endif

	float accum_volumetric_shadow(float4 shpos, float3 vspos, int2 pos2d, bool new_jitter = false)
	{
		if(vspos.z <= 0.01)
			return 1;

		float3 direction;
		float4 delta;

		if(new_jitter)
		{
			int jitter_size = 64;

			float jitter = jitter0.Sample(smp_jitter, (float2)pos2d / jitter_size).x;

			float coeff = (VOLUME_SHADOW_SAMPLES - jitter) / (VOLUME_SHADOW_SAMPLES * VOLUME_SHADOW_SAMPLES);	
			
			direction = vspos * coeff;
			delta = mul(m_shadow, float4(direction, 0));
		}
		else
		{
			direction = vspos / VOLUME_SHADOW_SAMPLES;
			delta = mul(m_shadow, float4(direction, 0));

			float jitter = noise::hash13(float3(pos2d, dwframe));
			delta *= jitter;
			direction *= jitter;
		}

		float depth_cycle = vspos.z;

		float4 shpos_cycle = shpos - delta * (1 + rcp(255));

		float volumetric = 0;

		for(int i = 0; i < VOLUME_SHADOW_SAMPLES; ++i)
		{
			depth_cycle -= direction.z;

			if(direction.z <= 0.3 - depth_cycle)
				continue;

			float3 shpos_p_curr = shpos_cycle.xyz / shpos_cycle.w;
			shpos_p_curr.z += 0.0003; // depth bias

			#ifdef MINMAX_SM_UNLOCK
				volumetric += sample_smap_minmax(shpos_p_curr);
			#else
				volumetric += sample_smap(shpos_p_curr);
			#endif

			shpos_cycle -= delta;
		}

		return saturate(volumetric / VOLUME_SHADOW_SAMPLES);
	}

	float shadow_rain(float4 tc, float2 tcJ) // jittered sampling
	{
		float4 r;

		const float scale = 4.0f / float(SMAP_size);

		float4 J0 = jitter0.Sample(smp_linear, tcJ) * scale;
		float4 J1 = jitter1.Sample(smp_linear, tcJ) * scale;

		r.x = sample_smap_proj_offset(tc, J0.xy);
		r.y = sample_smap_proj_offset(tc, J0.wz);
		r.z = sample_smap_proj_offset(tc, J1.xy);
		r.w = sample_smap_proj_offset(tc, J1.wz);

		return dot(r, 0.25);
	}
	
	float4 proj_to_screen(float4 proj)
	{
		float4 screen = proj / proj.a;
		screen.xy = screen.xy * 0.5f + 0.5f;
		screen.y = 1.0f - screen.y;
		return screen;
	}

	// расположение семплов на маленьких дистанциях лучше
	// ломается на дальних дистанциях, из-за неправильной проекции
	// хорошо показывает себя до ~0.5 метра
	float screen_space_shadow_hud
	(
		float2 tc, 
		G_BUFFER::GBD gbd, 
		float3 vslightvec, 
		int samples,
		float size_scale,
		float normal_scale
	)
	{
		bool is_hud = gbd.mask;
		if(!is_hud) return 1.0f;
		
		float bias = 0.00005f; // НЕЛИНЕЙНЫЙ!!!!
		// float threshold = DEVX; // НЕЛИНЕЙНЫЙ!!!!
		
		// pos
		float3 vspos = gbd.P + gbd.N * normal_scale;
		float4 pscpos = mul(m_P, float4(vspos, 1.0f));
		pscpos = proj_to_screen(pscpos);
		float3 pscpos_curr = pscpos.xyz;

		// pos with light dir
		float scale = size_scale / samples;
		float3 vsdir = -vslightvec * scale;
		float3 vspos_next = vspos + vsdir;
		float4 pscpos_next = mul(m_P, float4(vspos_next, 1.0f));
		pscpos_next = proj_to_screen(pscpos_next);

		// direction only
		float3 pscdir = pscpos_next.xyz - pscpos_curr;

		for(int i = 0; i < samples; i++)
		{
			pscpos_curr += pscdir;
			if(!is_in_quad(pscpos_curr.xy))	return 1.0f;
			float vsdepth_hit = G_BUFFER::load_depth(pscpos_curr.xy);
			uint mask = G_BUFFER::load_hud_mask(pscpos_curr.xy);
			if(vsdepth_hit <= 0.01f || (mask * !is_hud)) return 1.0f;
			float3 fvs_pos_hit = float3(pscpos_curr.xy, 1.0f);
			float4 fpvspos_hit = G_BUFFER::vs_vp(fvs_pos_hit * vsdepth_hit, 1.0f);
			float pscdepth_hit = fpvspos_hit.z / fpvspos_hit.w;
			// if(pscdepth_hit > pscpos_curr.z + threshold && !is_hud) return 1.0f;
			if(pscdepth_hit + bias < pscpos_curr.z) return 0.0f;
		}

		return 1.0f;
	}
	
	float screen_space_shadow(float2 tc, G_BUFFER::GBD gbd, float3 vslightvec)
	{
		// try hud shadows
		// if(gbd.mask)
			return screen_space_shadow_hud(tc, gbd, vslightvec, 25, 0.05, 0.0005);

	}
	
	void shadow_lerp_coeff(float4 PS, float s, G_BUFFER::GBD gbd, float3 dir, out float coeff, out float shadow)
	{
		static int samples = 15; // 30
		static float size = 0.25; // 0.13
		
		float3 dir1 = -dir * size / samples;

		coeff = 0;
		shadow = 1;

		float l = length(gbd.P);

		for(int i = 0; i < samples; i++)
		{
			float3 P1 = gbd.P + dir1 * i;
			float2 tc1 = G_BUFFER::vs_tc(P1);
			
			if(tc1.x < 0 || tc1.y < 0 || tc1.x > 1 || tc1.y > 1)
				continue;
			
			float3 Phit = G_BUFFER::load_position(tc1);
			
			float dist = length(P1) - length(Phit);
			
			if(dist > 0.002 * l && dist < 0.02 * l)
			{
				coeff = 1 - length(P1 - gbd.P) / size;
				shadow = 0;
				break;
			}
		}

		float blocker = s_smap.Sample(smp_rtlinear, PS.xy / PS.w);
		float full = PS.z / PS.w;
		
		float coeff1 = s - saturate(1250 * (full - blocker));

		coeff = lerp(coeff, 1, coeff1) * !shadow;
	}
	
	// Variance shadow mapping
	
	#if SHADOW_FILTERING < 2
		#define VSM_LOW
	#endif
	
	uniform Texture2D s_vsm;
	
	float2 shadowRes()
	{
		float2 res;
		s_vsm.GetDimensions(res.x, res.y);
		return res;
	}

	float2 sampleSmap(float2 tc)
	{
		return s_vsm.SampleLevel(smp_rtlinear, tc, 0).xy;
	}

	float3 sampleVsmAreaLow(float2 tc)
	{
		int r = 3;
		
		float3 areaCenter = sampleSmap(tc).xyx;

		float3 areaAvg = areaCenter;
		areaAvg.z = 0;
		
		float3 weightSum = 1;
		
		static int2 p[5] = { int2(0, 0), int2(1, 0), int2(-1, 0), int2(0, 1), int2(0, -1) };
		
		for(int i = 0; i < 5; ++i)
		{
			float2 b = p[i] * 2.5;
			float2 tap = tc + b / shadowRes();
			
			if(!is_in_quad(tap)) 
				continue;
			
			float3 areaCurr = sampleSmap(tap).xyx;
			
			float weight = b.x*b.x + b.y*b.y;
			weightSum.xy += weight;
			
			areaAvg.xy += areaCurr.xy * weight;
			areaAvg.z = max(areaAvg.z, abs(areaCurr.z - areaCenter.z));
		}
		
		return areaAvg / weightSum;
	}
	
	float3 sampleVsmArea(float2 tc)
	{
		int r = 3;
		
		float3 areaCenter = sampleSmap(tc).xyx;

		float3 areaAvg = areaCenter;
		areaAvg.z = 0;
		
		float3 weightSum = 1;
		
		for(int i = -r; i <= r; ++i)
		for(int j = -r; j <= r; ++j)
		{
			float2 b = float2(i, j);
			float2 tap = tc + b / shadowRes();
			
			if(!is_in_quad(tap)) 
				continue;
			
			float3 areaCurr = sampleSmap(tap).xyx;
			
			float weight = r * saturate(1.0 / sqrt(b.x*b.x + b.y*b.y));
			weightSum.xy += weight;
			
			areaAvg.xy += areaCurr.xy * weight;
			areaAvg.z = max(areaAvg.z, abs(areaCurr.z - areaCenter.z));
		}
		
		return areaAvg / weightSum;
	}
	
	float linstep(float min, float max, float v)
	{
		return saturate((v - min) / (max - min));
	}
	
	float reduceLightBleeding(float pMax, float amount) 
	{
		// Remove the [0, amount] tail and linearly rescale (amount, 1].    
		return linstep(amount, 1, pMax); 
	} 
	
	float shadowAccumVSM(float4 shadowPositionProj)
	{
		float3 hpos = shadowPositionProj.xyz / shadowPositionProj.w;
		
		float2 tc = hpos.xy;
		float t = hpos.z;
		
		float r = 3 / (SHADOW_CASCEDE_SCALE + 1);
		
		#ifdef VSM_LOW
			float3 area = sampleVsmAreaLow(tc);
		#else
			float3 area = sampleVsmArea(tc);
		#endif
		
		float2 m = area.xy;
		float isPenumbra = step(0.0001, area.z);
		
		float hard = t >= m.x;
		
		// One-tailed inequality valid if t > moments.x    
		float p = t <= m.x;
		
		// Compute variance.    
		float v = m.y - m.x*m.x;
		v = max(v, r / (1 << 23));
		
		// Compute probabilistic upper bound.  
		float d = t - m.x;		
		float pMax = v / (v + d*d);
		
		float f = lerp(0.99, 0.50, isPenumbra);
		float s = reduceLightBleeding(pMax, f);
		
		s = max(s, p);
		
		return s;
	}
#endif
