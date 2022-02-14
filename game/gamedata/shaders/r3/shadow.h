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

	SamplerComparisonState smp_smap;

	uniform Texture2D s_smap : register(ps, t0);

	#if defined(MINMAX_SM_UNLOCK)
		uniform Texture2D s_smap_minmax;
	#endif

	float sample_smap(float3 tcproj) { return s_smap.SampleCmpLevelZero(smp_smap, tcproj.xy, tcproj.z).x; }
	float sample_smap_proj(float4 tc) { return sample_smap(tc.xyz / tc.w); }
	float sample_smap_offset(float3 tcproj, float2 offset) { return sample_smap(tcproj.xyz + float3(offset, 0)); }
	float sample_smap_proj_offset(float4 tc, float2 offset) { return sample_smap_offset(tc.xyz / tc.w, offset); }

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

		// NVIDIA PCSS

		// Dynamic tap search region, bigger - better quality for big objects, but lower for small objects
		#define PCSS_TAP_SEARCH_SAMPLES 9

		// Dynamic tap search region, bigger - better quality for big objects, but lower for small objects
		#define PCSS_TAP_SEARCH_REGION 20

		// Number of samples for soft shadow is calculated automatically, you only need to specify the maximum and minimum number of samples
		#define PCSS_SAMPLES_MIN 5
		#define PCSS_SAMPLES_MAX 25

		// Blur tap scale
		#define PCSS_RADIUS_SCALE 0.15

		// Fixed blur tap additional scale
		#define PCSS_RADIUS_ADD 0.001

		float accum_shadow(in float4 tc)
		{
			float3 tcproj = tc.xyz / tc.w;

			float depth_accum = 0;

			int check = 0;
			bool out_of_range = false;

			int2 res_tcproj = tcproj.xy * SMAP_size;

			float jitter = noise::get_6((tcproj.xz + tcproj.xy - tcproj.xx) * screen_res.xy * SMAP_size / 256, 1) * 0.5 + 0.5;

			float golden_angle = 2.4f;

			float zreceiver = tcproj.z - 0.0001;

			[unroll(PCSS_TAP_SEARCH_SAMPLES)] for(int dsi = 0; dsi < PCSS_TAP_SEARCH_SAMPLES; dsi++)
			{
				float r = sqrt(dsi + 0.5f) / sqrt(PCSS_TAP_SEARCH_SAMPLES);

				float theta = dsi * golden_angle + jitter;

				float sine, cosine;
				sincos(theta, sine, cosine);

				float2 res_tcproj_tap = res_tcproj + PCSS_TAP_SEARCH_REGION * float2(r * cosine, r * sine) * SHADOW_CASCEDE_SCALE;

				float depth_light = s_smap.Load(int3(res_tcproj_tap, 0)).x;

				// Check search region, cant out of tc
				if(res_tcproj_tap.x <= 0 || res_tcproj_tap.y <= 0 || res_tcproj_tap.x / SMAP_size >= 1 || res_tcproj_tap.y / SMAP_size >= 1)
					out_of_range = true;

				if(!out_of_range && depth_light < zreceiver)
				{
					depth_accum += depth_light;
					check++;
				}
			}

			depth_accum /= check;

			float penumbra = (tcproj.z - depth_accum) / depth_accum;

			float tap_scaled = clamp((penumbra + PCSS_RADIUS_ADD) * PCSS_RADIUS_SCALE, 0, 0.0055);

			float shadow_accum = 0;

			float minmax_samples_range = PCSS_SAMPLES_MAX - PCSS_SAMPLES_MIN;
			float min_samples = PCSS_SAMPLES_MIN;

			// Skip small offsets
			if(step(tap_scaled, 0.001))
				return sample_smap(tcproj);

			float auto_samples_2 = min_samples + minmax_samples_range * saturate(tap_scaled / 0.0055); // min..max samples

			tap_scaled *= SHADOW_CASCEDE_SCALE;

			// For other smaps
			#if defined(ACCUM_BASE)
				if(SMAP_size == 4096)
					tap_scaled *= 0.5;
				else if(SMAP_size == 3072)
					tap_scaled *= 0.66666667;
				else if(SMAP_size == 2048)
					tap_scaled *= 1;
				else if(SMAP_size == 1024)
					tap_scaled *= 2;
			#endif

			#if defined(ACCUM_SHADOW_NEED_BOKEH)
				float shadow_max = 0;
			#endif

			[unroll] for(int i = 0; i < (int)auto_samples_2; i++)
			{
				float r = sqrt(i + 0.5f) / sqrt(auto_samples_2);

				float theta = i * golden_angle + jitter;

				float sine, cosine;
				sincos(theta, sine, cosine);
  
				float2 bias = tap_scaled * float2(r * cosine, r * sine);

				float3 tcproj_bias = float3(saturate(tcproj.xy + bias), tcproj.z);

				float shadow_current = sample_smap(tcproj_bias);

				#if defined(ACCUM_SHADOW_NEED_BOKEH)
					shadow_max = max(shadow_current, shadow_max);
				#endif

				shadow_accum += shadow_current;
			}

			shadow_accum /= auto_samples_2;

			#if defined(ACCUM_SHADOW_NEED_BOKEH)
				shadow_accum = lerp(shadow_accum, shadow_max, 0.1);
			#endif

			return shadow_accum;
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

	float screen_space_shadow
	(
		float2 tc, 
		G_BUFFER::GBD gbd, 
		float3 vslightvec, 
		int samples,
		float size_scale,
		float normal_scale
	)
	{
		// non-normalized projected screen pos
		float3 vspos = gbd.P + gbd.N * normal_scale;
		float4 pscpos = G_BUFFER::vs_vp(vspos, 1);
		pscpos /= pscpos.w;
		pscpos.xy = pscpos.xy * 0.5 + 0.5;
		pscpos.y = 1 - pscpos.y;
		float3 pscpos_curr = pscpos.xyz;

		// non-normalized projected screen pos with direction
		float scale = size_scale / samples;
		float3 vsdir = -vslightvec * scale;
		float3 vspos_next = vspos + vsdir;
		float4 pscpos_next = G_BUFFER::vs_vp(vspos_next, 1);
		pscpos_next /= pscpos_next.w;
		pscpos_next.xy = pscpos_next.xy * 0.5 + 0.5;
		pscpos_next.y = 1 - pscpos_next.y;

		// non-normalized projected direction
		float3 pscdir = pscpos_next.xyz - pscpos_curr;

		for(int i = 0; i < samples; i++)
		{
			pscpos_curr += pscdir;
			if(!is_in_quad(pscpos_curr.xy))	return 1;
			float vsdepth_hit = G_BUFFER::load_depth(pscpos_curr.xy);
			if(vsdepth_hit <= 0.01) return 1;
			float3 fvs_pos_hit = float3(pscpos_curr.xy, 1);
			float4 fpvspos_hit = G_BUFFER::vs_vp(fvs_pos_hit * vsdepth_hit, 1);
			float pscdepth_hit = fpvspos_hit.z / fpvspos_hit.w;
			if(pscdepth_hit < pscpos_curr.z) return 0;
		}

		return 1;
	}
	
	// Variance shadow mapping
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

	float3 sampleVsmArea(float2 tc, int r)
	{
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
		float3 area = sampleVsmArea(tc, r);
		
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
