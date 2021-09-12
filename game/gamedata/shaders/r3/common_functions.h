// Подключаем доп. функции и дефайны

#ifndef COMMON_FUNC_INCLUDED
#define COMMON_FUNC_INCLUDED

	void tonemap(out float4 low, out float4 high, float3 rgb, float scale, float overbright)
	{
		rgb = rgb * scale;

		low = 0;

		float max_white = 1 + overbright;
		float3 numerator = rgb * (1 + (rgb / (max_white * max_white)));

		low = saturate(numerator / (1 + rgb)).xyzz;

		high = rgb.xyzz / def_hdr;
	}

	bool is_in_range(float val, float min, float max)
	{
		return val >= min && val <= max;
	}

	bool is_in_quad(float2 p, float2 lt = float2(0, 0), float2 rb = float2(1, 1))
	{
		return is_in_range(p.x, lt.x, rb.x) && is_in_range(p.y, lt.y, rb.y);
	}

	float calc_vignette(in float2 tc, in float power, bool need_check_ss = false)
	{
		float2 ct = 1 - tc;

		float vignette = 1;

		if(!is_in_quad(tc)) vignette = 0;

		if(tc.y < power) vignette *= saturate(tc.y) / power;
		if(tc.x < power) vignette *= saturate(tc.x) / power;
		if(ct.y < power) vignette *= saturate(ct.y) / power;
		if(ct.x < power) vignette *= saturate(ct.x) / power;

		return vignette;
	}

	float calc_fogging(float4 w_pos)      
	{
		return dot(w_pos, fog_plane);         
	}

	float3 calc_model_lq_lighting(float3 norm_w)    
	{
		return L_material.xxx * max(0, norm_w.yyy) * L_hemi_color.xyz + L_ambient.xyz + L_material.yyy * L_sun_color.xyz * saturate(dot(norm_w, -L_sun_dir_w));
	}

	float2 unpack_tc_base(float2 tc, float du, float dv) { return (tc.xy + float2(du,dv)) * (32.f / 32768.f); }

	float3 unpack_normal(float3 v)	{ return v * 2 - 1; }

	float3 unpack_bx2(float3 v)	{ return v * 2 - 1; }
	float3 unpack_bx4(float3 v)	{ return v * 4 - 2; } // !reduce the amount of stretching from 4*v-2 and increase precision

	float2 unpack_tc_lmap(float2 tc) { return tc * (1.f / 32768.f); } // [-1 .. +1] 

	float4 unpack_D3DCOLOR(float4 c) { return c.bgra; }
	float3 unpack_D3DCOLOR(float3 c) { return c.bgr; }

	float3 p_hemi(float2 tc)
	{
		float4 t_lmh = s_hemi.Sample(smp_rtlinear, tc);
		return t_lmh.a;
	}

	float3 v_hemi(float3 n)
	{
		return L_hemi_color.xyz * (0.5f + 0.5f * n.yyy);       
	}

	float3 v_sun(float3 n)                        	
	{
		return L_sun_color * dot(n, -L_sun_dir_w);                
	}

	float2 tc_zoom(float2 tc, float zoom)
	{
		float rcpzoom = rcp(zoom);
		float add = 1 - rcpzoom;
		return (add * 0.5) + (tc * rcpzoom);
	}
#endif
