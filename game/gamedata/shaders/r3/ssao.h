#ifndef SSAO_H
#define SSAO_H

	static uint debug_disable = 0;
	#define debug_disable_val !DEVX

	// 
		// Make the sphere with the uniformly randomized radius and direction
	// 

	float3 sphere_random(float3 hash) 
	{
		float cos_theta = 1.0 - hash.x;
		float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

		float phi = 2.0 * M_PI * hash.y;

		// from spherical to cartesian coordinates
		float3 sphere;
		sphere.x = sin_theta * cos(phi);
		sphere.y = sin_theta * sin(phi);
		sphere.z = cos_theta;
		sphere *= hash.z;

		return sphere;
	}

	// 
		// Make the normal-oriented hemisphere with the uniformly randomized radius and direction
	// 

	struct HEMISPHERE
	{
		float3 	hemisphere;
		uint 	valid;
	};

	HEMISPHERE hemisphere_random(float3	hash, 
								 float3	view_position, 
								 float3	view_normal) 
	{
		float3 sphere 	= sphere_random(hash);

		float3 up 		= abs(view_normal.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
		float3 tangent 	= normalize(cross(up, view_normal));
		float3 binormal = cross(view_normal, tangent);

		HEMISPHERE hemisphere;
		hemisphere.hemisphere 	= float3((tangent * sphere.x) + (binormal * sphere.y) + (view_normal * sphere.z));
		hemisphere.valid 		= dot(hemisphere.hemisphere, view_normal) >= 0;

		return hemisphere;
	}

	// 
		// Perform SSAO by rough approximation of the depth error
	// 

	static uint		ssao_samples	= 6;		// Количество проходов
	static float	ssao_radius 	= 1;		// Максимальный радиус SSAO
	static float	ssao_bias 		= 0.015;	// Смещение от точки пересечения
	static float	ssao_power 		= 4;		// Сила затенения

	float ssao(float2 tc)
	{
		if(debug_disable && debug_disable_val)
			return 1;

		G_BUFFER::GBD gbd = G_BUFFER::load_P_N_hemi_mtl_mask(tc);

		if(gbd.P.z <= 0.01)
			return 1;

		uint2 pos2d4 = tc * screen_res.xy;
			  pos2d4 %= 4;

		float3 position = gbd.P + gbd.N * ssao_bias;

		float ssao = 0;

		for(uint i = 0; i < ssao_samples; i++) 
		{
			float4 hash = noise::hash43(float3(pos2d4, i));

			HEMISPHERE hemisphere = hemisphere_random(hash.xyz, gbd.P, gbd.N);

			if(!hemisphere.valid)
				continue;

			hemisphere.hemisphere *= ssao_radius * hash.w;

			float3 position_new = position + hemisphere.hemisphere * ssao_radius * hash.w; 

			float2 tc = G_BUFFER::vs_tc(position_new);

			if(!is_in_quad(tc))
				break;

			float hit = G_BUFFER::load_depth_wsky(tc);

			float error = position_new.z - hit;

			if(is_in_range(error, ssao_bias, 1))
				ssao += 1 - error * error;
		}

		ssao = pow(1 - ssao / ssao_samples, ssao_power);

		return ssao;
	}

	// 
		// Filter SSAO
	// 

	// static float ssao_edge_th = 0.1;

	uniform Texture2D<float> s_ssao;

	float ssao_filter(float2 tc)
	{
		if(debug_disable && debug_disable_val)
			return 1;

		float2 pixel = screen_res.zw;

		// float depth = G_BUFFER::load_depth(tc);

		// float threshold = depth * ssao_edge_th;

		// float depth0 = G_BUFFER::load_depth(tc + pixel * int2( 0,  1));
		// float depth1 = G_BUFFER::load_depth(tc + pixel * int2( 0, -1));
		// float depth2 = G_BUFFER::load_depth(tc + pixel * int2( 1,  0));
		// float depth3 = G_BUFFER::load_depth(tc + pixel * int2(-1,  0));

		// if(abs(depth - depth0) > threshold ||
		   // abs(depth - depth1) > threshold ||
		   // abs(depth - depth2) > threshold ||
		   // abs(depth - depth3) > threshold)
		// {
			// return s_ssao.Sample(smp_rtlinear, tc);
		// }
		// else
		// {
			return (s_ssao.Sample(smp_rtlinear, tc + pixel * float2( 1.5, -0.5))
				  + s_ssao.Sample(smp_rtlinear, tc + pixel * float2( 1.5,  1.5))
				  + s_ssao.Sample(smp_rtlinear, tc + pixel * float2(-0.5, -0.5))
				  + s_ssao.Sample(smp_rtlinear, tc + pixel * float2(-0.5,  1.5))) * 0.25;
		// }
	}
#endif
