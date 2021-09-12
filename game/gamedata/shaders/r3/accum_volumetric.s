function normal		(shader, t_base, t_second, t_detail)
	shader:begin	("accum_volumetric", "accum_volumetric")
			: fog		(false)
			: zb 		(true,false)
			: blend		(true,blend.one,blend.one)
			: sorting	(2, false)

	shader:dx10texture	("s_lmap", "shaders\\volume")	
	shader:dx10texture	("s_smap", "null")
	shader:dx10texture	("hqjitter0", "fx\\fx_noise")
	shader:dx10texture	("s_bnoise", "shaders\\blue_noise")

	shader:dx10sampler	("smp_rtlinear")
	shader:dx10sampler	("smp_linear")
	shader:dx10sampler	("smp_smap")
end