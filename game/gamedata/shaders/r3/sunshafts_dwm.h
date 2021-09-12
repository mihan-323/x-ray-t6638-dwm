#if !defined(DWM_SCREEN_SPACE_SUNSHAFTS_INCLUDED)
#define DWM_SCREEN_SPACE_SUNSHAFTS_INCLUDED
	/*
		Screen Space Sun Shafts
		Автор: DWM Team
	*/

	#include "common.h"

	#define SS_SUNSHAFTS_SAMPLES 15//10
	#define SS_SUNSHAFTS_HQ_LENGTH 5
	#define SS_SUNSHAFTS_FAR_PLANE 250
	#define SS_SUNSHAFTS_SUN_DIST 6400

	float3 get_vssundir();
	float perform_sunshafts(in float2 tc, in float2 tcxres);

	float perform_sunshafts(in float2 tc, in float2 pos2d)
	{
		float accum = 0;

		float s_direction = dot(eye_direction, normalize(L_sun_dir_w));

		if(s_direction <= 0)
		{
			float3 vsposition = G_BUFFER::load_position(tc);

			#if (DX11_STATIC_DEFFERED_RENDERER == 1)
				float3 direction = G_BUFFER::ws_vs(L_sun_dir_w.xyz); // Wrong, fix me
			#else
				float3 direction = Ldynamic_dir.xyz;
			#endif

			float3 vsposition_next = vsposition + direction * SS_SUNSHAFTS_SUN_DIST;

			float2 tc_next = G_BUFFER::vs_tc(vsposition_next);

			float2 step = tc_next - tc;

			float jitter = noise::get_4(pos2d, 1, 1);

			step = step * jitter / SS_SUNSHAFTS_SAMPLES;

			float2 tc_cycle = tc;

			for(int i = 0; i < SS_SUNSHAFTS_SAMPLES; i++)
			{
				tc_cycle += step;

				if(is_in_quad(tc_cycle))
				{
					float depth_new = G_BUFFER::load_depth(tc_cycle);
					accum += step(depth_new, 0.01) || step(SS_SUNSHAFTS_FAR_PLANE, depth_new);
				}
			}

			accum /= (float)SS_SUNSHAFTS_SAMPLES;
		}

		return accum;
	}

	float3 get_vssundir()
	{
		#if (DX11_STATIC_DEFFERED_RENDERER == 1)
			return G_BUFFER::ws_vs(L_sun_dir_w.xyz); // Wrong, fix me
		#else
			return Ldynamic_dir.xyz;
		#endif
	}
#endif