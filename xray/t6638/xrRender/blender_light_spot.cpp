#include "stdafx.h"
#pragma hdrstop

#include "blender_light_spot.h"

CBlender_accum_spot::CBlender_accum_spot() { description.CLS = 0; }
CBlender_accum_spot::~CBlender_accum_spot() {	}

void	CBlender_accum_spot::Compile(CBlender_Compile& C)
{
	IBlender::Compile(C);

	BOOL		blend = TRUE;
	D3D_BLEND	dest = blend ? D3D_BLEND_ONE : D3D_BLEND_ZERO;

	switch (C.iElement)
	{
	case SE_L_FILL:			// masking
		C.r_Pass("stub_notransform", "copy", FALSE, FALSE, FALSE);
		C.r_dx10Texture("s_base", C.L_textures[0]);
		C.r_dx10Sampler("smp_base");
		if (RImplementation.o.cspecular) C.r_ColorWriteEnable(true, true, true, false);
		C.r_End();
		break;
	case SE_L_UNSHADOWED:	// unshadowed
		C.r_Pass("accum_v", "accum_spot_unshadowed", FALSE, FALSE, FALSE, blend, D3D_BLEND_ONE, dest);
		C.r_dx10Texture("s_material", tex_t_material);
		C.r_dx10Texture("s_diffuse", tex_rt_Color);
		C.r_dx10Texture("s_position", tex_rt_Position);
		C.r_dx10Texture("s_depth", tex_t_depth);
		C.r_dx10Texture("s_depthms", tex_t_msaa_depth);
		C.r_dx10Texture("s_lmap", C.L_textures[0]);
		C.r_dx10Sampler("smp_material");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_rtlinear");
		if (RImplementation.o.cspecular) C.r_ColorWriteEnable(true, true, true, false);
		C.r_End();
		break;
	case SE_L_NORMAL:		// normal
		C.r_Pass("accum_v", "accum_spot_normal", FALSE, FALSE, FALSE, blend, D3D_BLEND_ONE, dest);
		C.r_dx10Texture("s_material", tex_t_material);
		C.r_dx10Texture("s_diffuse", tex_rt_Color);
		C.r_dx10Texture("s_position", tex_rt_Position);
		C.r_dx10Texture("s_depth", tex_t_depth);
		C.r_dx10Texture("s_depthms", tex_t_msaa_depth);
		C.r_dx10Texture("s_lmap", C.L_textures[0]);
		C.r_dx10Texture("s_smap", tex_rt_smap_depth);
		C.r_dx10Texture("s_vsm", tex_rt_vsm_depth);
		C.r_dx10Sampler("smp_material");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_smap");
		C.Jitter();
		if (RImplementation.o.cspecular) C.r_ColorWriteEnable(true, true, true, false);
		C.r_End();
		break;
	case SE_L_FULLSIZE:		// normal-fullsize
		C.r_Pass("accum_v", "accum_spot_fullsize", FALSE, FALSE, FALSE, blend, D3D_BLEND_ONE, dest);
		C.r_dx10Texture("s_material", tex_t_material);
		C.r_dx10Texture("s_diffuse", tex_rt_Color);
		C.r_dx10Texture("s_position", tex_rt_Position);
		C.r_dx10Texture("s_depth", tex_t_depth);
		C.r_dx10Texture("s_depthms", tex_t_msaa_depth);
		C.r_dx10Texture("s_lmap", C.L_textures[0]);
		C.r_dx10Texture("s_smap", tex_rt_smap_depth);
		C.r_dx10Texture("s_vsm", tex_rt_vsm_depth);
		C.r_dx10Sampler("smp_material");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_smap");
		C.Jitter();
		if (RImplementation.o.cspecular) C.r_ColorWriteEnable(true, true, true, false);
		C.r_End();
		break;
	case SE_L_TRANSLUENT:	// shadowed + transluency
		C.r_Pass("accum_v", "accum_spot_fullsize", FALSE, FALSE, FALSE, blend, D3D_BLEND_ONE, dest);
		C.r_dx10Texture("s_material", tex_t_material);
		C.r_dx10Texture("s_diffuse", tex_rt_Color);
		C.r_dx10Texture("s_position", tex_rt_Position);
		C.r_dx10Texture("s_depth", tex_t_depth);
		C.r_dx10Texture("s_depthms", tex_t_msaa_depth);
		C.r_dx10Texture("s_lmap", C.L_textures[0]);
		C.r_dx10Texture("s_smap", tex_rt_smap_depth);
		C.r_dx10Texture("s_vsm", tex_rt_vsm_depth);
		C.r_dx10Sampler("smp_material");
		C.r_dx10Sampler("smp_nofilter");
		C.r_dx10Sampler("smp_rtlinear");
		C.r_dx10Sampler("smp_smap");
		C.Jitter();
		if (RImplementation.o.cspecular) C.r_ColorWriteEnable(true, true, true, false);
		C.r_End();
		break;
	}
}
