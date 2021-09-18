#include "stdafx.h"
#include "du_cone.h"

//extern Fvector du_cone_vertices[DU_CONE_NUMVERTEX];

void CRenderTarget::accum_spot	(light* L)
{
	phase_accumulator				();
	RImplementation.stats.l_visible	++;

	// *** assume accumulator already setup ***
	// *****************************	Mask by stencil		*************************************
	ref_shader		shader;

	if (IRender_Light::OMNIPART == L->flags.type)	{
			shader		= L->s_point;
		if (!shader)
		{	
			shader		= s_accum_point;
		}
	} else {
		shader		= L->s_spot;
		if (!shader)
		{
			shader		= s_accum_spot;
		}	
	}

	BOOL	bIntersect			= FALSE; //enable_scissor(L);
	{
		// setup xform
		L->xform_calc					();
		RCache.set_xform_world			(L->m_xform			);
		RCache.set_xform_view			(Device.mView		);
		RCache.set_xform_project		(Device.mProject	);
		bIntersect						= enable_scissor	(L);
		enable_dbt_bounds				(L);

		// *** similar to "Carmack's reverse", but assumes convex, non intersecting objects,
		// *** thus can cope without stencil clear with 127 lights
		// *** in practice, 'cause we "clear" it back to 0x1 it usually allows us to > 200 lights :)
		//	Done in blender!
		//RCache.set_ColorWriteEnable		(FALSE);
		RCache.set_Element		(s_accum_mask->E[SE_MASK_SPOT]);		// masker

		// backfaces: if (stencil>=1 && zfail)			stencil = light_id
		RCache.set_CullMode		(D3D_CULL_FRONT);

		if (RImplementation.o.aa_mode == AA_MSAA)
			RCache.set_Stencil(TRUE, D3D_COMPARISON_LESS_EQUAL, dwLightMarkerID, 0x01, 0x7f, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE);
		else
			RCache.set_Stencil(TRUE, D3D_COMPARISON_LESS_EQUAL, dwLightMarkerID, 0x01, 0xff, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE);

		draw_volume					(L);

		// frontfaces: if (stencil>=light_id && zfail)	stencil = 0x1
		RCache.set_CullMode		(D3D_CULL_BACK);

		if (RImplementation.o.aa_mode == AA_MSAA)
			RCache.set_Stencil(TRUE, D3D_COMPARISON_LESS_EQUAL, 0x01, 0x7f, 0x7f, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE);
		else
			RCache.set_Stencil(TRUE, D3D_COMPARISON_LESS_EQUAL, 0x01, 0xff, 0xff, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE);

		draw_volume					(L);
	}

	// nv-stencil recompression
	if (RImplementation.o.nvstencil)	u_stencil_optimize();

	// *****************************	Minimize overdraw	*************************************
	// Select shader (front or back-faces), *** back, if intersect near plane
	RCache.set_ColorWriteEnable		();
	RCache.set_CullMode				(D3D_CULL_FRONT);		// back

	// 2D texgens 
	Fmatrix			m_Texgen;			u_compute_texgen_screen	(m_Texgen	);
	Fmatrix			m_Texgen_J;			u_compute_texgen_jitter	(m_Texgen_J	);

	// Shadow xform (+texture adjustment matrix)
	Fmatrix			m_Shadow,m_Lmap;
	{
		float			smapsize			= float(RImplementation.o.smapsize);
		float			fTexelOffs			= (.5f / smapsize);
		float			view_dim			= float(L->X.S.size-2)/smapsize;
		float			view_sx				= float(L->X.S.posX+1)/smapsize;
		float			view_sy				= float(L->X.S.posY+1)/smapsize;
		float			fRange				= float(1.f)* r__ls_depth_scale;
		float			fBias				= r__ls_depth_bias;
		Fmatrix			m_TexelAdjust		= {
			view_dim/2.f,							0.0f,									0.0f,		0.0f,
			0.0f,									-view_dim/2.f,							0.0f,		0.0f,
			0.0f,									0.0f,									fRange,		0.0f,
			view_dim/2.f + view_sx + fTexelOffs,	view_dim/2.f + view_sy + fTexelOffs,	fBias,		1.0f
		};

		// compute xforms
		Fmatrix			xf_world;		xf_world.invert	(Device.mView);
		Fmatrix			xf_view			= L->X.S.view;
		Fmatrix			xf_project;		xf_project.mul	(m_TexelAdjust,L->X.S.project);
		m_Shadow.mul					(xf_view, xf_world);
		m_Shadow.mulA_44				(xf_project	);

		// lmap
						view_dim			= 1.f;
						view_sx				= 0.f;
						view_sy				= 0.f;
		Fmatrix			m_TexelAdjust2		= {
			view_dim/2.f,							0.0f,									0.0f,		0.0f,
			0.0f,									-view_dim/2.f,							0.0f,		0.0f,
			0.0f,									0.0f,									fRange,		0.0f,
			view_dim/2.f + view_sx + fTexelOffs,	view_dim/2.f + view_sy + fTexelOffs,	fBias,		1.0f
		};

		// compute xforms
		xf_project.mul		(m_TexelAdjust2,L->X.S.project);
		m_Lmap.mul			(xf_view, xf_world);
		m_Lmap.mulA_44		(xf_project	);
	}

	// Common constants
	Fvector		L_dir,L_clr,L_pos;	float L_spec;
	L_clr.set					(L->color.r,L->color.g,L->color.b);
	L_clr.mul					(L->get_LOD());
	L_spec						= u_diffuse2s	(L_clr);
	Device.mView.transform_tiny	(L_pos,L->position);
	Device.mView.transform_dir	(L_dir,L->direction);
	L_dir.normalize				();

	// Draw volume with projective texgen
	{
		// Select shader
		u32 _id = 0;
		if (L->flags.bShadow)
		{
			bool bFullSize = (L->X.S.size == RImplementation.o.smapsize);
			if (L->X.S.transluent)
				_id = SE_L_TRANSLUENT;
			else if (bFullSize)
				_id = SE_L_FULLSIZE;
			else
				_id = SE_L_NORMAL;
		}
		else
		{
			_id = SE_L_UNSHADOWED;
			m_Shadow = m_Lmap;
		}
		RCache.set_Element(shader->E[_id]);

		RCache.set_CullMode(D3D_CULL_FRONT);		// back

		// Constants
		float	att_R = L->range * .95f;
		float	att_factor = 1.f / (att_R * att_R);
		RCache.set_c("Ldynamic_pos", L_pos.x, L_pos.y, L_pos.z, att_factor);
		RCache.set_c("Ldynamic_color", L_clr.x, L_clr.y, L_clr.z, L_spec);
		RCache.set_c("m_texgen", m_Texgen);
		RCache.set_c("m_texgen_J", m_Texgen_J);
		RCache.set_c("m_shadow", m_Shadow);
		RCache.set_ca("m_lmap", 0, m_Lmap._11, m_Lmap._21, m_Lmap._31, m_Lmap._41);
		RCache.set_ca("m_lmap", 1, m_Lmap._12, m_Lmap._22, m_Lmap._32, m_Lmap._42);

		// Render if (light_id <= stencil && z-pass)
		// nvidia 8000, 9000 and 200 series
		if (RImplementation.o.aa_mode == AA_MSAA &&
			HW.FeatureLevel == D3D_FEATURE_LEVEL_10_0)
		{
			// per pixel
			RCache.set_c(dx10_msaa_id, 0);
			RCache.set_Stencil(TRUE, D3D_COMPARISON_EQUAL, dwLightMarkerID, 0xff, 0x00);
			RCache.set_CullMode(D3D_CULL_FRONT);
			draw_volume(L);

			// per sample
			for (u32 i = 0; i < RImplementation.o.msaa_samples; i++)
			{
				RCache.set_c(dx10_msaa_id, (int)i);
				StateManager.SetSampleMask(u32(1) << i);
				RCache.set_Stencil(TRUE, D3D_COMPARISON_EQUAL, dwLightMarkerID | 0x80, 0xff, 0x00);
				RCache.set_CullMode(D3D_CULL_FRONT);
				draw_volume(L);
			}

			RCache.set_c(dx10_msaa_id, 0);
			StateManager.SetSampleMask(0xffffffff);
			RCache.set_Stencil(TRUE, D3D_COMPARISON_LESS_EQUAL, dwLightMarkerID, 0xff, 0x00);
		}
		else if (RImplementation.o.aa_mode == AA_MSAA)
		{
			// per pixel
			RCache.set_Stencil(TRUE, D3D_COMPARISON_EQUAL, dwLightMarkerID, 0xff, 0x00);
			RCache.set_CullMode(D3D_CULL_FRONT);
			draw_volume(L);

			// per sample
			RCache.set_Stencil(TRUE, D3D_COMPARISON_EQUAL, dwLightMarkerID | 0x80, 0xff, 0x00);
			RCache.set_CullMode(D3D_CULL_FRONT);
			draw_volume(L);

			RCache.set_Stencil(TRUE, D3D_COMPARISON_LESS_EQUAL, dwLightMarkerID, 0xff, 0x00);
		}
		else
		{
			RCache.set_Stencil(TRUE, D3D_COMPARISON_LESS_EQUAL, dwLightMarkerID, 0xff, 0x00);
			draw_volume(L);
		}     
	}

	RCache.set_Scissor(0);
	//CHK_DX		(HW.pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE));
	//dwLightMarkerID					+=	2;	// keep lowest bit always setted up
	increment_light_marker();

	u_DBT_disable				();
}

void CRenderTarget::accum_volumetric(light* L)
{
	if (!L->flags.bVolumetric) return;

	phase_vol_accumulator();

	// *** assume accumulator setted up ***
	// *****************************	Mask by stencil		*************************************
	BOOL	bIntersect			= FALSE; //enable_scissor(L);
	{
		// setup xform
		L->xform_calc					();
		RCache.set_xform_world			(L->m_xform			);
		RCache.set_xform_view			(Device.mView		);
		RCache.set_xform_project		(Device.mProject	);
		bIntersect						= enable_scissor	(L);

		//enable_dbt_bounds				(L);
	}

	RCache.set_ColorWriteEnable		();
	RCache.set_CullMode				(D3D_CULL_NONE);		// back

	// 2D texgens 
	Fmatrix			m_Texgen;			u_compute_texgen_screen	(m_Texgen	);
	Fmatrix			m_Texgen_J;			u_compute_texgen_jitter	(m_Texgen_J	);

	// Shadow xform (+texture adjustment matrix)
	Fmatrix			m_Shadow,m_Lmap;
	Fmatrix			mFrustumSrc;
	CFrustum		ClipFrustum;
	{
		float			smapsize			= float(RImplementation.o.smapsize);
		float			fTexelOffs			= (.5f / smapsize);
		float			view_dim			= float(L->X.S.size-2)/smapsize;
		float			view_sx				= float(L->X.S.posX+1)/smapsize;
		float			view_sy				= float(L->X.S.posY+1)/smapsize;
		float			fRange				= float(1.f)* r__ls_depth_scale;
		float			fBias				= r__ls_depth_bias;
		Fmatrix			m_TexelAdjust		= {
			view_dim/2.f,							0.0f,									0.0f,		0.0f,
			0.0f,									-view_dim/2.f,							0.0f,		0.0f,
			0.0f,									0.0f,									fRange,		0.0f,
			view_dim/2.f + view_sx + fTexelOffs,	view_dim/2.f + view_sy + fTexelOffs,	fBias,		1.0f
		};

		// compute xforms
		Fmatrix			xf_world;		xf_world.invert	(Device.mView);
		Fmatrix			xf_view			= L->X.S.view;
		Fmatrix			xf_project;		xf_project.mul	(m_TexelAdjust,L->X.S.project);
		m_Shadow.mul					(xf_view, xf_world);
		m_Shadow.mulA_44				(xf_project	);

		// lmap
		view_dim			= 1.f;
		view_sx				= 0.f;
		view_sy				= 0.f;
		Fmatrix			m_TexelAdjust2		= {
			view_dim/2.f,							0.0f,									0.0f,		0.0f,
			0.0f,									-view_dim/2.f,							0.0f,		0.0f,
			0.0f,									0.0f,									fRange,		0.0f,
			view_dim/2.f + view_sx + fTexelOffs,	view_dim/2.f + view_sy + fTexelOffs,	fBias,		1.0f
		};

		// compute xforms
		xf_project.mul		(m_TexelAdjust2,L->X.S.project);
		m_Lmap.mul			(xf_view, xf_world);
		m_Lmap.mulA_44		(xf_project	);

		// Compute light frustum in world space
		mFrustumSrc.mul(L->X.S.project, xf_view);
		ClipFrustum.CreateFromMatrix( mFrustumSrc, FRUSTUM_P_ALL);
		//	Adjust frustum far plane
		//	4 - far, 5 - near
		ClipFrustum.planes[4].d -= 
			(ClipFrustum.planes[4].d + ClipFrustum.planes[5].d)*(1-L->m_volumetric_distance);
		
	}

	//	Calculate camera space AABB
	//	Adjust AABB according to the adjusted distance for the light volume
	Fbox	aabb;
	
	//float	scaledRadius = L->spatial.sphere.R * (1+L->m_volumetric_distance)*0.5f;
	float	scaledRadius = L->spatial.sphere.R * L->m_volumetric_distance;
	Fvector	rr = Fvector().set(scaledRadius,scaledRadius,scaledRadius);
	Fvector pt = L->spatial.sphere.P;
	pt.sub(L->position);
	pt.mul(L->m_volumetric_distance);
	pt.add(L->position);
	//	Don't adjust AABB
	Device.mView.transform(pt);
	aabb.setb( pt, rr);

	// Common constants
	float		fQuality = L->m_volumetric_quality;
	int			iNumSlises = (int)(VOLUMETRIC_SLICES*fQuality);
	//			min 10 surfaces
	iNumSlises = _max(10, iNumSlises);
	//	Adjust slice intensity
	fQuality	= ((float)iNumSlises)/VOLUMETRIC_SLICES;
	Fvector		L_dir,L_clr,L_pos;	float L_spec;
	L_clr.set					(L->color.r,L->color.g,L->color.b);
	L_clr.mul					(L->m_volumetric_intensity);
	L_clr.mul					(L->m_volumetric_distance);
	L_clr.mul					(1/fQuality);
	L_clr.mul					(L->get_LOD());
	L_spec						= u_diffuse2s	(L_clr);
	Device.mView.transform_tiny	(L_pos,L->position);
	Device.mView.transform_dir	(L_dir,L->direction);
	L_dir.normalize				();

	// Draw volume with projective texgen
	{
		//	Set correct depth surface
		//	It's slow. Make this when shader is created
		{
			char*		pszSMapName = tex_rt_smap_depth;

			STextureList* _T = &*s_accum_volume->E[0]->passes[0]->T;

			STextureList::iterator	_it		= _T->begin	();
			STextureList::iterator	_end	= _T->end	();
			for (; _it!=_end; _it++)
			{
				std::pair<u32,ref_texture>&		loader	=	*_it;
				u32			load_id	= loader.first;
				//	Shadowmap texture always uses 0 texture unit
				if (load_id==0)		
				{
					//	Assign correct texture
					loader.second.create(pszSMapName);
				}
			}
		}
		

		RCache.set_Element			(s_accum_volume/*shader*/->E[0]);

		// Constants
		float	att_R				= L->m_volumetric_distance*L->range*.95f;
		float	att_factor			= 1.f/(att_R*att_R);
		RCache.set_c				("Ldynamic_pos",	L_pos.x,L_pos.y,L_pos.z,att_factor);
		RCache.set_c				("Ldynamic_color",	L_clr.x,L_clr.y,L_clr.z,L_spec);
		RCache.set_c				("m_texgen",		m_Texgen	);
		RCache.set_c				("m_texgen_J",		m_Texgen_J	);
		RCache.set_c				("m_shadow",		m_Shadow	);
		RCache.set_ca				("m_lmap",		0,	m_Lmap._11, m_Lmap._21, m_Lmap._31, m_Lmap._41	);
		RCache.set_ca				("m_lmap",		1,	m_Lmap._12, m_Lmap._22, m_Lmap._32, m_Lmap._42	);
		RCache.set_c				("vMinBounds",		aabb.x1, aabb.y1, aabb.z1, 0);
		//	Increase camera-space aabb z size to compensate decrease of slices number
		RCache.set_c				("vMaxBounds",		aabb.x2, aabb.y2, aabb.z1 + (aabb.z2-aabb.z1)/fQuality, 0);

		//	Set up user clip planes
		{
			static shared_str	strFrustumClipPlane("FrustumClipPlane");
			//	TODO: DX10: Check if it's equivalent to the previouse code.
			//RCache.set_ClipPlanes (TRUE,ClipFrustum.planes,ClipFrustum.p_count);

			//	Transform frustum to clip space
			Fmatrix PlaneTransform;
			PlaneTransform.transpose(Device.mInvFullTransform);
			//HW.pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0x3F);

			for ( int i=0; i<6; ++i)
			{
				Fvector4	&ClipPlane = *(Fvector4*)&ClipFrustum.planes[i].n.x;
				Fvector4	TransformedPlane;
				PlaneTransform.transform(TransformedPlane, ClipPlane);
				TransformedPlane.mul(-1.0f);
				RCache.set_ca(strFrustumClipPlane, i, TransformedPlane);
				//HW.pDevice->SetClipPlane( i, &TransformedPlane.x);
			}
			
		}
		


		RCache.set_ColorWriteEnable(D3DCOLORWRITEENABLE_RED|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_BLUE);

		RCache.set_Geometry(g_accum_volumetric);
		//	Igor: no need to do it per sub-sample. Plain AA will go just fine.
		RCache.Render(D3DPT_TRIANGLELIST,0,0,VOLUMETRIC_SLICES*4,0,VOLUMETRIC_SLICES*2);
		


		RCache.set_ColorWriteEnable();

		//	Restore clip planes
		//HW.pDevice->SetRenderState(D3DRS_CLIPPLANEENABLE, 0);
		RCache.set_ClipPlanes (FALSE,(Fmatrix *)0,0);
	}

	//CHK_DX		(HW.pDevice->SetRenderState(D3DRS_SCISSORTESTENABLE,FALSE));
	RCache.set_Scissor(0);
}


void CRenderTarget::accum_spot_reflective(light* L)
{
#pragma todo("Implement spot RSM")

	if (!need_to_render_spot_il(L->color))
		return;

	phase_rsm_accumulator();
	RImplementation.stats.l_visible	++;

	// *** assume accumulator already setup ***
	// *****************************	Mask by stencil		*************************************

	BOOL	bIntersect			= FALSE; //enable_scissor(L);
	{
		// hack: draw larger radius
		//L->range *= 5;

		// setup xform
		L->xform_calc					();
		RCache.set_xform_world			(L->m_xform			);
		RCache.set_xform_view			(Device.mView		);
		RCache.set_xform_project		(Device.mProject	);
		bIntersect						= enable_scissor	(L);
		enable_dbt_bounds				(L);

		// *** similar to "Carmack's reverse", but assumes convex, non intersecting objects,
		// *** thus can cope without stencil clear with 127 lights
		// *** in practice, 'cause we "clear" it back to 0x1 it usually allows us to > 200 lights :)
		//	Done in blender!
		//RCache.set_ColorWriteEnable		(FALSE);
		RCache.set_Element		(s_accum_mask->E[SE_MASK_SPOT]);		// masker

		// backfaces: if (stencil>=1 && zfail)			stencil = light_id
		RCache.set_CullMode		(D3D_CULL_FRONT);
		RCache.set_Stencil(TRUE, D3D_COMPARISON_LESS_EQUAL, dwLightMarkerID, 0x01, 0xff, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE);
		draw_volume					(L);

		// frontfaces: if (stencil>=light_id && zfail)	stencil = 0x1
		RCache.set_CullMode		(D3D_CULL_BACK);
		RCache.set_Stencil(TRUE, D3D_COMPARISON_LESS_EQUAL, 0x01, 0xff, 0xff, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE);
		draw_volume					(L);
	}
	
	// nv-stencil recompression
	if (RImplementation.o.nvstencil)	u_stencil_optimize();

	// *****************************	Minimize overdraw	*************************************
	// Select shader (front or back-faces), *** back, if intersect near plane
	RCache.set_ColorWriteEnable		();
	RCache.set_CullMode				(D3D_CULL_FRONT);		// back

	// 2D texgens 
	Fmatrix			m_Texgen;			u_compute_texgen_screen	(m_Texgen	);
	Fmatrix			m_Texgen_J;			u_compute_texgen_jitter	(m_Texgen_J	);

	// Shadow xform (+texture adjustment matrix)
	Fmatrix			m_Shadow,m_Lmap;
	{
		float			smapsize			= float(RImplementation.o.smapsize);
		float			fTexelOffs			= (.5f / smapsize);
		float			view_dim			= float(L->X.S.size-2)/smapsize;
		float			view_sx				= float(L->X.S.posX+1)/smapsize;
		float			view_sy				= float(L->X.S.posY+1)/smapsize;
		float			fRange				= float(1.f)* r__ls_depth_scale;
		float			fBias				= r__ls_depth_bias;
		Fmatrix			m_TexelAdjust		= {
			view_dim/2.f,							0.0f,									0.0f,		0.0f,
			0.0f,									-view_dim/2.f,							0.0f,		0.0f,
			0.0f,									0.0f,									fRange,		0.0f,
			view_dim/2.f + view_sx + fTexelOffs,	view_dim/2.f + view_sy + fTexelOffs,	fBias,		1.0f
		};

		// compute xforms
		Fmatrix			xf_world;		xf_world.invert	(Device.mView);
		Fmatrix			xf_view			= L->X.S.view;
		Fmatrix			xf_project;		xf_project.mul	(m_TexelAdjust,L->X.S.project);
		m_Shadow.mul					(xf_view, xf_world);
		m_Shadow.mulA_44				(xf_project	);

		// lmap
						view_dim			= 1.f;
						view_sx				= 0.f;
						view_sy				= 0.f;
		Fmatrix			m_TexelAdjust2		= {
			view_dim/2.f,							0.0f,									0.0f,		0.0f,
			0.0f,									-view_dim/2.f,							0.0f,		0.0f,
			0.0f,									0.0f,									fRange,		0.0f,
			view_dim/2.f + view_sx + fTexelOffs,	view_dim/2.f + view_sy + fTexelOffs,	fBias,		1.0f
		};

		// compute xforms
		xf_project.mul		(m_TexelAdjust2,L->X.S.project);
		m_Lmap.mul			(xf_view, xf_world);
		m_Lmap.mulA_44		(xf_project	);
	}

	// Common constants
	Fvector		L_dir,L_clr,L_pos;	float L_spec;
	L_clr.set					(L->color.r,L->color.g,L->color.b);
	L_clr.mul					(L->get_LOD());
	L_spec						= u_diffuse2s	(L_clr);
	Device.mView.transform_tiny	(L_pos,L->position);
	Device.mView.transform_dir	(L_dir,L->direction);
	L_dir.normalize				();

	// Draw SQ
	{
		//u32 bias = 0;
		//prepare_sq_vertex(bias, g_simple_quad);

		if (!L->flags.bShadow)
			m_Shadow = m_Lmap;

		RCache.set_Element(s_rsm->E[SE_RSM_SPOT]);

		RCache.set_CullMode(D3D_CULL_FRONT);		// back

		// Constants
		float	att_R = L->range * .95f;
		float	att_factor = 1.f / (att_R * att_R);

		RCache.set_c("Ldynamic_pos", L_pos.x, L_pos.y, L_pos.z, att_factor);
		RCache.set_c("Ldynamic_color", L_clr.x, L_clr.y, L_clr.z, L_spec);

		RCache.set_c("m_texgen", m_Texgen);
		RCache.set_c("m_texgen_J", m_Texgen_J);
		RCache.set_c("m_shadow", m_Shadow);

		RCache.set_c("c_rangle", L->range, 0, 0, 0);

		RCache.set_ca("m_lmap", 0, m_Lmap._11, m_Lmap._21, m_Lmap._31, m_Lmap._41);
		RCache.set_ca("m_lmap", 1, m_Lmap._12, m_Lmap._22, m_Lmap._32, m_Lmap._42);

		RCache.set_c("c_rsm_generate_params_0", r__sun_il_params_0);
		RCache.set_c("c_rsm_generate_params_1", r__sun_il_params_1);
		RCache.set_c("c_rsm_generate_params_2", r__sun_il_params_2);

		RCache.set_Stencil(TRUE, D3D_COMPARISON_LESS_EQUAL, dwLightMarkerID, 0xff, 0x00);
		//RCache.set_Stencil(FALSE);

		draw_volume(L);  

		//RCache.set_Geometry(g_simple_quad);
		//RCache.Render(D3DPT_TRIANGLELIST, bias, 0, 4, 0, 2);
	}

	RCache.set_Scissor(0);

	increment_light_marker();

	u_DBT_disable				();
}
