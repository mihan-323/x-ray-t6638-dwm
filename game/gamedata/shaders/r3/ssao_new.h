#ifndef SSAO_H
#define SSAO_H

	static uint debug_disable = 0;
	static uint debug_early_out = 0;
	//#define debug_disable_val !DEVX
	#define debug_disable_val 0

	// 
		// Intersection of the ray with view position
	// 

	struct INTERSECTION
	{
		float3 position;
		float3 hit;
		float2 tc;
		uint found;
	};

	INTERSECTION find_intersection(uint		steps, 
								   float3 	position, 
								   float3 	direction, 
								   float 	radius, 
								   float 	hash, 
								   float 	increase)
	{
		INTERSECTION inter = { position, float3(0, 0, 0), float2(0, 0), 0 };

		direction *= radius * hash / steps;

		for(uint j = 0; j < steps; j++)
		{
			inter.position += direction;
			direction *= increase;

			inter.tc = G_BUFFER::vs_tc(inter.position);
			inter.hit = G_BUFFER::load_position(inter.tc);

			if(is_in_range(inter.position.z - inter.hit.z, 0, 1))
			{
				inter.found = 1;
				break;
			}
		}

		return inter;
	}

	// 
		// Ambient occlusion by the screen space parth-tracing
	// 

	static uint 	ssao_pt_rays 		= 3;		// 
	static uint 	ssao_pt_steps 		= 5;		// 
	static float 	ssao_pt_radius 		= 0.4;		// 
	static float 	ssao_pt_increase 	= 1.2;		// 
	static float	ssao_pt_normal 		= 0.025;	// 
	// static float 	ssao_pt_power 		= 1.35;		// 
	static uint		ssao_pt_frames 		= 16;		// 12
	static float	ssao_pt_near 		= 0.05;		// 
	static float	ssao_pt_far 		= 100;		// 

	uniform Texture2D<float> s_ssao_prev;

	#ifndef dwframe_used
	#define dwframe_used
		uniform int dwframe; // current frame id
	#endif

	float ssao_path_trace(float2 tc, uint2 pos2d)
	{
		if(debug_disable && debug_disable_val)
			return 1;

		G_BUFFER::GBD gbd = G_BUFFER::load_P_N(tc);

		gbd.P += gbd.N * ssao_pt_normal;

		if(gbd.P.z <= ssao_pt_near || gbd.P.z >= ssao_pt_far)
			return 1;

		if(gbd.P.z <= 0.01)
			return 1;

		float depth = gbd.P.z;

		float3 up 		= abs(gbd.N.z) < 0.999f ? float3(0.0f, 0.0f, 1.0f) : float3(1.0f, 0.0f, 0.0f);
		float3 tangent 	= normalize(cross(up, gbd.N));
		float3 binormal = cross(gbd.N, tangent);

		float3x3 tbn = float3x3(tangent, binormal, gbd.N);

		float3 hash_tc = 0;
		hash_tc.xy = pos2d;
		hash_tc.z = dwframe % int(ssao_pt_frames / 1.5 + 0.5);

		// int hash_id = dwframe % (ssao_pt_frames - 1) + 1;
		
		// hash_tc.xy = pos2d % 4;
		// hash_tc.z = dwframe % ssao_pt_frames;

		if(debug_early_out) hash_tc.z = 0;

		float ssao = 0;

		float3 hash = noise::hash33(hash_tc);

		// int2 pos1 = pos2d.xy;
		// float2 gr1 = pos1 % 4;
		// float jitter1 = gr1.y * 0.25 + gr1.x * 0.0625 + 0.0625;
		
		// int2 pos2 = pos2d.xy + 2;
		// float2 gr2 = pos2 % 4;
		// float jitter2 = gr2.y * 0.25 + gr2.x * 0.0625 + 0.0625;
		
		// float2 hash_curr = float2(jitter1, jitter2);
		// float2 hash_curr = noise::hash23(hash_tc);
		// float2 hash_tap = hash_curr / ssao_pt_frames;
		
		// float2 hash = hash_tap * hash_id;
		
		uint steps = ssao_pt_steps;
		uint rays = ssao_pt_rays;
		float inc = ssao_pt_increase;

		float radius = ssao_pt_radius * sqrt(gbd.P.z);

		float circle = 6.2831853*16;
		float sector = circle / rays;
		float angle = circle * hash.x;

		float2 direction;
		sincos(angle, direction.y, direction.x);

		float2 rotation;
		sincos(sector, rotation.y, rotation.x);

		float2x2 rot = float2x2(rotation.x, -rotation.y, rotation.y, rotation.x);

		for(uint i = 0; i < rays; i++) 
		{
			float st = (i + hash.y) / rays;
			float ct = sqrt(1 - st*st);

			float2 uv_dir = direction * ct;
			direction = mul(direction, rot);

			float3 sph = float3(uv_dir, st);
			sph = mul(sph, tbn);

			INTERSECTION inter = find_intersection(steps, gbd.P, sph, radius, hash.z, inc);

			if(inter.found)
			{
				float dist = inter.position.z - inter.hit.z;
				float distsqr = 1 - dist*dist;
				ssao += distsqr;
			}
		}

		ssao = 1 - ssao / rays;
		// ssao = pow(abs(ssao), ssao_pt_power);

		float fade = smoothstep(ssao_pt_near, 1, gbd.P.z);
		fade *= 1 - smoothstep(ssao_pt_far * 0.75, ssao_pt_far, gbd.P.z);
		ssao = lerp(1, ssao, fade);

		ssao = saturate(ssao);
		
		return ssao;
	}

	// 
		// Spatial filter
	// 
	
	#define SSAO_USE_SPATIAL_FILTER
	
	// use some filters from RSM shader
	uniform Texture2D<float> s_ssao;

	// halton sequence (2, 3) discretized to 7x7 region
	static const int2 offsets[15] =
	{
		-3,  0,	-3,  3,	-2, -2,
		-2,  1,	-1, -2,	-1, -1,
		 0, -3,	 0,  1,	 0,  2,
		 1,  0,	 1,  2,	 2, -3,
		 2,  1,	 3,  0,	 3, -1,
	};
	
	float ssao_path_trace_spatial_filter(Texture2D<float> s_ssao, float2 tc, int r, out float _e)
	{
		#ifndef SSAO_USE_SPATIAL_FILTER
		_e = 1;
			return s_ssao.Sample(smp_rtlinear, tc);
		#else
			if(debug_disable && debug_disable_val)
				return 1;

			float d = G_BUFFER::load_depth(tc);
			float3 n = G_BUFFER::load_normal(tc);

			float ssao = 0;

			// for(int i = -r; i <= r; i++)
			// {
				// for(int j = -r; j <= r; j++)
				// {
					// float2 bias = screen_res.zw * int2(i, j);
					// float d1 = G_BUFFER::load_depth(tc + bias);
					// float de = abs(d1 - d) < 0.05 * d;
					// float3 n1 = G_BUFFER::load_normal(tc + bias);
					// float ne = abs(dot(n, n1)) > 0.9;
					// float e = de * ne;
					// ssao += s_ssao.Sample(smp_nofilter, tc + bias * e);
				// }
			// }

			_e = 0;
			for(int i = 0; i < 15; i++)
			{
				float2 bias = screen_res.zw * offsets[i];
				float d1 = G_BUFFER::load_depth(tc + bias);
				float de = abs(d1 - d) < 0.05 * d; // 0.05 - depth difference factor
				float3 n1 = G_BUFFER::load_normal(tc + bias);
				float ne = abs(dot(n, n1)) > 0.9; // 0.9 - normal parallel factor
				float e = de * ne;
				_e += e ;
				ssao += s_ssao.Sample(smp_nofilter, tc + bias * e);
			}

			_e /= 15;
			return ssao / 15;
		#endif
	}
	
	// 
		// Temporal reprojection
	// 

	static float ssao_temporal 		= 1.0 - 1.0 / ssao_pt_frames;
	// static float ssao_temporal_th1	= 0.003;
	static float ssao_temporal_th	= 0.05;

	uniform Texture2D<float> s_ssao_temp;

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

		float dist = length(gbd.P);

		float3 Pw = G_BUFFER::vs_ws(gbd.P, 1);
		float4 proj_next = mul(m_tVP, float4(Pw, 1));
		float2 tc_next = proj_next.xy / proj_next.ww * float2(0.5, -0.5) + 0.5;

		// if(gbd.mask)
			// tc_next = tc;

		float2 tc_next2 = tc_next;
		// tc_next = tc;

		float4 gbd_prev = G_BUFFER::load_history_packed(tc_next).z;

		float3 position_prev = G_BUFFER::unpack_position(tc_next, gbd_prev.z);

		float dist_prev = length(position_prev);

		// float ssao_temporal_th = gbd.mask ? DEVX : ssao_temporal_th0;

		bool is_plane = abs(dist_prev - dist) < ssao_temporal_th * dist;
		if(gbd.mask)
			is_plane = false;

		#ifdef USE_PT_DOWNSAMPLE
			float ssao_prev = s_ssao_prev.Sample(smp_rtlinear, tc_next2);
			float temporal = ssao_temporal * is_plane;
			float ssao = s_ssao_small.Sample(smp_rtlinear, tc);

			if(!is_in_quad(tc_next, 0, 1))
				return ssao;
		
			if(debug_early_out)
				return ssao;

			ssao = lerp(ssao, ssao_prev, temporal);

			return ssao;
		#else
			// float ssao_prev = s_ssao_prev.Sample(smp_rtlinear, tc_next2);
			// float temporal = ssao_temporal * is_plane;
			float temporal = ssao_temporal;

			float2 velocity = saturate(tc_next - tc);
			float subpixel_correction = frac(max(abs(velocity.x) * screen_res.x, abs(velocity.y) * screen_res.y)) * 0.5f;
			// factor = factor - subpixel_correction * 0.0375;
			temporal = temporal - subpixel_correction * temporal * 0.25; // 0.25 - subpixel correction factor

			float ssao = s_ssao.Sample(smp_rtlinear, tc);
			float _e;
			if(!is_plane)
			{
				ssao = ssao_path_trace_spatial_filter(s_ssao, tc, 2, _e);
				if(gbd.mask)
				{
					float3 n = G_BUFFER::load_normal(tc);
					float3 n1 = G_BUFFER::load_normal(tc_next);
					if(abs(dot(n, n1)) > 0.9) // 0.9 - weapon smoothing factor
					{
						float ssao_prev = s_ssao_prev.Sample(smp_rtlinear, tc_next2);
						ssao = lerp(ssao, ssao_prev, temporal);
					}
				}
			}
			else
			{
				float ssao_prev = ssao_path_trace_spatial_filter(s_ssao_prev, tc_next2, 2, _e);
				// ssao = ssao_path_trace_spatial_filter(s_ssao, tc, 2);
				temporal *= _e * 0.75 + 0.25; // 0.75 - temporal edge sharpening factor + 0.25 temporal edge smoothing factor = 1.0
				ssao = lerp(ssao, ssao_prev, temporal);
			}
		#endif

		// if(!is_in_quad(tc_next, 0, 1))
			// return ssao;
	
		// if(debug_early_out)
			// return ssao;

		// if(is_plane)
			// ssao = lerp(ssao, ssao_prev, temporal);

		return ssao;
	}
#endif
