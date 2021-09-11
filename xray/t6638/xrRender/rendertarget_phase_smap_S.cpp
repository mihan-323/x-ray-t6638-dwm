#include "stdafx.h"

#pragma todo("!!! implement R2 spot shadow mapping")

void	CRenderTarget::phase_smap_spot_clear()
{

	RCache.clear_DepthView(rt_smap_depth->pZRT);

}

void	CRenderTarget::phase_smap_spot		(light* L)
{

	// Targets + viewport
	u_setrt(NULL);
	u_setzb(rt_smap_depth);

	D3D_VIEWPORT VP					=	{(float)L->X.S.posX, (float)L->X.S.posY, (float)L->X.S.size, (float)L->X.S.size, 0, 1};
	//CHK_DX								(HW.pDevice->SetViewport(&VP));
	RCache.set_Viewport(&VP);
	//HW.pContext->RSSetViewports(1, &VP);

	// Misc		- draw only front-faces //back-faces
	RCache.set_CullMode					(D3D_CULL_BACK);
	RCache.set_Stencil					( FALSE		);
	// no transparency
	#pragma todo("can optimize for multi-lights covering more than say 50%...")
	RCache.set_ColorWriteEnable	(FALSE);

}

void	CRenderTarget::phase_smap_spot_tsh	(light* L)
{

	VERIFY(!"Implement clear of the buffer for tsh!");
	VERIFY							(RImplementation.o.tshadows);
	RCache.set_ColorWriteEnable		();
	if (IRender_Light::OMNIPART == L->flags.type)	{
		// omni-part
		//CHK_DX							(HW.pDevice->Clear( 0L, NULL, D3DCLEAR_TARGET,	0xffffffff,	1.0f, 0L));
		FLOAT ColorRGBA[4] = {1.0f, 1.0f, 1.0f, 1.0f};
		RCache.clear_CurrentRenderTargetView(ColorRGBA);
	} else {
		// real-spot
		// Select color-mask
		ref_shader		shader			= L->s_spot;
		if (!shader)	shader			= s_accum_spot;
		RCache.set_Element				(shader->E[SE_L_FILL]	);

		// Fill vertex buffer
		Fvector2						p0,p1;
		u32		Offset;
		u32		C						= color_rgba	(255,255,255,255);
		float	_w						= float(L->X.S.size);
		float	_h						= float(L->X.S.size);
		float	d_Z						= EPS_S;
		float	d_W						= 1.f;
		p0.set							(.5f/_w, .5f/_h);
		p1.set							((_w+.5f)/_w, (_h+.5f)/_h );

		FVF::TL* pv						= (FVF::TL*) RCache.Vertex.Lock	(4,g_combine->vb_stride,Offset);
		pv->set							(EPS,			float(_h+EPS),	d_Z,	d_W, C, p0.x, p1.y);	pv++;
		pv->set							(EPS,			EPS,			d_Z,	d_W, C, p0.x, p0.y);	pv++;
		pv->set							(float(_w+EPS),	float(_h+EPS),	d_Z,	d_W, C, p1.x, p1.y);	pv++;
		pv->set							(float(_w+EPS),	EPS,			d_Z,	d_W, C, p1.x, p0.y);	pv++;
		RCache.Vertex.Unlock			(4,g_combine->vb_stride);
		RCache.set_Geometry				(g_combine);

		// draw
		RCache.Render					(D3DPT_TRIANGLELIST,Offset,0,4,0,2);
	}

}
