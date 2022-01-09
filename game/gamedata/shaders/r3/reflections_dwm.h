#if !defined(SSR_DWM_INCLUDED)
#define SSR_DWM_INCLUDED

	#define TCXDEPTH_SPACE 0
	#define VIEW_SPACE 1
	#define XSTAGE 0
	#define YSTAGE 1
	#define DEFFERED_ATTENUTATION 1
	#define LIGHTING 2

			/*
				DWM Team
				i also used their work..
				 meltac, Anonim
				 ghost recon, Dynamic shaders, OGSE, ReShade
				Last update: 15.11.2020
			*/

			/*
				#define SSR_SKYBOX_USE

				#define SSR_ROAD
				#define SSR_ROAD_RENDERED

				#define SSR_ROAD
				#define SSR_ROAD_RENDERED
				#define SSR_ROAD_RENDERED_AND_DENOISED
			*/

			/*
				Planar SSR
				http://remi-genin.fr/blog/screen-space-plane-indexed-reflection-in-ghost-recon-wildlands/
			*/

	#if defined(SSR_DWM_INCLUDED)

				#if (REFLECTIONS_QUALITY == 3)
					#define SSR_ALTERNATIVE_FAST_MODE 1 // [0, 1]
				#else
					#define SSR_ALTERNATIVE_FAST_MODE 0 
				#endif

				#define SSR_ALT_FAST_ACCURATTLY 20 //
				#define SSR_ALT_FAST_ERROR_TAP 2 //
				

				#define SSR_TRACING_TYPE TCXDEPTH_SPACE //

				//
				// #if (REFLECTIONS_QUALITY == 6)
					// #define SSR_SAMPLE_AMOUNT 200
				// #elif (REFLECTIONS_QUALITY == 5)
				#if (REFLECTIONS_QUALITY == 5)
					#define SSR_SAMPLE_AMOUNT 85
				#else
					#define SSR_SAMPLE_AMOUNT 65
				#endif

				 //
				// #if (REFLECTIONS_QUALITY == 6)
					// #define SSR_RAY_TAPS 10
				// #elif (REFLECTIONS_QUALITY == 5)
				#if (REFLECTIONS_QUALITY == 5)
					#define SSR_RAY_TAPS 15
				#else
					#define SSR_RAY_TAPS 8
				#endif

				#define SSR_RAY_SCALER 0.6 // [0.1..1.0]
				#define SSR_RAY_RADIUS 2 // [1..?]

				#define SSR_RAY_RADIUS_DYNAMIC 0 // [0, 1]
				#define SSR_RAY_RADIUS_D_HEIGHT 5 // [1..998]

				#if ((REFLECTIONS_QUALITY == 2) || (REFLECTIONS_QUALITY > 3)) && (REFLECTIONS_QUALITY != 6)
					#define SSR_FAR_PLANE 1 // [0, 1]
				#else
					#define SSR_FAR_PLANE 0 
				#endif

				#if defined(SSR_ROAD)
					#define SSR_FAR_PLANE_ALT_MIX 1 // [0, 1]
				#else
					#define SSR_FAR_PLANE_ALT_MIX 0 
				#endif

				#if (REFLECTIONS_QUALITY == 2)
					#define SSR_FAR_DISTANCE 15 // [0..sky]
				#else
					#define SSR_FAR_DISTANCE 50 
				#endif

				#define SSR_ALTERNATIVE_FAST_MODE_JITTER 1 // [0, 1]

				#if (REFLECTIONS_QUALITY >= 5)
					#define SSR_IMPROVED_JITTERING 1 // [0, 1]
				#else
					#define SSR_IMPROVED_JITTERING 0 
				#endif

				#define SSR_IMPROVED_JITTERING_SEARCH_RANGE 5 // [1..Res]
				#define SSR_IMPROVED_JITTERING_THRESHOLD 0.25 // [0..far filtered - sky]
				#define SSR_IMPROVED_JITTERING_DEBUG 0 // [0, 1]

				#define SSR_OUTRANGE_TC_CUTER 0.075 // [0..0.5]

				#if defined(SSR_ROAD)
					#define SSR_JITTER_SCALE 0.3 // [0.0..0.5]
					
					// [1..sky]
					// #if (REFLECTIONS_QUALITY == 6)
						// #define SSR_RAY_LENGTH 100 
					// #else
						#define SSR_RAY_LENGTH 65 
					// #endif
				#else
					#define SSR_JITTER_SCALE 0.05 // [0.0..0.5]

					// [1..sky]
					// #if (REFLECTIONS_QUALITY == 6)
						// #define SSR_RAY_LENGTH 100 
					// #else
						#define SSR_RAY_LENGTH 80 
					// #endif
				#endif

				#define SSR_ALPHA_MODE DEFFERED_ATTENUTATION // 0, DEFFERED_ATTENUTATION, LIGHTING

				#if (SSR_ALPHA_MODE == LIGHTING)
					#define SSR_PUDDLES_NEED_LIGHTING 0 // [0, 1] 
					#define SSR_PUDDLES_TEST_TORCH_ENABLED 0 // [0, 1] 
				#elif (SSR_ALPHA_MODE == DEFFERED_ATTENUTATION)
					#define SSR_PUDDLES_DEFFERED_ATTENUTATION 1 // [0, 1] 
				#endif

				#define SSR_SKYBOX_COLOR float3(58/255, 64/255, 78/255) // 
	#endif

			#if (SSR_RAY_RADIUS_DYNAMIC == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
				#define stepadd(low, high, high_add) step(low, high / high_add)
			#else
				#define stepadd(low, high, high_add) step(low, high)
			#endif

			#if (SSR_TRACING_TYPE == TCXDEPTH_SPACE)
				#define _position scposition
				#define _position_cycle scposition_cycle
				#define _vreflect scvreflect
			#elif (SSR_TRACING_TYPE == VIEW_SPACE)
				#define _position vsposition
				#define _position_cycle vsposition_cycle
				#define _vreflect vsvreflect
			#endif

			#if defined(SSR_ROAD_RENDERED) && (REFLECTIONS_QUALITY > 0)
				uniform Texture2D s_dtssr, s_dtssr_denoised; // smp_nofilter
			#endif

			uniform float4 wea_sky_color; // 

			#include "lmodel.h"

			float3 reflections_sample(in float2 tc, inout float error_near, in bool need_additional_test = true)
			{
				float3 reflection_near = s_image.SampleLevel(smp_rtlinear, tc, 0).xyz;
				return reflection_near;
			}

			/*bool is_in_quad(in float2 tc)
			{
				return (tc.x <= 1) && (tc.y <= 1) && (tc.x >= 0) && (tc.y >= 0);
			}*/

			/*// 0..1
			float calc_vignette(in float2 tc)
			{
				float vgn = 1;

				if(tc.y < SSR_OUTRANGE_TC_CUTER)
					vgn *= saturate(tc.y) / SSR_OUTRANGE_TC_CUTER;

				if(tc.x < SSR_OUTRANGE_TC_CUTER)
					vgn *= saturate(tc.x) / SSR_OUTRANGE_TC_CUTER;

				float2 tci = 1 - tc;

				if(tci.y < SSR_OUTRANGE_TC_CUTER)
					vgn *= saturate(tci.y) / SSR_OUTRANGE_TC_CUTER;

				if(tci.x < SSR_OUTRANGE_TC_CUTER)
					vgn *= saturate(tci.x) / SSR_OUTRANGE_TC_CUTER;

				return vgn;
			}*/

			// sky..
			#if defined(SSR_SKYBOX_USE)
				float3 sample_sky_by_wsunwrap(in float3 wsunwrap)
				{
					float3 s_wsunwrap = wsunwrap / max(abs(wsunwrap.x), max(abs(wsunwrap.y), abs(wsunwrap.z)));

					if(s_wsunwrap.y < 0.99)
						s_wsunwrap.y = s_wsunwrap.y * 2 - 1;

					float3 env0 = env_s0.Sample(smp_base, s_wsunwrap);
					float3 env1 = env_s1.Sample(smp_base, s_wsunwrap);

					return lerp(env0, env1, L_ambient.w);
				}
			#endif

			#if (REFLECTIONS_QUALITY == 6)
				#undef SSR_FAR_PLANE
				#define SSR_FAR_PLANE 0
				#undef SSR_RAY_RADIUS_DYNAMIC
				#define SSR_RAY_RADIUS_DYNAMIC 0
				#undef SSR_IMPROVED_JITTERING
				#define SSR_IMPROVED_JITTERING 1
				#undef SSR_TRACING_TYPE
				#define SSR_TRACING_TYPE TCXDEPTH_SPACE

				#define SSR_HQ_INC_SAMPLES 		5 		
				#define SSR_HQ_RAY_INCREASE 	1.75 
				#define SSR_HQ_RAY_STEPS 		16 		
				#define SSR_HQ_RAY_BASE_LENGTH 	1.2 	
				#define SSR_HQ_DIFF_THRESHOLD 	1.5
				#define SSR_HQ_RAY_REFINE 		5 		
				#define SSR_HQ_MARCH_MIN_DIST 	1 		
				#define SSR_HQ_MARCH_MAX_DIST 	235 	
				#define SSR_HQ_JITTER_SCALE 	0.3 	

				void calc_reflections_new
				(
					in uint2 pos2d, 
					in float3 scposition,  
					in float3 scvreflect,   
					in float3 wsvreflect, 
					in float frenel, 

					out float3 reflection, 
					out float error, 

					#if defined(SSR_ROAD)
						out unsigned int need_denoise,
					#endif

					out float3 reflection_sky
				)
				{
					reflection_sky = reflection = 0;
					error = 0;

					if(step(SSR_HQ_MARCH_MAX_DIST, scposition.z))
					{
						#if defined(SSR_SKYBOX_USE)
							reflection_sky = sample_sky_by_wsunwrap(wsvreflect) * wea_sky_color.xyz;
						#else
							reflection_sky = SSR_SKYBOX_COLOR;
						#endif

						return;
					}

					float depth_scale = clamp(scposition.z, SSR_HQ_MARCH_MIN_DIST, SSR_HQ_MARCH_MAX_DIST);
					float scale = SSR_HQ_RAY_BASE_LENGTH * sqrt(depth_scale) / SSR_HQ_RAY_STEPS;

					#if defined(SSR_ROAD)
						float jitter_scale = noise::get_4(pos2d.xy, 1);
					#else
						float jitter_scale = noise::get_random(pos2d.xy * screen_res.zw + timers.w);
					#endif

					scale *= lerp(1, jitter_scale, SSR_HQ_JITTER_SCALE);

					// 
					float3 scvreflect_cycle = scvreflect * scale;

					float3 scposition_cycle = scposition;

					float diff_threshold = SSR_HQ_DIFF_THRESHOLD * scale;

					// noise flag
					#if defined(SSR_ROAD)
						need_denoise = 0;
					#endif

					float2 tc_new;

					int ray_steps = SSR_HQ_RAY_STEPS * frenel;

					if(ray_steps)
					{
						for(int i = 0; i < SSR_HQ_INC_SAMPLES; i++)
						{
							scvreflect_cycle *= SSR_HQ_RAY_INCREASE;
							diff_threshold *= SSR_HQ_RAY_INCREASE;

							for(int j = 0; j < ray_steps; j++)
							{
								scposition_cycle += scvreflect_cycle;

								float2 tc_step = scposition_cycle.xy / scposition_cycle.zz;

								if(!is_in_quad(tc_step))
								{
									#if defined(SSR_ROAD)
										need_denoise = 1;
									#endif

									i = SSR_HQ_INC_SAMPLES;
									break;
								}

								float depth_new = G_BUFFER::load_depth(tc_step);

								if(depth_new <= min(0.01, scposition.z))
								{
									#if defined(SSR_ROAD)
										need_denoise = 1;
									#endif

									continue;
								}

								float depth_diff = scposition_cycle.z - depth_new;

								if(step(diff_threshold, depth_diff))
								{
									#if defined(SSR_ROAD)
										need_denoise = 1;
									#endif

									break;
								}

								if(depth_diff >= 0)
								{
									tc_new = tc_step;
									error = 1;
									i = SSR_HQ_INC_SAMPLES;
									break;
								}

								#if defined(SSR_ROAD)
									need_denoise = i >= SSR_HQ_INC_SAMPLES - 1;
								#endif
							}
						}

						if(SSR_HQ_RAY_REFINE)
						{
							scposition_cycle -= scvreflect_cycle;
							scvreflect_cycle /= SSR_HQ_RAY_REFINE;

							for(int i = 0; i < SSR_HQ_RAY_REFINE; i++)
							{
								scposition_cycle += scvreflect_cycle;

								float2 tc_step = scposition_cycle.xy / scposition_cycle.zz;

								if(!is_in_quad(tc_step))
									break;

								float depth_new = G_BUFFER::load_depth(tc_step);

								if(depth_new <= min(0.01, scposition.z))
									continue;

								float depth_diff = scposition_cycle.z - depth_new;

								if(step(diff_threshold, depth_diff))
									break;

								if(depth_diff >= 0)
								{
									tc_new = tc_step;
									error = 1;
									break;
								}
							}
						}

						reflection = reflections_sample(tc_new, error);

						error *= calc_vignette(tc_new, SSR_OUTRANGE_TC_CUTER);
					}

					#if defined(SSR_SKYBOX_USE)
						reflection_sky = sample_sky_by_wsunwrap(wsvreflect) * wea_sky_color.xyz;
					#else
						reflection_sky = SSR_SKYBOX_COLOR;
					#endif
				}
			#endif

			// ssr
			void calc_reflections
			(
				in uint2 pos2d, 
				in float2 tc, 

				#if (SSR_FAR_PLANE == 1)
					in float2 tc_far, 
				#endif

				#if (SSR_RAY_RADIUS_DYNAMIC == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
					in float dyn_height,
				#endif

				in float3 vsposition, 
				in float3 scposition,  
				in float3 vsvreflect, 
				in float3 scvreflect,   
				in float3 wsvreflect, 

				out float3 reflection_near, 
				out float error_near, 

				#if (SSR_FAR_PLANE == 1)
					out float3 reflection_far, 
					out float error_far, 
				#endif

				#if defined(SSR_ROAD) && (SSR_IMPROVED_JITTERING == 1)
					out unsigned int need_denoise,
				#endif

				out float3 reflection_sky
			)
			{
				#if defined(SSR_SKYBOX_USE)
					// sky
					reflection_sky = sample_sky_by_wsunwrap(wsvreflect) * wea_sky_color.xyz;
				#else
					reflection_sky = SSR_SKYBOX_COLOR;
				#endif

				error_near = 0;

				// noise
				//#if defined(SSR_ROAD)
				//	float jitter_scale = noise::get_random(tc);
				//#else
					//float jitter_scale = noise::get_4(tc * screen_res.xy, 1);
					float jitter_scale = noise::get_4(pos2d.xy, 1);
				//#endif
				jitter_scale = lerp(1, jitter_scale, (float)SSR_JITTER_SCALE);

				float svreflect_range = (float)SSR_RAY_LENGTH * jitter_scale / (float)SSR_SAMPLE_AMOUNT;
				//scvreflect *= svreflect_range;
				_vreflect *= svreflect_range; //

				//float3 scposition_cycle = scposition;
				float3 _position_cycle = _position; // 
				float2 tc_near;

				bool need_refine = false, need_reflection = false, end_reflection = false, check_sky_step = false;

				#if (SSR_FAR_PLANE == 1) && (SSR_FAR_PLANE_ALT_MIX == 1)
					bool check_sky_step_temp = false;
				#endif

				#if defined(SSR_ROAD) && (SSR_IMPROVED_JITTERING == 1)
					// 
					float depth_tap;
					need_denoise = 1;
				#endif

				/*float3 vsposition_next = vsposition + vsvreflect;
				float2 tcposition_next = G_BUFFER::vs_tc(vsposition_next);

				float2 ray2d = tcposition_next - tc;

				scvreflect.xy = ray2d;
				scposition_cycle.xy /= scposition.zz;*/

				int sky_step = 0;

				#if (SSR_FAR_PLANE == 1)
					error_far = 1;
				#endif

				for(unsigned int us = 0; us < SSR_SAMPLE_AMOUNT; us++)
				{
					float sampleid_current = (float)us / SSR_SAMPLE_AMOUNT;

					float scale = saturate(sampleid_current + SSR_RAY_SCALER);

					//scposition_cycle += scvreflect * scale;
					_position_cycle += _vreflect * scale;

					// unwrap
					
					//tc_near = scposition_cycle.xy / scposition_cycle.zz;
					#if (SSR_TRACING_TYPE == TCXDEPTH_SPACE)
						tc_near = _position_cycle.xy / _position_cycle.zz;
						//tc_near = _position_cycle.xy;
					#elif (SSR_TRACING_TYPE == VIEW_SPACE)
						tc_near = G_BUFFER::vs_tc(_position_cycle);
					#endif

					//bool screen_space = (tc_near.x <= 1) && (tc_near.y <= 1) && (tc_near.x >= 0) && (tc_near.y >= 0);
					bool screen_space = is_in_quad(tc_near);

					if(screen_space)
					{
						#if defined(SSR_ROAD) && (SSR_IMPROVED_JITTERING == 1)
							depth_tap = G_BUFFER::load_depth(tc_near);
						#else
							float depth_tap = G_BUFFER::load_depth(tc_near);
						#endif

						//float depth_difference = scposition_cycle.z - depth_tap;
						float depth_difference = _position_cycle.z - depth_tap;

						need_reflection = step(0, depth_difference);

						#if (SSR_FAR_PLANE == 1) && (SSR_FAR_PLANE_ALT_MIX == 1)
							check_sky_step_temp = check_sky_step;
						#endif
						check_sky_step = step(0.01, depth_tap);

						#if (SSR_FAR_PLANE == 1) && (SSR_FAR_PLANE_ALT_MIX == 1)
							if(!check_sky_step)
							{
								// ... miss geometry
								if(check_sky_step_temp && sky_step == 1)
									sky_step = 2;

								if(sky_step == 0)
									sky_step = 1;
							}
						#endif

						bool check_screen_space_depth = step(vsposition.z, depth_tap);

						#if (SSR_FAR_PLANE == 1) && (SSR_FAR_PLANE_ALT_MIX == 1)
							if(need_reflection && !check_screen_space_depth && sky_step == 1)
								error_far = 0;
						#endif

						if(need_reflection && check_sky_step && check_screen_space_depth)
						{
							bool ray_in_range = false;

							if(need_refine)
							{
								ray_in_range = true;
								us = SSR_SAMPLE_AMOUNT;
							}
							else
							{
								// hit?
								
								// stepadd(low, high, high_add) = step(low, high / high_add)
								ray_in_range = stepadd(depth_difference, SSR_RAY_RADIUS * svreflect_range * scale, float(dyn_height + 1.0));

								if(ray_in_range)
								{
									need_refine = true;
									//scposition_cycle -= scvreflect * scale;
									//scvreflect = scvreflect / jitter_scale / SSR_RAY_TAPS;
									_position_cycle -= _vreflect * scale;
									_vreflect = _vreflect / jitter_scale / SSR_RAY_TAPS;
									us = SSR_SAMPLE_AMOUNT - SSR_RAY_TAPS - 1;
								}
							}

							if(ray_in_range)
								error_near = 1;
						}
						#if defined(SSR_ROAD) && (SSR_IMPROVED_JITTERING == 1)
							else 
							{
								if(SSR_SAMPLE_AMOUNT - us == 1 && !need_refine) // penult 
								// noise ..
								//if(SSR_SAMPLE_AMOUNT == us) // last
									need_denoise = 2;
							}
						#endif
					}
					else
					{
						// this pixel is trash
						error_near = 0;
						us = SSR_SAMPLE_AMOUNT;
					}

					#if (SSR_FAR_PLANE == 1) && (SSR_FAR_PLANE_ALT_MIX == 1)
						if(us == SSR_SAMPLE_AMOUNT)
							error_far = 1;
					#endif
				}

				reflection_near = reflections_sample(tc_near, error_near);

				error_near *= calc_vignette(tc_near, SSR_OUTRANGE_TC_CUTER);

				#if (SSR_FAR_PLANE == 1)
					// far
					error_far *= calc_vignette(tc_far, SSR_OUTRANGE_TC_CUTER);

					float depth_far = G_BUFFER::load_depth(tc_far);

					if(vsposition.z > depth_far)
						error_far = 0;

					float depth_far_scaled = saturate(depth_far / SSR_FAR_DISTANCE);

					error_far *= depth_far_scaled * depth_far_scaled * depth_far_scaled;

					if(error_far > 0) 
						reflection_far = reflections_sample(tc_far, error_far, false);
					else 
						reflection_far = float3(0, 0, 0);
				#endif

				#if defined(SSR_ROAD) && (SSR_IMPROVED_JITTERING == 1)
					if(is_in_quad(tc_near))
					{
						float reflection_depth_threshold = SSR_IMPROVED_JITTERING_THRESHOLD;

						unsigned int reflection_depth_search_range = SSR_IMPROVED_JITTERING_SEARCH_RANGE;

						float
						reflection_down_right_depth = G_BUFFER::load_depth(tc_near + screen_res.zw * float2( 1, 1) * reflection_depth_search_range),
						reflection_up_left_depth 	= G_BUFFER::load_depth(tc_near - screen_res.zw * float2( 1, 1) * reflection_depth_search_range),
						reflection_down_left_depth 	= G_BUFFER::load_depth(tc_near + screen_res.zw * float2(-1, 1) * reflection_depth_search_range),
						reflection_up_right_depth 	= G_BUFFER::load_depth(tc_near - screen_res.zw * float2(-1, 1) * reflection_depth_search_range);

						// threshold check
						if(
							need_denoise == 1 &&

							!(
								abs(depth_tap - reflection_down_right_depth	) > reflection_depth_threshold
							||	abs(depth_tap - reflection_up_left_depth	) > reflection_depth_threshold
							||	abs(depth_tap - reflection_down_left_depth	) > reflection_depth_threshold
							||	abs(depth_tap - reflection_up_right_depth	) > reflection_depth_threshold
							)
						)
						{
							need_denoise = 0;
						}
						else
						{
							need_denoise = 1;
						}
					}
					else
					{
						need_denoise = 0;
					}
				#endif
			}

			void calc_reflections_fast
			(
				in uint2 pos2d,
				in float2 tc,
				//in float3 scposition,
				//in float3 scvreflect, 
				in float3 vsposition,
				in float3 vsvreflect, 
				in float3 wsvreflect, 

				out float3 reflection_obj, 
				out float error_obj, 

				out float3 reflection_sky
			)
			{
				float2 tc_new;
				float accurattly = 0.5;

				#if (SSR_ALTERNATIVE_FAST_MODE_JITTER == 1)
					//float jitter_scale = noise::get_4(tc * screen_res.xy, 1);
					//float jitter_scale = noise::get_4(pos2d.xy, 1);
					float jitter_scale = noise::get_random(tc);
					jitter_scale = lerp(1, jitter_scale, (float)SSR_JITTER_SCALE);
					//vsvreflect *= jitter_scale;
					accurattly = jitter_scale;
				#endif

				for(unsigned int us = 0; us < SSR_ALT_FAST_ACCURATTLY; us++)
				{
					//float3 scposition_cycle = scposition + scvreflect * accurattly;
					float3 vsposition_cycle = vsposition + vsvreflect * accurattly;

					//tc_new = scposition_cycle.xy / scposition_cycle.zz;
					tc_new = G_BUFFER::vs_tc(vsposition_cycle);

					bool screen_space = is_in_quad(tc_new);

					if(screen_space)
					{
						float3 vsposition_tap = G_BUFFER::load_position(tc_new);
						//float depth_tap = vsposition_tap.z;//G_BUFFER::load_depth(tc_new);

						if(vsposition_tap.z <= 0.01)
						{
							error_obj = 0;
							// const tap
							accurattly += SSR_ALT_FAST_ERROR_TAP;
							//us = SSR_ALT_FAST_ACCURATTLY;
						}
						else
						{
							//if(depth_tap < scposition.z)
							if(vsposition_tap.z < vsposition.z)
							{
								error_obj = 0;
								us = SSR_ALT_FAST_ACCURATTLY;
							}
							else
							{
								error_obj = 1;
								//accurattly = depth_tap - scposition.z;
								accurattly = length(vsposition_tap - vsposition);
							}
						}
					}
					else
					{
						error_obj = 0;
						us = SSR_ALT_FAST_ACCURATTLY;
					}
				}

				error_obj *= calc_vignette(tc_new, SSR_OUTRANGE_TC_CUTER);

				reflection_obj = reflections_sample(tc_new, error_obj);

				#if defined(SSR_SKYBOX_USE)
					//  
					reflection_sky = sample_sky_by_wsunwrap(wsvreflect) * wea_sky_color.xyz;
				#else
					reflection_sky = SSR_SKYBOX_COLOR;
				#endif

				reflection_obj = lerp(reflection_sky, reflection_obj, error_obj);
			}

			//----- wet refl

			#if defined(SSR_ROAD)
				// check wet
				// bool checkwet(in float2 tc, in float depth, out float addition_sky_m)
				// {
					// // 
					// float2 dwmb = G_BUFFER::load_dwmbuffer(tc).xz;
					// addition_sky_m = dwmb.y;

					// float wet = s_wetmap.SampleLevel(smp_nofilter, tc, 0).x;

					// #if defined(ENGINE_RENDER_DWMBUFFER)
						// wet = wet / 0.9 - 0.1;
					// #endif

					// if(depth > 30)
						// wet = 1;

					// return (dwmb.x * wet * rain_updater.x > 0.001);
				// }

				// check normals
				bool checknormals(float3 wsnormal)
				{
					float testangle = dot(wsnormal, float3(0, 1, 0));
					float threshold = -0.1;
					return step(threshold, testangle);
				}

				// plepare relf space
				void prepare_space_for_wet_refls
				(
					in float2 tc,
					in uint2 pos2d,

					//in bool need_scspace,

					out float3 vsposition,
					out float3 scposition,
					out float3 vsvreflect,
					out float3 scvreflect,
					out float3 wsvreflect,

					#if (SSR_PUDDLES_NEED_LIGHTING == 1)
						out float3 vsnormal,
					#endif

					out float frenel,
					out float attenutation,
					out float addition_sky_m,

					out bool need_reflection
				)
				{
					vsposition = scposition = vsvreflect = scvreflect = wsvreflect = float3(0, 0, 0);
					frenel = attenutation = 0;

					float depth = G_BUFFER::load_depth(tc);

					// need_reflection = checkwet(tc, depth, addition_sky_m);
					addition_sky_m = 1;
					need_reflection = true;

					//----- 
					//if(!checkwet(tc, depth)) 
					//	return 0.15;

					//----- is wet

					if(need_reflection) 
					{
						// view
						//float2 tc_norm = tc.xy * float2(2.0, -2.0) - float2(1.0, -1.0);
						//vsposition = float3(tc_norm * InvFocalLen, 1) * depth;
						vsposition = G_BUFFER::tcdepth_vs(float3(tc, depth));

						// norm view packed
						//if(need_scspace)
							scposition = float3(tc.xy, 1) * depth;

						// view
						#if (SSR_PUDDLES_NEED_LIGHTING == 1)
							vsnormal = G_BUFFER::load_normal(tc);
						#else
							float3 vsnormal = G_BUFFER::load_normal(tc);
						#endif

						// world
						//float3 wsnormal = mul(vsnormal, (float3x3)m_WV); // Dynamic Shaders
						float3 wsnormal = G_BUFFER::vs_ws(vsnormal);

						need_reflection = checknormals(wsnormal);

							//if(!checknormals(wsnormal, vsposition.z))
							//	return 0;

							//float3 wscompvec = float3(0.000005, 0.9999, 1000000);
							//float3 wscompnormal = wsnormal; 
							//wscompnormal.y = wscompnormal.y * wscompvec.z / vsposition.z; 
							//wscompnormal = normalize(wscompnormal);

							// check Y
							//if(!(
							//	(wscompnormal.y > wscompvec.y) 
							//&&	(wscompnormal.x - wscompvec.x < 0) 
							//&&	(wscompnormal.x + wscompvec.x > 0) 
							//&&	(wscompnormal.z - wscompvec.x < 0) 
							//&&	(wscompnormal.z + wscompvec.x > 0)
							//)) 
							//	return 0;

						if(need_reflection)
						{
							//vsnormal.y *= 1000;
							//wsnormal.y *= 100; // 
							//wsnormal.y *= 5; // it_min

							// #if (WET_DTSSR_MINMAX_FILTERING == 1)
								// bool it_max = (G_BUFFER::load_dwmbuffer(tc).y == 1);

								// if(it_max)
									// wsnormal.y *= 4;
							// #endif

							#if (WETNESS_DEBUG == 4)
								wsnormal = float3(0, 1, 0); // test
							#endif

							//vsnormal = normalize(mul((float3x3)m_V, wsnormal));
							vsnormal = normalize(G_BUFFER::ws_vs(wsnormal));
							wsnormal = normalize(wsnormal);

							//float vsnormalmixer = saturate(eye_position.y + 1);

							//vsnormal.y *= clamp(100 * vsnormalmixer, 1 , 100);
							//vsnormal.z *= clamp(100 * (1 - vsnormalmixer), 1 , 100);

							//vsnormal = normalize(mul((float3x3)m_V, vsnormal));
							//vsnormal = normalize(vsnormal);

							//vsnormal = normalize(cross(ddx(vsposition), ddy(vsposition)));

							// view with sky
							float3 vsposition_s = vsposition;
							if(vsposition_s.z <= 0.01) vsposition_s.z = DWM_SKY_CONST_DEPTH;

							float3 vsggvec = normalize(vsposition_s);

							//float3 vsup = mul((float3x3)m_V, float3(0, 1, 0)); // up
							//vsvreflect = normalize(reflect(vsggvec, vsup));
							vsvreflect = normalize(reflect(vsggvec, vsnormal));

							// norm view packed
							//if(need_scspace)
							//{
								scvreflect = vsvreflect;
								//scvreflect.xy = scvreflect.xy / InvFocalLen / float2(2, -2) + float2(0.5, 0.5) * scvreflect.zz;
								//scvreflect.xy = (scvreflect.xy + pos_decompression_params.xy * scvreflect.zz) / pos_decompression_params.zw / screen_res.xy;
								scvreflect.xy = G_BUFFER::vs_tcxdepth(vsvreflect);
							//}

							// world
							//wsvreflect = mul((float3x3)m_v2w, vsvreflect);
							//wsvreflect = mul(vsvreflect, (float3x3)m_V);
							wsvreflect = G_BUFFER::vs_ws(vsvreflect);

							// n check
							//frenel = dot(eye_direction, wsnormal);
							//float frenel_add = 0.05;
							//frenel = saturate(frenel + 1 + frenel_add);
							//frenel = 1 - dot(-vsggvec, vsnormal);
							// 
							float3 wsggvec = G_BUFFER::vs_ws(vsggvec);
							frenel = saturate(dot(wsggvec, wsvreflect));
							//frenel = saturate(dot(vsggvec, vsvreflect) * 1.5);
							//frenel = saturate(frenel);

							attenutation = saturate(1 - dot(-vsggvec, vsnormal));
						}
					}
				}

				// lighting 
				#if (SSR_PUDDLES_NEED_LIGHTING == 1)
					//float3 calc_ssr_puddles_lighting(in float3 vsposition, in float3 vsnormal, in float3 vssunvec)
					float calc_ssr_puddles_lighting(in float3 vsposition, in float3 vsnormal, in float3 vssunvec)
					{
						//float light;

						//--------------------------------
						// Sun 

							// spec
							float sunspec = plight_infinity(0.5, vsposition, vsnormal, vssunvec).w;
							return sunspec;

							//light = L_sun_color.xyz * sunspec * 0.1;

						//--------------------------------
						// Torch 

							//float3 vstorchdir = G_BUFFER::ws_vs(float3(-eye_direction.x, eye_direction.y, -eye_direction.z));

							//float3 torchspec = plight_infinity(0.5, vsposition, vsnormal, vstorchdir).www;
							//torchspec *= 0.009375 * 8;

							//float3 torchfactor = saturate(torchspec * float3(0.67, 0.67, 0.83) * (float)SSR_PUDDLES_TEST_TORCH_ENABLED * (1 - vsposition.zzz / 15));

							//light = lerp(light, saturate(torchspec), torchfactor);

						//return light
					}
				#endif

				// ssr + sky
				float4 wet_reflections_create(float2 tc, uint2 pos2d)
				{
					float3 vsposition, scposition, vsvreflect, scvreflect, wsvreflect; 

					#if (SSR_PUDDLES_NEED_LIGHTING == 1)
						float3 vsnormal;
					#endif

					float frenel, attenutation, addition_sky_m;

					bool need_reflection;

					// 
					prepare_space_for_wet_refls
					(
						tc, 
						pos2d.xy, 

						vsposition, 
						scposition, 
						vsvreflect, 
						scvreflect, 
						wsvreflect, 

						#if (SSR_PUDDLES_NEED_LIGHTING == 1)
							vsnormal,
						#endif

						frenel, 
						attenutation,
						addition_sky_m, 

						need_reflection
					);

					if(!need_reflection)
						return 0;

					#if (SSR_FAR_PLANE == 1)
						float2 tc_far = scvreflect.xy / scvreflect.zz;

						float error_far;
						float3 reflection_far;
					#endif

					// 
					float error_near;
					float3 reflection_near, reflection_sky, reflection_sun;

					#if (SSR_IMPROVED_JITTERING == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
						unsigned int need_denoise;
					#endif

					#if (SSR_RAY_RADIUS_DYNAMIC == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
						//float3 wsdir = mul(float4(vsposition, 1), m_V);
						float3 wsdir = G_BUFFER::vs_ws(vsposition);
						float dyn_height = smoothstep(0, SSR_RAY_RADIUS + 0.1, saturate(wsdir.y / SSR_RAY_RADIUS_D_HEIGHT));
					#endif

					#if (REFLECTIONS_QUALITY == 6)
						calc_reflections_new
						(
							pos2d.xy,
							scposition, 
							scvreflect, 
							wsvreflect, 
							frenel, 

							reflection_near, 
							error_near, 
							need_denoise,
							reflection_sky
						);
					#elif (SSR_ALTERNATIVE_FAST_MODE == 1)
						calc_reflections_fast
						(
							pos2d.xy,
							tc, 
							//scposition,
							//scvreflect, 
							vsposition,
							vsvreflect, 
							wsvreflect, 

							reflection_near, 
							error_near, 

							reflection_sky
						);
					#else
						calc_reflections
						(
							pos2d.xy,
							tc, 

							#if (SSR_FAR_PLANE == 1)
								tc_far, 
							#endif

							#if (SSR_RAY_RADIUS_DYNAMIC == 1)
								dyn_height, 
							#endif

							vsposition, 
							scposition,  
							vsvreflect, 
							scvreflect,  
							wsvreflect, 

							reflection_near, 
							error_near, 

							#if (SSR_FAR_PLANE == 1)
								reflection_far, 
								error_far, 
							#endif

							#if (SSR_IMPROVED_JITTERING == 1)
								need_denoise,
							#endif

							reflection_sky
						);
					#endif

					//float frenel = 1 - dot(-vsggvec, vsnormal);
					//frenel = saturate(frenel);

					error_near *= frenel;

					#if (WETNESS_DEBUG < 3)
						reflection_sky = reflection_sky * wea_sky_color.www / 1.5;
						reflection_near *= 0.5;
					#endif

					//float3 vssunvec = mul((float3x3)m_V, L_sun_dir_w);
					float3 vssunvec = G_BUFFER::ws_vs(L_sun_dir_w);

					// sun
					//reflection_sun = pow(abs(dot(normalize(vsggvec + vssunvec), vsnormal)), 192.0) * L_sun_color.xyz * 2;

					#if (SSR_FAR_PLANE == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
						reflection_sky = lerp(reflection_sky, reflection_far, error_far) * addition_sky_m.xxx;
					#endif

					// mix
					float3 reflection_final = lerp(reflection_sky/* + reflection_sun*/, reflection_near, error_near);

					#if !(SSR_PUDDLES_DEFFERED_ATTENUTATION == 1)
						//float attenutation = saturate(1 - dot(-vsggvec, vsnormal));
						reflection_final *= pow(attenutation, 0.5);
					#endif

					//float sunshadow = s_accumulator.SampleLevel(smp_nofilter, tc, 0).w;

					//float sunspec = plight_infinity(1, vsposition, vsnormal, vssunvec).w;

					//float lightcompressed = sunspec * 0.15;

					#if (SSR_PUDDLES_NEED_LIGHTING == 1)
						//float3 vssunvec = G_BUFFER::ws_vs(L_sun_dir_w.xyz);
						//reflection_final += calc_ssr_puddles_lighting(vsposition, vsnormal, vssunvec);

						float light = calc_ssr_puddles_lighting(vsposition, vsnormal, vssunvec);

						#if (SSR_IMPROVED_JITTERING == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
							float signed_light = light + rcp(255);

							if(need_denoise == 0)
								signed_light = -signed_light;

							signed_light = signed_light * 0.5 + 0.5;
						#endif

						#if (SSR_IMPROVED_JITTERING == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
							return float4(reflection_final, /*lightcompressed*//*need_denoise*/signed_light);
						#else
							return float4(reflection_final, /*lightcompressed*//*0*/light);
						#endif
					#elif (SSR_PUDDLES_DEFFERED_ATTENUTATION == 1)
						float att = sqrt(attenutation);

						#if (SSR_IMPROVED_JITTERING == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
							float signed_att = att + rcp(255);

							if(need_denoise == 0)
								signed_att = -signed_att;

							signed_att = signed_att * 0.5 + 0.5;
						#endif

						#if (SSR_IMPROVED_JITTERING == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
							return float4(reflection_final, /*lightcompressed*//*need_denoise*/signed_att);
						#else
							return float4(reflection_final, /*lightcompressed*//*0*/att);
						#endif
					#else
						#if (SSR_IMPROVED_JITTERING == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
							return float4(reflection_final, need_denoise);
						#else
							return float4(reflection_final, 0);
						#endif
					#endif
				}

				// sky
				float4 wet_reflections_create_only_sky(float2 tc, uint2 pos2d)
				{
					float3 vsposition, scposition, vsvreflect, scvreflect, wsvreflect; 

					#if (SSR_PUDDLES_NEED_LIGHTING == 1)
						float3 vsnormal;
					#endif

					float frenel, attenutation, addition_sky_m;

					bool need_reflection;

					prepare_space_for_wet_refls
					(
						tc, 
						pos2d.xy, 

						vsposition, 
						scposition, 
						vsvreflect, 
						scvreflect, 
						wsvreflect, 

						#if (SSR_PUDDLES_NEED_LIGHTING == 1)
							vsnormal,
						#endif

						frenel, 
						attenutation, 
						addition_sky_m, 

						need_reflection
					);

					if(!need_reflection)
						return 0;

					#if defined(SSR_SKYBOX_USE)
						float3 reflection_sky = sample_sky_by_wsunwrap(wsvreflect) * wea_sky_color.xyz;
					#else
						float3 reflection_sky = SSR_SKYBOX_COLOR;
					#endif

					reflection_sky *= wea_sky_color.w;

					reflection_sky *= pow(frenel, 0.5);

					#if (SSR_PUDDLES_NEED_LIGHTING == 1)
						float3 vssunvec = G_BUFFER::ws_vs(L_sun_dir_w.xyz);
						//reflection_sky += calc_ssr_puddles_lighting(vsposition, vsnormal, vssunvec);
						float light = calc_ssr_puddles_lighting(vsposition, vsnormal, vssunvec);
						return float4(reflection_sky, /*0*/light);
					#elif (SSR_PUDDLES_DEFFERED_ATTENUTATION == 1)
						return float4(reflection_sky, sqrt(attenutation));
					#else
						return float4(reflection_sky, 0);
					#endif
				}

				// far plane + sky
				float4 wet_reflections_create_farplane_sky(float2 tc, uint2 pos2d)
				{
					float3 vsposition, scposition, vsvreflect, scvreflect, wsvreflect; 

					#if (SSR_PUDDLES_NEED_LIGHTING == 1)
						float3 vsnormal;
					#endif

					float frenel, attenutation, addition_sky_m;

					bool need_reflection;

					prepare_space_for_wet_refls
					(
						tc, 
						pos2d.xy, 

						vsposition, 
						scposition, 
						vsvreflect, 
						scvreflect, 
						wsvreflect, 

						#if (SSR_PUDDLES_NEED_LIGHTING == 1)
							vsnormal,
						#endif

						frenel, 
						attenutation, 
						addition_sky_m,

						need_reflection
					);

					if(!need_reflection)
						return 0;

					#if defined(SSR_SKYBOX_USE)
						float3 reflection_sky = sample_sky_by_wsunwrap(wsvreflect) * wea_sky_color.xyz;
					#else
						float3 reflection_sky = SSR_SKYBOX_COLOR;
					#endif

					reflection_sky *= wea_sky_color.w;

					//float2 tc_far = G_BUFFER::vs_tc(vsvreflect);
					float2 tc_far = scvreflect.xy / scvreflect.zz;

					float depth_far = G_BUFFER::load_depth(tc_far);

					float error_far = 1;

					if(vsposition.z > depth_far)
						error_far = 0;

					float depth_far_scaled = saturate(depth_far / SSR_FAR_DISTANCE);

					error_far *= depth_far_scaled * depth_far_scaled * depth_far_scaled;

					float3 reflection_far = 0;

					if(error_far > 0) 
						reflection_far = reflections_sample(tc_far, error_far, false);

					error_far *= calc_vignette(tc_far, SSR_OUTRANGE_TC_CUTER);

					reflection_sky = lerp(reflection_sky, reflection_far, error_far) * addition_sky_m.xxx;

					reflection_sky *= pow(frenel, 0.5);

					#if (SSR_PUDDLES_NEED_LIGHTING == 1)
						float3 vssunvec = G_BUFFER::ws_vs(L_sun_dir_w.xyz);
						//reflection_sky += calc_ssr_puddles_lighting(vsposition, vsnormal, vssunvec);
						float light = calc_ssr_puddles_lighting(vsposition, vsnormal, vssunvec);
						return float4(reflection_sky, /*0*/light);
					#elif (SSR_PUDDLES_DEFFERED_ATTENUTATION == 1)
						return float4(reflection_sky, sqrt(attenutation));
					#else
						return float4(reflection_sky, 0);
					#endif
				}

				// rendered denoise
				#if defined(SSR_ROAD_RENDERED)
					#if defined(SSR_ROAD_RENDERED_AND_DENOISED)
						float4 wet_reflections_filtering_and_mix(float2 tc, float4 img)
						{
							float depth = G_BUFFER::load_depth(tc);

							#if (WETNESS_DEBUG < 1)
								// img *= 1 - (1 - smooth_out_depth(depth, 30, 3) * saturate(depth / 150)) * rain_updater.x / 4;
								// img *= 1 - (1 - pow(smoothstep(0, 30, depth), 3) * saturate(depth / 150)) * rain_updater.x / 4;
								img *= 1 - (1 - pow(smoothstep(0, 30, depth), 3) * saturate(depth / 150)) * 1 / 4;
							#endif

							if(depth > 75)
								return img;

							#if (REFLECTIONS_QUALITY > 0)
								#ifdef WET_DTSSR_NORMAL_FILTERING
									//float2 bias = 0;

									// float wet = s_wetmap.SampleLevel(smp_nofilter, tc, 0).x;
									float wet = 1;

									// #if defined(ENGINE_RENDER_DWMBUFFER)
										// wet = wet / 0.9 - 0.1;
									// #endif

									if(depth > 30)
										wet = 1;

									float2 tc_r = tc;

									//#if (WET_DTSSR_MINMAX_FILTERING == 1)
									//	float puddle_mask = G_BUFFER::load_dwmbuffer(tc).y; // puddles
									//	if(puddle_mask != 1)
									//#else
									//	if(true)
									//#endif
									//{
										//bias = WET_DTSSR_NORMAL_OFFSET * wet - saturate(pow(mul(m_v2w, float4(G_BUFFER::load_normal(tc), 0)).y, 4)) * WET_DTSSR_NORMAL_OFFSET * wet;
									//	float3 _vsnormal = G_BUFFER::load_normal(tc);
									//	float3 _vsposition = G_BUFFER::unpack_position(tc, depth);
									//	float3 _vsposition_n = _vsposition + _vsnormal * developer_float4.x;
									//	tc_r = G_BUFFER::vs_tc(_vsposition_n);
									//}

									//#if (WET_DTSSR_MINMAX_FILTERING == 1)
									//	if(puddle_mask == 1)
									//	{
									//		_vsposition_n = _vsposition + _vsnormal * developer_float4.x * developer_float4.y;
									//		tc_r = G_BUFFER::vs_tc(_vsposition_n);
									//	}
									//#endif

									//float2 tc_r = saturate(tc + bias);
								#else
									float2 tc_r = tc;
								#endif

								#if (WETNESS_DEBUG == 4)
									tc_r = tc;
								#endif

								float4 reflection = s_dtssr_denoised.SampleLevel(smp_nofilter, tc_r, 0);

								#if (SSR_ALTERNATIVE_FAST_MODE == 0) && (REFLECTIONS_QUALITY > 3)
									if(reflection.z == 0)
										return img;

									#if (SSR_IMPROVED_JITTERING == 1)
										#if ((SSR_PUDDLES_NEED_LIGHTING == 1) || (SSR_PUDDLES_DEFFERED_ATTENUTATION == 1)) && !(REFLECTIONS_QUALITY == 6)
											unsigned int need_denoise = (reflection.w * 2 - 1 > 0);
										#else
											unsigned int need_denoise = (reflection.w);
										#endif

										float4 reflection_not_denoised = s_dtssr.SampleLevel(smp_nofilter, tc, 0);

										//  noise check
										if(!need_denoise) 
											reflection = reflection_not_denoised;

										#if (SSR_IMPROVED_JITTERING_DEBUG == 1)
											if(need_denoise) 
												return 1;
										#endif

										#if (SSR_PUDDLES_NEED_LIGHTING == 1) || (SSR_PUDDLES_DEFFERED_ATTENUTATION == 1)
											reflection.w = abs(reflection_not_denoised.w * 2 - (1 + rcp(255)));
										#endif
									#endif
								#endif
							#else
								float4 reflection = float4(SSR_SKYBOX_COLOR, 1);//float4(0.2, 0.36, 0.53, 0);//wea_sky_color.xyz * wea_sky_color.www;
							#endif

							if(reflection.x == 0)
								return img;

							#if !defined(WET_DTSSR_NORMAL_FILTERING) || (REFLECTIONS_QUALITY == 0)
								// float wet = s_wetmap.SampleLevel(smp_nofilter, tc, 0).x;
								float wet = 1;

								// #if defined(ENGINE_RENDER_DWMBUFFER)
									// wet = wet / 0.9 - 0.1;
								// #endif

								if(depth > 30)
									wet = 1;

								if(!wet)
									return img;
							#endif

							//#if !(WET_DTSSR_MINMAX_FILTERING==1) || (REFLECTIONS_QUALITY == 0)
								// float puddle_mask = G_BUFFER::load_dwmbuffer(tc).y; // puddles
							//#endif

							// #if (WETNESS_DEBUG == 4)
								puddle_mask = 1; 
							// #endif

							if(puddle_mask == 0)
								return img;

							// float wet_factor = wet * puddle_mask * rain_updater.x;
							float wet_factor = wet * puddle_mask * 1;

							// float att = smooth_out_depth(depth, 75, 3);
							float att = pow(smoothstep(0, 75, depth), 3);

							#if (REFLECTIONS_QUALITY > 0)
								// #if (SSR_PUDDLES_DEFFERED_ATTENUTATION == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0) && (REFLECTIONS_QUALITY > 3)
									float reflectance = wet_factor * att * reflection.w * 0.5;
								// #else
									// float luma_refl = clamp(dot(reflection.xyz, LUMINANCE_VECTOR) * 6, 0.3, 1) * 0.5;
									// float reflectance = wet_factor * att * luma_refl;
								// #endif
							#else
								float reflectance = reflection.x * wet_factor * att / 3;
							#endif

							#if (SSR_PUDDLES_NEED_LIGHTING == 1)
								float3 light = reflection.www * L_sun_color.xyz;

								reflection.xyz = saturate(reflection.xyz + light);

								if(light.x + light.y + light.z > img.x + img.y + img.z)
									reflectance = saturate(reflectance + reflection.w * wet_factor);
							#endif

							#if (WETNESS_DEBUG>=3)
								return reflection;
							#endif

							//#if (REFLECTIONS_QUALITY == 5)	
							//	// Check wet planes
							//	//reflectance *= isuplcomp.xxx;
							//	img.xyz += L_sun_color.xyz * isuplcomp.yyy * reflectance.xxx * 3;
							//#endif

							img.xyz = lerp(img.xyz, reflection.xyz, reflectance);

							return img;
						}
					//#else
					#elif (REFLECTIONS_QUALITY > 3)
						float4 wet_reflections_denoise(float2 tc, unsigned int STAGEID)
						{
							#if (REFLECTIONS_QUALITY == 6) && (SSR_IMPROVED_JITTERING == 1)
								float4 test_sample;
								float4 reflection_accum = 0;
								float2 bias = screen_res.zw * int2(STAGEID == XSTAGE, STAGEID == YSTAGE);
								int contrib = 0;

								for(int biasmul = -2; biasmul < 2; biasmul++)
								{
									float2 new_tc = tc + bias * biasmul;

									if(is_in_quad(new_tc))
									{
										test_sample = s_dtssr.SampleLevel(smp_nofilter, new_tc, 0);

										if(test_sample.x != 0 && test_sample.y != 0 && test_sample.z != 0)
										{
											reflection_accum.xyz += test_sample.xyz;

											unsigned int denoise_map = (test_sample.w * 2 - 1 > 0);

											reflection_accum.w += denoise_map; 
											
											contrib++;
										}
									}
								}

								// check noise map
								if(reflection_accum.w > 0)
									reflection_accum.w = 1;

								if(contrib)
									reflection_accum.xyz /= contrib;

								return reflection_accum;
							#else
								float4 reflection;
								float4 reflection_accum = 0;

								float2 bias = screen_res.zw * int2(STAGEID == XSTAGE, STAGEID == YSTAGE);

								for(int biasmul = -2; biasmul < 2; biasmul++)
								{
									float2 new_tc = tc + bias * biasmul;
									reflection_accum += s_dtssr.SampleLevel(smp_nofilter, new_tc, 0);
								}

								reflection = reflection_accum / 4;

								return reflection;
							#endif
						}
					#endif
				#endif
			#else

				//static const float2 frame_id_decoder[16] = 
				//{
					//-2, -2,
					//-1, -1,
					// 1, -0,
					// 0,  1,
					//-1, -2,
					//-2, -1,
					// 0, -0,
					// 1,  1,
					// 0, -2,
					// 1, -1,
					//-1, -0,
					//-2,  1,
					// 1, -2,
					// 0, -1,
					//-2, -0,
					//-1,  1,
				//};

				void prepare_space_for_water_refls
				(
					in float3 wsnormal, 
					in float3 ws2point, 
					in float3 vsposition, 

					inout float2 tc, 

					out float3 ws2point_n, 
					out float3 wsvreflect,

					out float3 vsvreflect,
					out float3 scvreflect,
					out float3 scposition,

					out float frenel
				)
				{
					scposition = float3(tc, 1) * vsposition.zzz;

					ws2point_n = normalize(ws2point);

					wsvreflect = normalize(reflect(ws2point_n, wsnormal));
					//vsvreflect = mul((float3x3)m_V, wsvreflect);
					vsvreflect = G_BUFFER::ws_vs(wsvreflect);
					scvreflect = vsvreflect;
					//scvreflect.xy = scvreflect.xy / InvFocalLen / float2(2, -2) + float2(0.5, 0.5) * scvreflect.zz;
					scvreflect.xy = G_BUFFER::vs_tcxdepth(vsvreflect);

					frenel = dot(ws2point_n, wsvreflect);
					frenel = saturate(frenel);
					//frenel = dot(eye_direction, wsnormal);
					//frenel = saturate(frenel + 1);
				}

				// ssr + sky
				void water_reflections
				(
					in float3 wsposition, 
					in float3 wsnormal, 
					in float3 ws2point, 
					in float3 vsposition, 
					in float2 tc, 

					out float3 ws2point_n, 
					out float3 wsvreflect,

					out float3 ref_objects,
					out float3 ref_sky
				)
				{
					float3 vsvreflect, scvreflect, scposition;
					float frenel;

					prepare_space_for_water_refls(wsnormal, ws2point, vsposition, tc, ws2point_n, wsvreflect, vsvreflect, scvreflect, scposition, frenel);

					#if (SSR_FAR_PLANE == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
						float2 tc_far = scvreflect.xy / scvreflect.zz;

						float error_far;
						float3 reflection_far;
					#endif

					float error_near;
					float3 reflection_near, reflection_sky, reflection_sun;

					#if (SSR_RAY_RADIUS_DYNAMIC == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
						float dyn_height = smoothstep(0, SSR_RAY_RADIUS + 0.1, saturate(ws2point.y / SSR_RAY_RADIUS_D_HEIGHT));
					#endif

					uint2 pos2d = G_BUFFER::tc2p2d(tc);

					#if (REFLECTIONS_QUALITY == 6)
						calc_reflections_new
						(
							pos2d.xy,
							scposition, 
							scvreflect, 
							wsvreflect, 
							frenel, 

							reflection_near, 
							error_near, 
							reflection_sky
						);
					#elif (SSR_ALTERNATIVE_FAST_MODE == 1)
						calc_reflections_fast
						(
							pos2d.xy,
							tc, 
							//scposition,
							//scvreflect, 
							vsposition,
							vsvreflect, 
							wsvreflect, 

							reflection_near, 
							error_near, 

							reflection_sky
						);
					#else
						calc_reflections
						(
							pos2d.xy,
							tc, 

							#if (SSR_FAR_PLANE == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
								tc_far, 
							#endif

							#if (SSR_RAY_RADIUS_DYNAMIC == 1)
								dyn_height,
							#endif

							vsposition, 
							scposition,  
							vsvreflect, 
							scvreflect,  
							wsvreflect, 

							reflection_near, 
							error_near, 

							#if (SSR_FAR_PLANE == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
								reflection_far, 
								error_far, 
							#endif

							reflection_sky
						);
					#endif

					reflection_sky *= wea_sky_color.w;

					//float frenel = dot(eye_direction, wsnormal);
					//frenel = saturate(frenel + 1);

					error_near *= frenel;

					// sun LVutner
					//reflection_sun = pow(abs(dot(normalize(ws2point_n + L_sun_dir_w), wsnormal)), 192.0) * L_sun_color.xyz * 0.75;

					#if (SSR_FAR_PLANE == 1) && (SSR_ALTERNATIVE_FAST_MODE == 0)
						reflection_sky = lerp(reflection_sky, reflection_far, error_far);
					#endif

					ref_objects = reflection_near * error_near;
					ref_sky = (reflection_sky/* + reflection_sun*/) * (1 - error_near);
				}

				// far plane + sky
				void water_reflections_farplane_sky
				(
					in float3 wsposition, 
					in float3 wsnormal, 
					in float3 ws2point, 
					in float3 vsposition, 
					in float2 tc, 

					out float3 ws2point_n, 
					out float3 wsvreflect,
					out float3 ref_objects,
					out float3 ref_sky
				)
				{
					float3 vsvreflect, scvreflect, scposition;
					float frenel;

					prepare_space_for_water_refls(wsnormal, ws2point, vsposition, tc, ws2point_n, wsvreflect, vsvreflect, scvreflect, scposition, frenel);

					#if defined(SSR_SKYBOX_USE)
						float3 reflection_sky = sample_sky_by_wsunwrap(wsvreflect) * wea_sky_color.xyz;
					#else
						float3 reflection_sky = SSR_SKYBOX_COLOR;
					#endif

					reflection_sky *= wea_sky_color.w;

					//float2 tc_far = G_BUFFER::vs_tc(vsvreflect);
					float2 tc_far = scvreflect.xy / scvreflect.zz;

					float depth_far = G_BUFFER::load_depth(tc_far);

					float error_far = 1;

					if(vsposition.z > depth_far)
						error_far = 0;

					float depth_far_scaled = saturate(depth_far / SSR_FAR_DISTANCE);

					error_far *= depth_far_scaled * depth_far_scaled * depth_far_scaled;

					float3 reflection_far = 0;

					if(error_far > 0) 
						reflection_far = reflections_sample(tc_far, error_far, false);

					error_far *= calc_vignette(tc_far, SSR_OUTRANGE_TC_CUTER);

					reflection_sky = lerp(reflection_sky, reflection_far, error_far);

					//reflection_sky *= pow(frenel, 0.5);

					ref_objects = 0;
					ref_sky = reflection_sky;
				}

				// Planar SSR

				#define PLANAR_SSR_FULL_CS

				uint2 planar_ssr_make_reflection(in uint2 pos2d, in float reflector_h)
				{
					uint2 pos2d_hit = 0;

					float2 tc = (float2)pos2d.xy * screen_res.zw;

					float3 vpos = G_BUFFER::load_position_wsky(tc, pos2d.xy).xyz;
					float3 wpos = G_BUFFER::vs_ws(vpos, 1);

					if(wpos.y < reflector_h)
						return pos2d_hit;

					float3 refpos = wpos;

					// Y plane reflection
					refpos.y = -(refpos.y - 2 * reflector_h);

					float2 tc_new = G_BUFFER::ws_tc(refpos, 1);

					// Apply TAA
					// float4 hpos = float4(tc_new, 0, 0.5);
					// update_taa_vertex(hpos);
					// tc_new = hpos.xy;

					pos2d_hit = tc_new * screen_res.xy + 0.5;

					return pos2d_hit;
				}

				// shit..
				#ifdef MSAA_SAMPLES
					uniform Texture2DMS<float, MSAA_SAMPLES> s_depth1;

					float planar_ssr_detect_water()
					{
						float down = G_BUFFER::load(s_depth1, screen_res.xy / 2 + int2(0, screen_res.y / 4)); // Down
						if(down) return down;
						float center = G_BUFFER::load(s_depth1, screen_res.xy / 2); // Center
						if(center) return center;
						return 0;
					}
				#else
					uniform Texture2D<float> s_depth1;

					float planar_ssr_detect_water()
					{
						float down = s_depth1.SampleLevel(smp_rtlinear, float2(0.75, 0.50) + screen_res.zw * 0.5, 0); // Down
						if(down) return down;
						float center = s_depth1.SampleLevel(smp_rtlinear, float2(0.50, 0.50) + screen_res.zw * 0.5, 0); // Center
						if(center) return center;
						return 0;
					}
				#endif

				uniform Texture2D<uint> s_sspr_hash; // 

				// hit..
				void planar_ssr_hit_pos(in uint2 pos2d, out uint2 pos2d_hit, out float2 tc_hit)
				{
					uint pos2d_sspr_packed = s_sspr_hash.Load(uint3(pos2d.xy, 0));
					pos2d_hit = uint2(pos2d_sspr_packed & 0xFFFF, pos2d_sspr_packed >> 16);
					tc_hit = pos2d_hit * screen_res.zw;
				}

				uint is_valid_hit(float depth_hit_0, float depth_hit)
				{
					return !is_in_range(depth_hit, -0.01, 0.01) && 
							depth_hit != G_BUFFER::sky && 
							depth_hit_0 != depth_hit;
				}

				float4 planar_ssr_obj_reflections(in uint2 pos2d, in float2 tc)
				{
					uint2 pos2d_hit; float2 tc_hit;
					planar_ssr_hit_pos(pos2d, pos2d_hit, tc_hit);

					float depth_hit_0 = G_BUFFER::load_depth(float2(0, 0));
					float depth_hit = G_BUFFER::load_depth(tc_hit);

					uint stencil = is_valid_hit(depth_hit_0, depth_hit);

					static const int2 offsets[4] =
					{
						 0,  1,
						 0, -1,
						 1,  0,
						-1,  0,
					};

					// sort hit position by near if it is valid
					for(int i = 0; i < 4 && stencil; i++)
					{
						uint2 pos2d_hit_test; float2 tc_hit_test;
						planar_ssr_hit_pos(pos2d + offsets[i], pos2d_hit_test, tc_hit_test);

						float depth_hit_test = G_BUFFER::load_depth(tc_hit_test);

						stencil = is_valid_hit(depth_hit_0, depth_hit_test);

						if(stencil && depth_hit_test < depth_hit)
						{
							depth_hit = depth_hit_test;
							pos2d_hit = pos2d_hit_test;
							tc_hit = tc_hit_test;
						}
					}

					// fill neighbourhood if it is invalid
					for(int i = 0; i < 4 && !stencil; i++)
					{
						planar_ssr_hit_pos(pos2d + offsets[i], pos2d_hit, tc_hit);
						depth_hit = G_BUFFER::load_depth(tc_hit);
						stencil = is_valid_hit(depth_hit_0, depth_hit);
						if(!stencil) continue;
					}

					float3 objects = s_image.Sample(smp_rtlinear, tc_hit);
					float3 position_hit = G_BUFFER::unpack_position(tc_hit, pos2d_hit, depth_hit);
					float vignette = calc_vignette(tc_hit, SSR_OUTRANGE_TC_CUTER, true);

					float mask = vignette * stencil;

					float fog = saturate(length(position_hit) * fog_params.w + fog_params.x);
					float skyblend = fog * fog;

					objects = lerp(objects, fog_color.xyz, fog);
					mask *= 1 - skyblend;

					return float4(objects, mask);
				}

				void water_reflections_planar_ssr
				(
					in float3 wsposition, 
					in float3 wsnormal, 
					in float3 ws2point, 
					in float3 vsposition, 
					in float2 tc, 
					in float  normal_scale,

					out float3 ws2point_n, 
					out float3 wsvreflect,
					out float3 ref_objects,
					out float3 ref_sky
				)
				{
					float3 vsvreflect, scvreflect, scposition;
					float frenel;

					#if WATER_DISABLE_NORMAL != 1
						float3 wsnormal_bias = wsnormal - float3(0, 1, 0);
						float3 wsposition_w_nbias = wsposition + wsnormal_bias;
						float2 tc_w_bias = G_BUFFER::ws_tc(wsposition_w_nbias, 1);
						float2 tc_bias = tc_w_bias - tc;
						tc += tc_bias * sqrt(vsposition.z) * normal_scale;
					#endif

					prepare_space_for_water_refls(wsnormal, ws2point, vsposition, tc, ws2point_n, wsvreflect, vsvreflect, scvreflect, scposition, frenel);

					#if defined(SSR_SKYBOX_USE)
						float3 reflection_sky = sample_sky_by_wsunwrap(wsvreflect) * wea_sky_color.xyz;
					#else
						float3 reflection_sky = SSR_SKYBOX_COLOR;
					#endif

					reflection_sky *= wea_sky_color.w;

					float mask;

					float4(ref_objects, mask) = planar_ssr_obj_reflections(tc * screen_res.xy, tc);

					ref_objects *= mask;
					ref_sky = reflection_sky * (1 - mask);
				}

				// Planar 
				#ifdef USE_MSAA_REFLECTIONS_WITHOUT_RESOLVE
					uniform Texture2DMS<float4, MSAA_SAMPLES> s_planar_color_ms;
				#else
					uniform Texture2D<float3> s_planar_color;
				#endif

				void water_reflections_planar
				(
					in float3 wsposition, 
					in float3 wsnormal, 
					in float3 ws2point, 
					in float3 vsposition, 
					in float2 tc, 
					in float  normal_scale,

					out float3 ws2point_n, 
					out float3 wsvreflect,
					out float3 ref
				)
				{
					float3 vsvreflect, scvreflect, scposition;
					float frenel;

					#if WATER_DISABLE_NORMAL != 1
						float3 wsnormal_bias = wsnormal - float3(0, 1, 0);
						float3 wsposition_w_nbias = wsposition + wsnormal_bias;
						float2 tc_w_bias = G_BUFFER::ws_tc(wsposition_w_nbias, 1);
						float2 tc_bias = tc_w_bias - tc;
					#else
						float2 tc_bias = 0;
					#endif

					prepare_space_for_water_refls(wsnormal, ws2point, vsposition, tc, ws2point_n, wsvreflect, vsvreflect, scvreflect, scposition, frenel);

					float2 tc_hit = tc;
					tc_hit.x = 1 - tc_hit.x;

					tc_hit += tc_bias * sqrt(vsposition.z) * normal_scale;

					#ifdef USE_MSAA_REFLECTIONS_WITHOUT_RESOLVE
						ref = G_BUFFER::load(s_planar_color_ms, tc_hit * screen_res.xy).xyz;
					#else
						ref = s_planar_color.Sample(smp_rtlinear, tc_hit);
					#endif
				}

				void water_reflections_planar_s
				(
					in float3 wsposition, 
					in float3 wsnormal, 
					in float3 ws2point, 
					in float3 vsposition, 
					in float2 tc, 
					in float  normal_scale,
					in float  threshold_h,

					out float3 ws2point_n, 
					out float3 wsvreflect,
					out float3 ref,
					out uint succed
				)
				{
					float3 vsvreflect, scvreflect, scposition;
					float frenel;

					prepare_space_for_water_refls(wsnormal, ws2point, vsposition, tc, ws2point_n, wsvreflect, vsvreflect, scvreflect, scposition, frenel);

					if(abs(wsposition.y - depth_params.w) < threshold_h)
					{
						#if WATER_DISABLE_NORMAL != 1
							float3 wsnormal_bias = wsnormal - float3(0, 1, 0);
							float3 wsposition_w_nbias = wsposition + wsnormal_bias;
							float2 tc_w_bias = G_BUFFER::ws_tc(wsposition_w_nbias, 1);
							float2 tc_bias = tc_w_bias - tc;
						#else
							float2 tc_bias = 0;
						#endif

						float2 tc_hit = tc;
						tc_hit.x = 1 - tc_hit.x;
						tc_hit += tc_bias * sqrt(vsposition.z) * normal_scale;

						#ifdef USE_MSAA_REFLECTIONS_WITHOUT_RESOLVE
							ref = G_BUFFER::load(s_planar_color_ms, tc_hit * screen_res.xy).xyz;
						#else
							ref = s_planar_color.Sample(smp_rtlinear, tc_hit);
						#endif

						succed = 1;
					}
					else
					{
						#if defined(SSR_SKYBOX_USE)
							ref = sample_sky_by_wsunwrap(wsvreflect) * wea_sky_color.xyz;
						#else
							ref = SSR_SKYBOX_COLOR;
						#endif

						succed = 0;
					}
				}

			#endif

	#if defined(CLEAR_SSR_DEFINES)
		#undef TCXDEPTH_SPACE
		#undef VIEW_SPACE
		#undef XSTAGE
		#undef YSTAGE
		#undef DEFFERED_ATTENUTATION
		#undef LIGHTING
	#endif
#endif