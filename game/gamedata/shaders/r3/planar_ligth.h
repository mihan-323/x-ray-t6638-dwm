#if !defined(PLANAR_LIGHT)
#define PLANAR_LIGHT
	static const float spot_intensity = 1.0;
	static const float3	spot_color = float3(0.60, 0.55, 0.55);

	void planar_vertex_ligth(in float3 Nw, in float hemi, in float4 env, in float4 amb, out float3 light_sun, out float3 light_spot, out float3 diffuse)
	{
		float3 light_vec_w = L_sun_dir_w;

		float3 e0d = env_s0.SampleLevel(smp_base, Nw, 0);
		float3 e1d = env_s1.SampleLevel(smp_base, Nw, 0);

		float3 env_d = env.xyz * lerp(e0d, e1d, env.w);
		env_d *= env_d;

		diffuse = env_d * (amb.xyz + hemi);

		float angle_sun = saturate(-dot(Nw, light_vec_w));
		light_sun = L_sun_color * angle_sun;

		float angle_spot = saturate(-dot(Nw, eye_direction));
		float att_spot = 1 - saturate(eye_direction.y);
		light_spot = spot_color * att_spot * angle_spot * spot_intensity;
	}

	void planar_vertex_proj_camera_space(in float3 w_pos, in float4x4 vp_camera, out float2 screen_tc)
	{
		float4 Pp_camera = mul(vp_camera, float4(w_pos, 1));

		float2 proj_tc = Pp_camera.xy / Pp_camera.ww;

		screen_tc = (proj_tc + float2(1, -1)) * float2(0.5, -0.5); // [-1..1] to [0..1]
		screen_tc.y = (screen_tc.y - 0.5) * screen_res.y * screen_res.z + 0.5; // make quad
	}
#endif
