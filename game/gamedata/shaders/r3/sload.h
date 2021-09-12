#ifndef SLOAD_H
#define SLOAD_H
	#include "common.h"

	namespace SLoad
	{
		static int mip = -1; // only for s_base

		struct SURFACE
		{
			float4	base;
			float3	normal;
			float	gloss;
		};

		void force_mip(int new_mip)
		{
			mip = new_mip;
		}

		float4 sample_base(float2 tc)
		{
			if(mip < 0)
				return s_base.Sample(smp_base, tc);
			else
				return s_base.SampleLevel(smp_base, tc, mip);
		}

		void perform_tc_offset(inout v2p_bumped		p);
		void perform_tc_offset(inout v2p_bumped		p, in Texture2D s_bumpX_new);

		// base & det
		SURFACE fill(v2p_flat		p);
		SURFACE fill(v2p_bumped		p);

		// det & mask
	#ifdef USE_TDETAIL
		SURFACE fill(v2p_bumped		p,	Texture2D s_base_det, Texture2D s_bump_det, Texture2D s_bumpX_det, 	uint need_mask = 0, float mask = 1);
		SURFACE fill(v2p_flat		p,	Texture2D s_base_det,												uint need_mask = 0, float mask = 1);
	#endif

		SURFACE fill(v2p_flat p)
		{
			SURFACE S;

			S.base = sample_base(p.tcdh.xy);

			S.normal = normalize(p.N.xyz); // it Ne
			S.gloss	= def_gloss;

		#if defined(USE_TDETAIL)
			float3 detail = s_detail.Sample(smp_base, p.tcdbump).xyz;
			S.base.xyz = S.base.xyz * detail.xyz * 2;
		#endif

			return S;
		}

		SURFACE fill(v2p_bumped p)
		{
			SURFACE S;

		#if defined(USE_PARALLAX) || defined(USE_STEEPPARALLAX)
			perform_tc_offset(p);
		#endif

			S.base = sample_base(p.tcdh.xy);

			float4 Nu  = s_bump.Sample(smp_base,  p.tcdh.xy);
			float4 NuE = s_bumpX.Sample(smp_base, p.tcdh.xy);

			S.normal = Nu.wzy + (NuE.xyz - 1);
			S.gloss	= Nu.x * Nu.x;

		#if defined(USE_TDETAIL)
			float4 detail = s_detail.Sample(smp_base, p.tcdbump);

			#if defined(USE_TDETAIL_BUMP)
				float4 NDetail	= s_detailBump.Sample(smp_base, p.tcdbump);
				float4 NDetailX	= s_detailBumpX.Sample(smp_base, p.tcdbump);

				S.gloss   = S.gloss * NDetail.x * 2;
				S.normal += NDetail.wzy + NDetailX.xyz - 1;
			#else
				S.gloss = S.gloss * detail.w * 2;
			#endif

			S.base.xyz = S.base.xyz * detail.xyz * 2;
		#endif

			return S;
		}

	#ifdef USE_TDETAIL
		SURFACE fill(v2p_bumped p, Texture2D s_base_det, Texture2D s_bump_det, Texture2D s_bumpX_det, uint need_mask, float mask)
		{
			SURFACE S;

			mask = need_mask ? mask : 1;

			if(mask <= 0)
			{
				S.base = 0; 
				S.gloss = 0; 
				S.normal = 0;
				return S;
			}

		#if defined(DEFFER_IBM_MODE)
			perform_tc_offset(p, s_bumpX_det);
		#endif

			S.base = s_base_det.Sample(smp_base, p.tcdbump) * mask;

			float4 Nu = s_bump_det.Sample(smp_base, p.tcdbump);

			S.gloss = Nu.x * Nu.x * mask;
			S.normal = (Nu.wzy - 0.5) * mask;

			return S;
		}

		SURFACE fill(v2p_flat p, Texture2D s_base_det, uint need_mask, float mask)
		{
			SURFACE S;

			mask = need_mask ? mask : 1;

			if(mask <= 0)
			{
				S.base = 0; 
				S.gloss = 0; 
				S.normal = 0;
				return S;
			}

			S.base = s_base_det.Sample(smp_base, p.tcdh.xy * dt_params) * mask;

			S.normal = normalize(p.N.xyz); // it Ne
			S.gloss	= def_gloss;

			return S;
		}
	#endif

		#define PARALLAX_NEAR_PLANE 0.01
		#define PARALLAX_FAR_PLANE 40
		#define PARALLAX_DEPTH 0.013

		void perform_tc_offset(inout v2p_bumped p)
		{
			perform_tc_offset(p, s_bumpX);
		}

		void perform_tc_offset(inout v2p_bumped p, in Texture2D s_bumpX_new)
		{
			if ((p.position.z > PARALLAX_NEAR_PLANE) && (p.position.z < PARALLAX_FAR_PLANE))
			{
				float3 eye = normalize(mul(float3x3(p.M1.x, p.M2.x, p.M3.x, 
													p.M1.y, p.M2.y, p.M3.y,
													p.M1.z, p.M2.z, p.M3.z),	-p.position.xyz));

			#if (DEFFER_IBM_MODE == 1)
				#if defined(DEFFER_TERRAIN)
					p.tcdbump += eye * PARALLAX_DEPTH * (s_bumpX_new.Sample(smp_base, p.tcdbump).ww - 0.5);
				#else
					p.tcdh.xy += eye * PARALLAX_DEPTH * (s_bumpX_new.Sample(smp_base, p.tcdh.xy).ww - 0.5);
				#endif
			#elif (DEFFER_IBM_MODE == 2) || (DEFFER_IBM_MODE == 3)
				// steps minmax and refines minmax
				int4 steps = int4(3, 10, 4, 6); // 3..10, 7..16

				bool need_disp_lerp = true;

			#if (DEFFER_IBM_MODE == 2)
				bool need_refine = false;
			#elif (DEFFER_IBM_MODE == 3)
				bool need_refine = true;
			#endif

				float view_angle = abs(dot(float3(0.0, 0.0, 1.0), eye));

				float layer_step = rcp(lerp(steps.y, steps.x, view_angle));

				float2 tc_step = layer_step * eye.xy * PARALLAX_DEPTH;

			#if defined(DEFFER_TERRAIN)
				float2 displaced_tc = p.tcdbump;
			#else
				float2 displaced_tc = p.tcdh.xy;
			#endif

				float curr_disp, curr_layer = 0;

				do
				{
					displaced_tc -= tc_step;
					curr_disp = 1 - s_bumpX_new.SampleLevel(smp_base, displaced_tc, 0).w;
					curr_layer += layer_step;
				}
				while(curr_layer < curr_disp);

				if(need_refine)
				{
					displaced_tc += tc_step;
					curr_layer -= layer_step;

					float refine_steps = lerp(steps.w, steps.z, view_angle);

					tc_step /= refine_steps;
					layer_step /= refine_steps;

					do
					{
						displaced_tc -= tc_step;
						curr_disp = 1 - s_bumpX_new.SampleLevel(smp_base, displaced_tc, 0).w;
						curr_layer += layer_step;
					}
					while(curr_layer < curr_disp);
				}

				if(need_disp_lerp)
				{
					float2 displaced_tc_prev = displaced_tc + tc_step;

					float after_depth  = curr_disp - curr_layer;
					float before_depth = 1 - s_bumpX_new.SampleLevel(smp_base, displaced_tc_prev, 0).w - curr_layer + layer_step;

					float weight = after_depth / (after_depth - before_depth);

					displaced_tc = lerp(displaced_tc, displaced_tc_prev, weight);
				}

			#if defined(DEFFER_TERRAIN)
				p.tcdbump = displaced_tc;
			#else
				p.tcdh.xy = displaced_tc;

				#if defined(USE_TDETAIL)
					p.tcdbump = p.tcdh.xy * dt_params; // Recalc detail tex uv
				#endif
			#endif
			#endif
			}
		}
	}
#endif
