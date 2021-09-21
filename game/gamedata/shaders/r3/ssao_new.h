#ifndef SSAO_H
#define SSAO_H

	static uint debug_disable = 0;
	#define debug_disable_val !DEVX

	// !with MSAA enabled used low quality 16-bit path tracing

	// 
		// Project the vector to screen by view & screen positions
	// 

	float3 delinearize_vector(float3	view_vector,		// linear vector
							  float3	view_position, 		// linear position
							  float3	screen_position) 	// non-linear position
	{
		#ifdef MSAA_SAMPLES
			return view_vector;
		#else
			float4 screen_position_next = G_BUFFER::vs_vp(view_position + view_vector, 1);
			screen_position_next = screen_position_next / screen_position_next.w;
			screen_position_next.xy = screen_position_next.xy * float2(0.5, -0.5) + 0.5;

			float3 screen_vector = screen_position_next.xyz - screen_position;
			return screen_vector;
		#endif
	}

	// 
		// Make the normal-oriented hemisphere with the uniformly randomized radius and direction
	// 

	struct HEMISPHERE
	{
		float3 	hemisphere;
		uint 	valid;
	};

	HEMISPHERE hemisphere_random_screen(float3	hash, 
										float3 	view_position, 
										float3 	view_normal, 
										float3 	screen_position) 
	{
		float cos_theta = 1.0 - hash.x;
		float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

		float phi = 6.2831852 * hash.y;

		// from spherical to cartesian coordinates
		float3 sphere;
		sphere.x = sin_theta * cos(phi);
		sphere.y = sin_theta * sin(phi);
		sphere.z = cos_theta;
		sphere *= hash.z;

		float3 up 		= abs(view_normal.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
		float3 tangent 	= normalize(cross(up, view_normal));
		float3 binormal = cross(view_normal, tangent);

		HEMISPHERE hemisphere;
		hemisphere.hemisphere 	= float3((tangent * sphere.x) + (binormal * sphere.y) + (view_normal * sphere.z));
		hemisphere.valid 		= dot(hemisphere.hemisphere, view_normal) >= 0;
		hemisphere.hemisphere 	= delinearize_vector(hemisphere.hemisphere, view_position, screen_position);

		return hemisphere;
	}

	// 
		// Find the intersection of your ray with position
	// 

	struct INTERSECTION
	{
		uint found;
		float3 position;
		float2 tc;
	};

	INTERSECTION find_intersection_screen(uint 		steps, 
										  float3 	screen_position, 
										  float3 	screen_direction, 
										  float 	radius, 
										  float 	increase, 
										  float 	screen_bias, 
										  float 	screen_thickness)
	{
		INTERSECTION intersection = { 0, screen_position, float2(0, 0) };

		screen_direction = screen_direction * radius / steps;

		for(uint j = 0; j < steps; j++)
		{
			intersection.position += screen_direction;

			screen_direction *= increase;

			#ifdef MSAA_SAMPLES
				intersection.tc = G_BUFFER::vs_tc(intersection.position);
			#else
				intersection.tc = intersection.position.xy;
			#endif

			if(!is_in_quad(intersection.tc))
				break;

			#ifdef MSAA_SAMPLES
				float hit = G_BUFFER::load_depth(intersection.tc);
			#else
				float hit = G_BUFFER::load_depth_hw(intersection.tc);
			#endif

			float error = intersection.position.z - hit;

			if(is_in_range(error, screen_bias, screen_thickness))
			{
				intersection.found = 1;
				break;
			}
		}

		return intersection;
	}

	// 
		// Perform SSAO by screen space parth-tracing
	// 

	static uint 	ssao_pt_rays 		= 4;		// 
	static uint 	ssao_pt_steps 		= 8;		// 
	static float 	ssao_pt_radius 		= 6;		// 
	static float 	ssao_pt_increase 	= 1.2;		// 
	static float	ssao_pt_bias 		= 0.02;		// 
	static float 	ssao_pt_thickness 	= 1;		// 
	static float 	ssao_pt_power 		= 1.35;		// 
	static uint		ssao_pt_frames 		= 18;		// 
	static float	ssao_pt_near 		= 0.5;		// 
	static float	ssao_pt_far 		= 100;		// 

	uniform Texture2D<float> s_ssao_prev;

	uniform int dwframe;

	float ssao_path_trace(float2 tc)
	{
		if(debug_disable && debug_disable_val)
			return 1;

		// #ifdef USE_PT_DOWNSAMPLE
			// tc /= 0.5;
		// #endif

		G_BUFFER::GBD gbd = G_BUFFER::load_P_N_hemi_mtl_mask(tc);

		if(gbd.P.z <= ssao_pt_near || gbd.P.z >= ssao_pt_far)
			return 1;

		float3 screen_position;
		screen_position.xy = tc;
		screen_position.z = G_BUFFER::load_depth_hw(tc);

		#ifndef MSAA_SAMPLES
			gbd.P = G_BUFFER::unpack_position(tc, G_BUFFER::linearize_depth(screen_position.z)); // replace 16-bit position to 24-bit
		#endif

		if(gbd.P.z <= 0.01 || gbd.mask)
			return 1;

		float bias = lerp(ssao_pt_bias, 0.25, smoothstep(ssao_pt_near, ssao_pt_far, gbd.P.z));

		uint	rm_steps		= ssao_pt_steps;
		float 	rm_radius		= ssao_pt_radius * sqrt(gbd.P.z) * 2 / rm_steps; // * 2 because hash give average 0.5 value
		float	rm_bias			= delinearize_vector(float3(0, 0, bias),	  gbd.P, screen_position).z;
		float 	rm_thickness	= delinearize_vector(float3(0, 0, ssao_pt_thickness), gbd.P, screen_position).z;
		float 	rm_increase		= ssao_pt_increase;

		float2	hash_tc			= tc * 64;

		float ssao = 0;

		for(uint i = 0; i < ssao_pt_rays; i++) 
		{
			float4 hash = noise::hash44(float4(hash_tc, dwframe % ssao_pt_frames, i));

			HEMISPHERE hemisphere = hemisphere_random_screen(hash.xyz, gbd.P, gbd.N, screen_position);

			if(!hemisphere.valid) 
				continue;

			#ifdef MSAA_SAMPLES
				INTERSECTION intersection = find_intersection_screen(rm_steps, gbd.P, hemisphere.hemisphere, rm_radius * hash.w, rm_increase, rm_bias, rm_thickness);
			#else
				INTERSECTION intersection = find_intersection_screen(rm_steps, screen_position, hemisphere.hemisphere, rm_radius * hash.w, rm_increase, rm_bias, rm_thickness);
			#endif

			ssao += intersection.found;
		}

		ssao = 1 - ssao / ssao_pt_rays;
		ssao = pow(abs(ssao), ssao_pt_power);

		float fade = smoothstep(ssao_pt_near, 1, gbd.P.z);
		fade *= 1 - smoothstep(ssao_pt_far * 0.75, ssao_pt_far, gbd.P.z);
		ssao = lerp(1, ssao, fade);

		return saturate(ssao);
	}

	// 
		// Filter SSAO
	// 

	static float ssao_temporal 		= 0.93;
	static float ssao_temporal_th 	= 0.15;
	static float ssao_edge_th 		= 0.03;
	static float ssao_normal_th 	= 0.02;

	uniform Texture2D<float> s_ssao, s_ssao_temp;

	#ifdef USE_PT_DOWNSAMPLE
		uniform Texture2D<float> s_ssao_small;
	#endif

	uniform float4 ssao_pt_blur_params;

	uniform float4x4 m_tVP;

	float ssao_path_trace_temporal_filter(float2 tc)
	{
		if(debug_disable && debug_disable_val)
			return 1;

		G_BUFFER::GBD gbd = G_BUFFER::load_P_mask(tc);

		float3 Pw = G_BUFFER::vs_ws(gbd.P, 1);
		float4 proj_next = mul(m_tVP, float4(Pw, 1));
		float2 tc_next = proj_next.xy / proj_next.ww * float2(0.5, -0.5) + 0.5;

		#ifdef USE_PT_DOWNSAMPLE
			float ssao = s_ssao_small.Sample(smp_rtlinear, tc);
		#else
			float ssao = s_ssao.Sample(smp_rtlinear, tc);
		#endif

		if(gbd.mask || !is_in_quad(tc_next)) 
			return ssao;

		float4 gbd_prev = G_BUFFER::load_history_packed(tc_next).z;

		float depth_prev = gbd_prev.z;
		uint plane0 = abs(depth_prev - gbd.P.z) < ssao_temporal_th * gbd.P.z;

		float ssao_prev = s_ssao_prev.Sample(smp_rtlinear, tc_next);
		float temporal = ssao_temporal * plane0;
		ssao = lerp(ssao, ssao_prev, temporal);

		return ssao;
	}

	float ssao_path_trace_blur_filter(float2 tc)
	{
		if(debug_disable && debug_disable_val)
			return 1;

		float ssao = 0;

		G_BUFFER::GBD gbd = G_BUFFER::load_P_N(tc);

		float dist = length(gbd.P);

		int range = ssao_pt_blur_params.y / 2;

		for(int i = -range; i <= range; i++)
		{
			float2 bias = screen_res.zw * 2 * i * ssao_pt_blur_params.zw;

			G_BUFFER::GBD gbd_hit = G_BUFFER::load_P_N(tc + bias);

			uint plane0 = abs(dist - length(gbd_hit.P)) < ssao_edge_th * dist;
			uint plane1 = abs(gbd.N - gbd_hit.N) < ssao_normal_th.xxx;

			float2 tc_new = tc + (bias + screen_res.zw * 0.5) * plane0 * plane1;

			ssao += s_ssao.SampleLevel(smp_rtlinear, tc_new, 0);
		}

		ssao /= ssao_pt_blur_params.y + 1;

		return ssao;
	}
#endif
