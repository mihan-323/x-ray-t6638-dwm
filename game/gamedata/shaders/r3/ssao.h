#ifndef SSAO_H
#define SSAO_H

	static uint debug_disable = 0;
	#define debug_disable_val !DEVX

	// 
		// Vector in the sphere with a random uniformly distributed radius
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
		// Vector in the normal-oriented hemisphere with a random uniformly distributed radius
	// 

	float3 hemisphere_random(float3	hash, float3 view_normal) 
	{
		float3 sphere 	= sphere_random(hash);

		float3 up 		= abs(view_normal.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
		float3 tangent 	= normalize(cross(up, view_normal));
		float3 binormal = cross(view_normal, tangent);

		float3 hemi = float3((tangent * sphere.x) + (binormal * sphere.y) + (view_normal * sphere.z));

		return hemi;
	}

	// 
		// Perform ambient occlusion by the very roughly approximating the depth difference
	// 

	static uint		ssao_samples	= 6;
	static float	ssao_radius 	= 0.15;
	static float	ssao_normal		= 0.0015;
	static float	ssao_power 		= 2;

	float ssao(float2 tc)
	{
		if(debug_disable && debug_disable_val)
			return 1;

		G_BUFFER::GBD gbd = G_BUFFER::load_P_N_hemi_mtl_mask(tc);

		if(gbd.P.z <= 0.01)
			return 1;

		uint2 pos2d4 = tc * screen_res.xy;
			  pos2d4 %= 4;

		float3 position = gbd.P + gbd.N * ssao_normal * gbd.P.z;

		float radius = ssao_radius * sqrt(gbd.P.z);

		float occ = 0;

		for(uint i = 0; i < ssao_samples; i++) 
		{
			float3 hash = noise::hash33(float3(pos2d4, i));

			float3 hemi = hemisphere_random(hash, gbd.N);

			float3 position_new = position + hemi * radius;

			float2 tc = G_BUFFER::vs_tc(position_new);

			float hit = G_BUFFER::load_depth_wsky(tc);

			float error = position_new.z - hit;

			if(is_in_range(error, 0, 1))
				occ += 1 - error * error;
		}

		occ = pow(1 - occ / ssao_samples, ssao_power);

		float att = smoothstep(0.8, 1.1, length(gbd.P.xyz));
		occ = lerp(1, occ, att);

		return occ;
	}

	// MXAO from ReShade

	float ssao_mxao(float2 tc, float2 pos2d)
	{
		if(debug_disable && debug_disable_val)
			return 1;

		G_BUFFER::GBD gbd = G_BUFFER::load_P_N(tc);

		if(gbd.P.z <= 0.01)
			return 1;

		float far_plane = 250;

		float aspect = screen_res.x / screen_res.y;

		float3 position = gbd.P;
		float3 normal = gbd.N;

		float sample_jitter = dot(floor(pos2d % 4 + 0.1), float2(0.0625, 0.25)) + 0.0625;

		int samples = 16;
		float power = 1.5;
		float radius = 1.5;
		float normal_bias = 0.05;

		float scaled_radius  = 0.25 * radius / (samples * (position.z + 2.0));
		float falloff_factor = -1.0/(radius * radius);

		position += normal * normal_bias;

		float2 sample_uv, sample_direction;
		sincos(38.3994112 * sample_jitter, sample_direction.x, sample_direction.y); //2.3999632 * 16
		sample_direction *= scaled_radius;   

		float4 color = 0.0;

		[loop]
		for(int i = 0; i < samples; i++)
		{
			sample_uv = tc + sample_direction.xy * aspect * (i + sample_jitter);   

			sample_direction.xy = mul(sample_direction.xy, 
									  float2x2(0.76465, -0.64444, 0.64444, 0.76465)); //cos/sin 2.3999632 * 16            

			float3 position_curr = G_BUFFER::load_position(sample_uv);

			float dist = saturate(position_curr.z - position.z);

			float3 occlusion_vector = -position + position_curr;
			float  occlusion_distance_squared = dot(occlusion_vector, occlusion_vector);
			float  occlusion_normal_angle = dot(occlusion_vector, normal) * rsqrt(occlusion_distance_squared);

			float sample_occlusion = saturate(1.0 + falloff_factor * occlusion_distance_squared) 
								   * saturate(occlusion_normal_angle - normal_bias);

			color.w += sample_occlusion;
		}

		color = saturate(color / ((1.0 - normal_bias) * samples) * 2.0);
		color.w = pow(1 - color.w, power);

		return color.w;
	}

	// 
		// Simple 16-pixel blur by bilinear filtering
	// 

	uniform Texture2D<float> s_ssao;

	float ssao_filter(float2 tc)
	{
		// return s_ssao.Sample(smp_rtlinear, tc);

		if(debug_disable && debug_disable_val)
			return 1;

		float2 pixel = screen_res.zw;

		return (s_ssao.Sample(smp_rtlinear, tc + pixel * float2( 1.5, -0.5))
			  + s_ssao.Sample(smp_rtlinear, tc + pixel * float2( 1.5,  1.5))
			  + s_ssao.Sample(smp_rtlinear, tc + pixel * float2(-0.5, -0.5))
			  + s_ssao.Sample(smp_rtlinear, tc + pixel * float2(-0.5,  1.5))) * 0.25;
	}
#endif
