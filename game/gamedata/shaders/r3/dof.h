#if !defined(DOF_H)
#define	DOF_H
	#if defined(USE_DOF)
		uniform float4 dof_params;	// x - kernel size, y - focus, z - far, w - sky

		float4 dof(float2 tc)
		{
			float4 img = float4(0, 0, 0, s_image.SampleLevel(smp_rtlinear, tc, 0).w);

			float depth = G_BUFFER::load_depth(tc);

			if(depth <= 0.01) 
				depth = dof_params.w;

			float 	bias = saturate((depth - dof_params.y) / (dof_params.z - dof_params.y));
					bias = bias * bias * dof_params.x * screen_res.zw / 10;

			float3	sum = 0,
					msum = 0;

			float check = 0;

			[unroll(25)]for(unsigned int p_id = 0; p_id < 25; p_id++) 
			{
				float2 tc_bias = tc + bias * poisson25_round[p_id];

				float tap_depth = G_BUFFER::load_depth(tc_bias);

				if(tap_depth <= 0.01) 
					tap_depth = dof_params.w;

				float tap_contrib = saturate((tap_depth - dof_params.y) / (dof_params.z - dof_params.y));

				check += tap_contrib;

				tap_contrib = tap_contrib * tap_contrib;

				tc_bias = tc + bias * poisson25_round[p_id] * tap_contrib;

				float3 tap_color = s_image.SampleLevel(smp_rtlinear, tc_bias, 0);

				sum += tap_color;

				msum = max(tap_color, msum);
			}

			float mixer = dot(msum.xyz, LUMINANCE_VECTOR) + 0.5;
			mixer = saturate(mixer * mixer - 0.5);

			img.xyz = lerp(sum / 25, msum, mixer);

			return img;
		}
	#endif
#endif
