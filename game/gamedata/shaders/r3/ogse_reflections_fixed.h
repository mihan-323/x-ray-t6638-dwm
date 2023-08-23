#ifndef OGSE_REFLECTIONS_H
#define OGSE_REFLECTIONS_H
#define USE_REFLECTIONS
#define REFL_RANGE 100
#define SKY_EPS float(0.001)

static const float2 resolution = screen_res.xy;
static const float2 inv_resolution = screen_res.zw;

float4 proj_to_screen(float4 proj)
{
	float4 screen = proj;
	screen.x = (proj.x + proj.w);
	screen.y = (proj.w - proj.y);
	screen.xy *= 0.5;
	return screen;
}

float	get_depth_fast			(float2 tc)	{return G_BUFFER::load_depth(tc);}
half is_sky(float depth)		{return step(depth, SKY_EPS);}
half is_not_sky(float depth)	{return step(SKY_EPS, depth);}

float4 get_reflection (float3 screen_pixel_pos, float3 next_screen_pixel_pos, float3 reflect)
{
	float4 final_color = {1.0,1.0,1.0,1.0};
	float2 factors = {1.f,1.f};
	
	screen_pixel_pos.xy *= screen_pixel_pos.z;
	next_screen_pixel_pos.xy *= next_screen_pixel_pos.z;
	
	float3 main_vec = next_screen_pixel_pos - screen_pixel_pos;
	// float3 grad_vec = main_vec / (max(abs(main_vec.x), abs(main_vec.y)) * 256);
	float3 grad_vec = main_vec * 0.75;
	
	// handle case when reflect vector faces the camera
	factors.x = dot(eye_direction, reflect);
	
	if ((factors.x < -0.5) || (screen_pixel_pos.z > REFL_RANGE)) return final_color;
	else
	{	
		float3 curr_pixel = screen_pixel_pos;
		float max_it = 40; //140;
		float i = 0;
		bool br = false;
		
		while ((i < max_it) && (br == false))
		{
			curr_pixel.xyz += grad_vec.xyz;
			float2 tc = curr_pixel.xy/curr_pixel.z;
			float depth = get_depth_fast(tc);
			depth = lerp(depth, 0.f, is_sky(depth));
			float delta = step(depth, curr_pixel.z)*step(screen_pixel_pos.z, depth);
			if (delta > 0.5)
			{
				final_color = s_image.Sample(smp_rtlinear, float4(tc,0,0));
				float2 tmp = tc;
				tmp.y = lerp(tmp.y, 0.5, step(0.5, tmp.y));
				float screendedgefact = saturate(distance(tmp , float2(0.5, 0.5)) * 2.0);
				final_color.w = pow(screendedgefact,6);
				br = true;
			}
			i += 1.0;
		}

		if (i < max_it) {
			curr_pixel.xyz -= grad_vec.xyz;
			float max_it = 5;
			grad_vec *= rcp(max_it);
			for (float i = 0; i < max_it; ++i)
			{
				curr_pixel.xyz += grad_vec.xyz;
				float2 tc = curr_pixel.xy/curr_pixel.z;
				float depth = get_depth_fast(tc);
				depth = lerp(depth, 0.f, is_sky(depth));

				float delta = step(depth, curr_pixel.z)*step(screen_pixel_pos.z, depth);
				if (delta > 0.5)
				{
					// edge detect
					final_color = s_image.Sample(smp_rtlinear, float4(tc,0,0));
					float2 temp = tc;
					// make sure that there is no fade down the screen
					temp.y = lerp(temp.y, 0.5, step(0.5, temp.y));
					float screendedgefact = saturate(distance(temp , float2(0.5, 0.5)) * 2.0);
					final_color.w = pow(screendedgefact,6);// * screendedgefact;
					i = max_it; //break;
				}
				++i;
			}
		}
		return final_color;
	}
}

float4 calc_reflections(float4 pos, float3 vreflect)
{
	float4 refl = {1.0,1.0,1.0,1.0};
	
	float3 v_pixel_pos = mul((float3x4)m_V, pos);
	float4 p_pixel_pos = mul(m_VP, pos);
	float4 s_pixel_pos = proj_to_screen(p_pixel_pos);
	s_pixel_pos.xy /= s_pixel_pos.w;
	s_pixel_pos.z = v_pixel_pos.z;
		
	float3 reflect_vec = normalize(vreflect);
	float3 W_m_point = pos.xyz + reflect_vec;

	float3 V_m_point = mul((float3x4)m_V, float4(W_m_point, 1.0));
	float4 P_m_point = mul(m_VP, float4(W_m_point, 1.0));
	float4 S_m_point = proj_to_screen(P_m_point);
	S_m_point.xy /= S_m_point.w;
	S_m_point.z = V_m_point.z;
	
	refl = get_reflection(s_pixel_pos.xyz, S_m_point.xyz, reflect_vec);
	
	return refl;
}
#endif