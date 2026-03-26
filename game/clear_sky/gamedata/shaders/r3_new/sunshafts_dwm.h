#if !defined(DWM_SCREEN_SPACE_SUNSHAFTS_INCLUDED)
#define DWM_SCREEN_SPACE_SUNSHAFTS_INCLUDED
	/*
		Screen Space Sun Shafts
		Автор: DWM Team
	*/

	#include "common.h"

	#define SS_SUNSHAFTS_SAMPLES 15//10
	#define SS_SUNSHAFTS_LENGTH 7
	#define SS_SUNSHAFTS_FAR_PLANE 250
	#define SS_SUNSHAFTS_SUN_DIST 6400

	float perform_sunshafts(in float2 tc, in float2 pos2d)
	{
		float accum = 0;

		float s_direction = dot(eye_direction, normalize(L_sun_dir_w));

		if(s_direction <= 0)
		{
			float3 vsposition = G_BUFFER::load_position(tc);

			float3 direction = G_BUFFER::ws_vs(L_sun_dir_w.xyz);

			float3 vsposition_next = vsposition + direction * SS_SUNSHAFTS_SUN_DIST; // Wrong, fix me

			float2 tc_next = G_BUFFER::vs_tc(vsposition_next);

			float2 step = tc_next - tc;

			// dist to the sun
			// float sun_dist = 180 / (sqrt(1 - L_sun_dir_w.y * L_sun_dir_w.y));

			// sun pos
			// float3 sun_pos_world = sun_dist * L_sun_dir_w + eye_position;
			// float2 sun_pos_screen = G_BUFFER::ws_tc(sun_pos_world, 1);

			// sun-pixel vector
			// float2 sun_vec_screen = normalize(sun_pos_screen - tc);

			// float2 step = sun_vec_screen;

			float jitter = noise::get_4(pos2d, 1, 1);
			jitter = jitter * 0.5 + 0.5;

			step = step * jitter / SS_SUNSHAFTS_SAMPLES;

			float2 tc_step = tc;

			for(int i = 0; i < SS_SUNSHAFTS_SAMPLES; i++)
			{
				tc_step += step;

				if(is_in_quad(tc_step))
				{
					float depth_hit = G_BUFFER::load_depth_wsky(tc_step);
					accum += step(SS_SUNSHAFTS_FAR_PLANE, depth_hit);
				}
			}

			accum /= (float)SS_SUNSHAFTS_SAMPLES;
		}

		return accum;
	}
#endif
