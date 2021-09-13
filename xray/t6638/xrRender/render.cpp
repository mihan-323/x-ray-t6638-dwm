#include "stdafx.h"
#include "render.h"
#include "fbasicvisual.h"
#include "../xrEngine/xr_object.h"
#include "../xrEngine/CustomHUD.h"
#include "../xrEngine/igame_persistent.h"
#include "../xrEngine/environment.h"
#include "SkeletonCustom.h"
#include "LightTrack.h"
#include "dxRenderDeviceRender.h"
#include "dxWallMarkArray.h"
#include "dxUIShader.h"
#include "ShaderResourceTraits.h"

#include "3DFluidManager.h"
#include "D3DX11Core.h"

CRender RImplementation;

void cl_hud_fov_mask		::setup(R_constant* C) { RCache.set_c(C, RImplementation.m_hud_fov_mask					); }
void cl_planar_env			::setup(R_constant* C) { RCache.set_c(C, RImplementation.m_planar_env					); }
void cl_planar_amb			::setup(R_constant* C) { RCache.set_c(C, RImplementation.m_planar_amb					); }
void cl_planar_bias			::setup(R_constant* C) { RCache.set_c(C, RImplementation.m_planar_bias					); }
void cl_planar_shadow		::setup(R_constant* C) { RCache.set_c(C, RImplementation.m_planar_shadow				); }
void cl_planar_vp_camera	::setup(R_constant* C) { RCache.set_c(C, RImplementation.m_planar_vp_camera				); }

//////////////////////////////////////////////////////////////////////////
class CGlow				: public IRender_Glow
{
public:
	bool				bActive;
public:
	CGlow() : bActive(false)		{ }
	virtual void					set_active			(bool b)					{ bActive=b;		}
	virtual bool					get_active			()							{ return bActive;	}
	virtual void					set_position		(const Fvector& P)			{ }
	virtual void					set_direction		(const Fvector& D)			{ }
	virtual void					set_radius			(float R)					{ }
	virtual void					set_texture			(LPCSTR name)				{ }
	virtual void					set_color			(const Fcolor& C)			{ }
	virtual void					set_color			(float r, float g, float b)	{ }
};

float		r_dtex_range		= 50.f;

//////////////////////////////////////////////////////////////////////////

ShaderElement*			CRender::rimp_select_sh	(dxRender_Visual *pVisual, float cdist_sq)
{
	u32 id;

	bool hq = _sqrt(cdist_sq) - pVisual->vis.sphere.R < r_dtex_range;

	switch (RImplementation.phase)
	{
	case PHASE_NORMAL:		// deffer
		id = hq ? SE_NORMAL_HQ : SE_NORMAL_LQ;
		break;
	case PHASE_SMAP:		// deffer
		id = SE_SHADOW;
		break;
	case PHASE_ZPREPASS:	// deffer
		id = SE_ZPREPASS;
		break;
	case PHASE_RSMAP:		// deffer
		id = SE_RSM_FILL_RTS;
		break;
	case PHASE_PLANAR:		// forward
		id = SE_PLANAR;
		break;
	default:
		id = SE_UNKNOWN;
		Log("! invalid render phase, set SE_UNKNOWN by default");
	}

	return pVisual->shader->E[id]._get();
}

static class cl_LOD		: public R_constant_setup
{
	virtual void setup	(R_constant* C)
	{
		RCache.LOD.set_LOD(C);
	}
} binder_LOD;

static class cl_pos_decompress_params		: public R_constant_setup		{	virtual void setup	(R_constant* C)
{
	float VertTan =  -1.0f * tanf( deg2rad(float(Device.fFOV) /2.0f ) );
	float HorzTan =  - VertTan / Device.fASPECT;

	RCache.set_c(C, HorzTan, VertTan, (2.0f * HorzTan) / RImplementation.fWidth, (2.0f * VertTan) / RImplementation.fHeight);

}}	binder_pos_decompress_params;

static class cl_screen_res : public R_constant_setup		{	virtual void setup	(R_constant* C)
{
	RCache.set_c(C, RImplementation.fWidth, RImplementation.fHeight, 1.0f / RImplementation.fWidth, 1.0f / RImplementation.fHeight);

}}	binder_screen_res;

static class cl_sun_shafts_intensity : public R_constant_setup		
{	
	virtual void setup	(R_constant* C)
	{
		CEnvDescriptor&	E = *g_pGamePersistent->Environment().CurrentEnv;
		float fValue = E.m_fSunShaftsIntensity;
		RCache.set_c	(C, fValue, fValue, fValue, 0);
	}
}	binder_sun_shafts_intensity;

//////////////////////////////////////////////////////////////////////////
// Just two static storage
void CRender::set_create_options()
{
	m_need_adv_cache = TRUE;
	m_need_warnings = FALSE;
	m_need_recomp = FALSE;

	// smap
	u32 smap_size_d = 512;
	o.smapsize = is_sun_static() ? smap_size_d : r__smap_size;

	//	For ATI it's much faster on DX11 to use D32F format
	o.smap_format = HW.Caps.id_vendor == 0x1002 ? DXGI_FORMAT_D32_FLOAT : DXGI_FORMAT_D24_UNORM_S8_UINT;

	o.nvstencil = opt(R__NEED_NVDSTENCIL) && HW.Caps.id_vendor == 0x10DE && HW.Caps.id_device >= 0x40;
	o.nvdbt = opt(R__USE_NVDBT);
	o.tshadows = opt(R__USE_TRANS_SHADOWS);
	o.forceskinw = !!strstr(Core.Params, "-skinw");

	o.tessellation = opt(R__NEED_TESSELATION) && HW.FeatureLevel >= D3D_FEATURE_LEVEL_11_0;

	o.wet_surfaces = opt(R__USE_WET_SURFACES);

	o.ssaa = r__ssaa;

	o.advanced_mode = opt(R__USE_ADVANCED_MODE);

	if (o.advanced_mode)
	{
		switch (r__aa)
		{
		case FALSE:			o.aa_mode = FALSE;		o.msaa_samples = 1; o.txaa = FALSE;	break;
		case AA_MLAA:		o.aa_mode = AA_MLAA;	o.msaa_samples = 1; o.txaa = FALSE;	break;
		case AA_FXAA:		o.aa_mode = AA_FXAA;	o.msaa_samples = 1; o.txaa = FALSE;	break;
		case AA_MSAA_FXAA:	o.aa_mode = AA_MSAA;	o.msaa_samples = 2; o.txaa = FALSE;	break;
		case AA_MSAA2S:		o.aa_mode = AA_MSAA;	o.msaa_samples = 2; o.txaa = FALSE;	break;
		case AA_MSAA4S:		o.aa_mode = AA_MSAA;	o.msaa_samples = 4; o.txaa = FALSE;	break;
		case AA_MSAA8S:		o.aa_mode = AA_MSAA;	o.msaa_samples = 8; o.txaa = FALSE;	break;
		case AA_TAA:		o.aa_mode = AA_TAA;		o.msaa_samples = 1; o.txaa = FALSE;	break;
		case AA_TAA_V2:		o.aa_mode = AA_TAA_V2;	o.msaa_samples = 1; o.txaa = FALSE;	break;
		case AA_TXAA:		o.aa_mode = AA_TXAA;	o.msaa_samples = 1; o.txaa = TRUE;	break;
		case AA_TXAA2S:		o.aa_mode = AA_MSAA;	o.msaa_samples = 2; o.txaa = TRUE;	break;
		case AA_TXAA4S:		o.aa_mode = AA_MSAA;	o.msaa_samples = 4; o.txaa = TRUE;	break;
		}

		switch (r__aa)
		{
		case FALSE:			Msg("* Antialiasing type: disabled");		break;
		case AA_MLAA:		Msg("* Antialiasing type: MLAA");			break;
		case AA_FXAA:		Msg("* Antialiasing type: FXAA");			break;
		case AA_MSAA_FXAA:	Msg("* Antialiasing type: MSAA + FXAA");	break;
		case AA_MSAA2S:		Msg("* Antialiasing type: MSAA 2X");		break;
		case AA_MSAA4S:		Msg("* Antialiasing type: MSAA 4X");		break;
		case AA_MSAA8S:		Msg("* Antialiasing type: MSAA 8X");		break;
		case AA_TAA:		Msg("* Antialiasing type: TAA");			break;
		case AA_TAA_V2:		Msg("* Antialiasing type: TAA V2");			break;
		case AA_TXAA:		Msg("* Antialiasing type: TXAA");			break;
		case AA_TXAA2S:		Msg("* Antialiasing type: TXAA 2X");		break;
		case AA_TXAA4S:		Msg("* Antialiasing type: TXAA 4X");		break;
		}
		
		o.sm_minmax = opt(R__NEED_MINMAX_SM);
		if (o.sm_minmax)
		{
			if (HW.Caps.id_vendor == 0x1002)
				o.sm_minmax = MMSM_AUTO;
			else
				o.sm_minmax = MMSM_OFF;
		}

		o.msaa_samples_reflections = r__msaa_reflections;
		o.volumetricfog = opt(R__USE_VSMOKE);
		o.volumetriclight = opt(R__USE_VLIGHT);
		o.hud_shadows = opt(R__USE_HUD_SHADOWS);
		o.cspecular = opt(R__USE_CSPECULAR);
		o.sunshafts = r__sunshafts_mode > 0;
		o.ssao = r__ssao_mode;
		o.ssr = opt(R__USE_SSR);
		o.ssr_replace = opt(R__REP_SSR_TO_PLANAR);
		o.sun_il = opt(R__USE_SUN_IL);
		o.spot_il = opt(R__USE_SPOT_IL);
		o.planar = opt(R__USE_PLANAR);
		o.gbd_opt = opt(R__USE_GBD_OPT);
		o.pt_downsample = opt(R__PT_DOWNSAMPLE);
	}
	else
	{
		o.aa_mode = FALSE;
		o.sm_minmax = FALSE;
		o.volumetricfog = FALSE;
		o.volumetriclight = FALSE;
		o.hud_shadows = FALSE;
		o.cspecular = FALSE;
		o.sunshafts = FALSE;
		o.ssao = FALSE;
		o.ssr = FALSE;
		o.ssr_replace = FALSE;
		o.sun_il = FALSE;
		o.spot_il = FALSE;
		o.planar = FALSE;
		o.gbd_opt = FALSE;
		o.msaa_samples = 1;
		o.msaa_samples_reflections = 1;
		o.txaa = FALSE;
		o.pt_downsample = FALSE;
	}

	if (o.planar)
	{
		o.ssr = FALSE;
		o.ssr_replace = FALSE;
	}

	if (
		o.sun_il ||
		o.spot_il ||
		o.ssao == SSAO_PATH_TRACING
		)
	{
		Log("* Keep G_Buffer of the previous frame: enabled");
		o.gbd_save = TRUE;
	}
	else
	{
		Log("* Keep G_Buffer of the previous frame: disabled");
		o.gbd_save = FALSE;
	}

	o.sm_minmax_area_thold = 1600 * 1200;

	// recompile all shaders
	if (strstr(Core.Params, "-noscache"))
	{
		m_need_adv_cache = FALSE;
		m_need_warnings = FALSE;
		m_need_recomp = TRUE;
	}

	// if "-warnings" is on "-noscache" must be enabled
	if (strstr(Core.Params, "-warnings"))
	{
		m_need_adv_cache = FALSE;
		m_need_warnings = TRUE;
		m_need_recomp = TRUE;
	}

	// distortion
	{
		o.distortion = TRUE;
	}
}

bool					CRender::is_sun_static()
{
	extern	ENGINE_API	u32		renderer_value;

	return renderer_value == R_R4A;
}

void					CRender::create					()
{
	Device.seqFrame.Add	(this,REG_PRIORITY_HIGH+0x12345678);

	// init verify
	{
		BOOL mrt = (HW.Caps.raster.dwMRT_count >= 3);
		VERIFY2(mrt && (HW.Caps.raster.dwInstructions >= 256), "Hardware doesn't meet minimum feature-level");
	}

	m_skinning = -1;
	m_MSAASample = -1;

	fWidth = (float)Device.dwWidth;
	fHeight = (float)Device.dwHeight;

	set_create_options();

	if (o.txaa == TRUE)
	{
		if (HW.m_TXAA_initialized)
		{
			//Log("* TXAA supported and used");
		}
		else
		{
			o.txaa = FALSE;

			// TXAA 2X, 4X, replace with MSAA
			//if (o.aa_mode == AA_MSAA)
			//{
				//Log("! TXAA doesn't supported ... Enabled MSAA 2X/4X");
				//o.aa_mode = AA_MSAA;
				//o.msaa_samples = o.msaa_samples;
			//}

			// TXAA 1X, replace with TAA
			//if (o.aa_mode == AA_TXAA)
			{
				Log("! TXAA doesn't supported ... Enabled TAA");
				o.aa_mode = AA_TAA_V2;
				o.msaa_samples = 1; 
			}
		}
	}
	
	if (o.ssao == SSAO_HBAO_PLUS)
	{
		if (HW.FeatureLevel >= D3D_FEATURE_LEVEL_11_0)
		{
			//Log("* HBAO+ supported and used");
		}
		else
		{
			Log("! HBAO+ doesn't supported ... Enabled simple SSAO");
			o.ssao = SSAO_SSAO;
		}
	}

	// Turn off SSPR if CS isn't support
	if (!HW.m_cs_support)
	{
		o.ssr_replace = FALSE;
	}

	// constants
	dxRenderDeviceRender::Instance().Resources->RegisterConstantSetup("sun_shafts_intensity",		&binder_sun_shafts_intensity);
	dxRenderDeviceRender::Instance().Resources->RegisterConstantSetup("pos_decompression_params",	&binder_pos_decompress_params);
	dxRenderDeviceRender::Instance().Resources->RegisterConstantSetup("screen_res",					&binder_screen_res);
	dxRenderDeviceRender::Instance().Resources->RegisterConstantSetup("triLOD",						&binder_LOD);

	c_lmaterial					= "L_material";
	c_sbase						= "s_base";

	m_bMakeAsyncSS				= false;

	Target						= xr_new<CRenderTarget>		();	// Main target

	Models						= xr_new<CModelPool>		();
	PSLibrary.OnCreate			();
	HWOCC.occq_create			(occq_size);

	Target->enable_SSAA();
	rmNormal					();
	marker						= 0;

	D3D_QUERY_DESC			qdesc;
	qdesc.MiscFlags				= 0;
	qdesc.Query					= D3D_QUERY_EVENT;

	ZeroMemory(q_sync_point, sizeof(q_sync_point));

	for (u32 i=0; i<HW.Caps.iGPUNum; ++i)
		R_CHK(HW.pDevice->CreateQuery(&qdesc, &q_sync_point[i]));

	HW.pContext->End(q_sync_point[0]);

	xrRender_apply_tf			();
	::PortalTraverser.initialize();

	FluidManager.Initialize( 70, 70, 70 );
	FluidManager.SetScreenSize((int)fWidth, (int)fHeight);
}

void					CRender::destroy				()
{
	m_bMakeAsyncSS				= false;

	FluidManager.Destroy();

	::PortalTraverser.destroy	();

	for (u32 i=0; i<HW.Caps.iGPUNum; ++i)
		_RELEASE				(q_sync_point[i]);
	
	HWOCC.occq_destroy			();
	xr_delete					(Models);
	xr_delete					(Target);
	PSLibrary.OnDestroy			();
	Device.seqFrame.Remove		(this);
	r_dsgraph_destroy			();
}

void CRender::reset_begin()
{
	// Update incremental shadowmap-visibility solver
	// BUG-ID: 10646
	{
		u32 it=0;
		for (it=0; it<Lights_LastFrame.size(); it++)	{
			if (0==Lights_LastFrame[it])	continue	;
			try {
				Lights_LastFrame[it]->svis.resetoccq ()	;
			} catch (...)
			{
				Msg	("! Failed to flush-OCCq on light [%d] %X",it,*(u32*)(&Lights_LastFrame[it]));
			}
		}
		Lights_LastFrame.clear	();
	}

	xr_delete					(Target);
	HWOCC.occq_destroy			();
	//_RELEASE					(q_sync_point[1]);
	//_RELEASE					(q_sync_point[0]);
	for (u32 i=0; i<HW.Caps.iGPUNum; ++i)
		_RELEASE				(q_sync_point[i]);
}

void CRender::reset_end()
{
	D3D_QUERY_DESC			qdesc;
	qdesc.MiscFlags				= 0;
	qdesc.Query					= D3D_QUERY_EVENT;

	for (u32 i=0; i<HW.Caps.iGPUNum; ++i)
		R_CHK(HW.pDevice->CreateQuery(&qdesc,&q_sync_point[i]));

	//	Prevent error on first get data
	HW.pContext->End(q_sync_point[0]);

	HWOCC.occq_create			(occq_size);

	Target						=	xr_new<CRenderTarget>	();

	xrRender_apply_tf			();

	FluidManager.SetScreenSize((int)fWidth, (int)fHeight);

	// clear shader errors vector
	if (m_need_warnings)
		for(u32 i = 0; i < m_vecShaderErrorBuf.size(); i++)
			m_vecShaderErrorBuf.pop_back();

	// Set this flag true to skip the first render frame,
	// that some data is not ready in the first frame (for example device camera position)
	m_bFirstFrameAfterReset		= true;
}

void CRender::OnFrame()
{
	Models->DeleteQueue			();

	// MT-details (@front)
	Device.seqParallel.insert	(Device.seqParallel.begin(),
		fastdelegate::FastDelegate0<>(Details,&CDetailManager::MT_CALC));

	// MT-HOM (@front)
	Device.seqParallel.insert	(Device.seqParallel.begin(),
		fastdelegate::FastDelegate0<>(&HOM,&CHOM::MT_RENDER));
}


// Implementation
IRender_ObjectSpecific*	CRender::ros_create				(IRenderable* parent)				{ return xr_new<CROS_impl>();			}
void					CRender::ros_destroy			(IRender_ObjectSpecific* &p)		{ xr_delete(p);							}
IRenderVisual*			CRender::model_Create			(LPCSTR name, IReader* data)		{ return Models->Create(name,data);		}
IRenderVisual*			CRender::model_CreateChild		(LPCSTR name, IReader* data)		{ return Models->CreateChild(name,data);}
IRenderVisual*			CRender::model_Duplicate		(IRenderVisual* V)					{ return Models->Instance_Duplicate((dxRender_Visual*)V);	}
void					CRender::model_Delete			(IRenderVisual* &V, BOOL bDiscard)	
{ 
	dxRender_Visual* pVisual = (dxRender_Visual*)V;
	Models->Delete(pVisual, bDiscard);
	V = 0;
}
IRender_DetailModel*	CRender::model_CreateDM			(IReader*	F)
{
	CDetail*	D		= xr_new<CDetail> ();
	D->Load				(F);
	return D;
}
void					CRender::model_Delete			(IRender_DetailModel* & F)
{
	if (F)
	{
		CDetail*	D	= (CDetail*)F;
		D->Unload		();
		xr_delete		(D);
		F				= NULL;
	}
}
IRenderVisual*			CRender::model_CreatePE			(LPCSTR name)	
{ 
	PS::CPEDef*	SE			= PSLibrary.FindPED	(name);		R_ASSERT3(SE,"Particle effect doesn't exist",name);
	return					Models->CreatePE	(SE);
}
IRenderVisual*			CRender::model_CreateParticles	(LPCSTR name)	
{ 
	PS::CPEDef*	SE			= PSLibrary.FindPED	(name);
	if (SE) return			Models->CreatePE	(SE);
	else{
		PS::CPGDef*	SG		= PSLibrary.FindPGD	(name);		R_ASSERT3(SG,"Particle effect or group doesn't exist",name);
		return				Models->CreatePG	(SG);
	}
}
void					CRender::models_Prefetch		()					{ Models->Prefetch	();}
void					CRender::models_Clear			(BOOL b_complete)	{ Models->ClearPool	(b_complete);}

ref_shader				CRender::getShader				(int id)			{ VERIFY(id<int(Shaders.size()));	return Shaders[id];	}
IRender_Portal*			CRender::getPortal				(int id)			{ VERIFY(id<int(Portals.size()));	return Portals[id];	}
IRender_Sector*			CRender::getSector				(int id)			{ VERIFY(id<int(Sectors.size()));	return Sectors[id];	}
IRender_Sector*			CRender::getSectorActive		()					{ return pLastSector;									}
IRenderVisual*			CRender::getVisual				(int id)			{ VERIFY(id<int(Visuals.size()));	return Visuals[id];	}
D3DVERTEXELEMENT9*		CRender::getVB_Format			(int id, BOOL	_alt)	{ 
	if (_alt)	{ VERIFY(id<int(xDC.size()));	return xDC[id].begin();	}
	else		{ VERIFY(id<int(nDC.size()));	return nDC[id].begin(); }
}
ID3DBuffer*	CRender::getVB					(int id, BOOL	_alt)	{
	if (_alt)	{ VERIFY(id<int(xVB.size()));	return xVB[id];		}
	else		{ VERIFY(id<int(nVB.size()));	return nVB[id];		}
}
ID3DBuffer*	CRender::getIB					(int id, BOOL	_alt)	{
	if (_alt)	{ VERIFY(id<int(xIB.size()));	return xIB[id];		}
	else		{ VERIFY(id<int(nIB.size()));	return nIB[id];		}
}
FSlideWindowItem*		CRender::getSWI					(int id)			{ VERIFY(id<int(SWIs.size()));		return &SWIs[id];	}
IRender_Target*			CRender::getTarget				()					{ return Target;										}

IRender_Light*			CRender::light_create			()					{ return Lights.Create();								}
IRender_Glow*			CRender::glow_create			()					{ return xr_new<CGlow>();								}

void					CRender::flush					()					{ r_dsgraph_render_graph	(0);						}

BOOL					CRender::occ_visible			(vis_data& P)		{ return HOM.visible(P);								}
BOOL					CRender::occ_visible			(sPoly& P)			{ return HOM.visible(P);								}
BOOL					CRender::occ_visible			(Fbox& P)			{ return HOM.visible(P);								}

void					CRender::add_Visual				(IRenderVisual*		V )	{ add_leafs_Dynamic((dxRender_Visual*)V);								}
void					CRender::add_Geometry			(IRenderVisual*		V )	{ add_Static((dxRender_Visual*)V,View->getMask());					}
void					CRender::add_StaticWallmark		(ref_shader& S, const Fvector& P, float s, CDB::TRI* T, Fvector* verts)
{
	if (T->suppress_wm)	return;
	VERIFY2							(_valid(P) && _valid(s) && T && verts && (s>EPS_L), "Invalid static wallmark params");
	Wallmarks->AddStaticWallmark	(T,verts,P,&*S,s);
}

void CRender::add_StaticWallmark			(IWallMarkArray *pArray, const Fvector& P, float s, CDB::TRI* T, Fvector* V)
{
	dxWallMarkArray *pWMA = (dxWallMarkArray *)pArray;
	ref_shader *pShader = pWMA->dxGenerateWallmark();
	if (pShader) add_StaticWallmark		(*pShader, P, s, T, V);
}

void CRender::add_StaticWallmark			(const wm_shader& S, const Fvector& P, float s, CDB::TRI* T, Fvector* V)
{
	dxUIShader* pShader = (dxUIShader*)&*S;
	add_StaticWallmark		(pShader->hShader, P, s, T, V);
}

void					CRender::clear_static_wallmarks	()
{
	Wallmarks->clear				();
}

void CRender::add_SkeletonWallmark	(intrusive_ptr<CSkeletonWallmark> wm)
{
	Wallmarks->AddSkeletonWallmark				(wm);
}

void CRender::add_SkeletonWallmark	(const Fmatrix* xf, CKinematics* obj, ref_shader& sh, const Fvector& start, const Fvector& dir, float size)
{
	Wallmarks->AddSkeletonWallmark				(xf, obj, sh, start, dir, size);
}

void CRender::add_SkeletonWallmark	(const Fmatrix* xf, IKinematics* obj, IWallMarkArray *pArray, const Fvector& start, const Fvector& dir, float size)
{
	dxWallMarkArray *pWMA = (dxWallMarkArray *)pArray;
	ref_shader *pShader = pWMA->dxGenerateWallmark();
	if (pShader) add_SkeletonWallmark(xf, (CKinematics*)obj, *pShader, start, dir, size);
}

void					CRender::set_TorchEnabled(bool b	)
{ 
	m_torch_enabled = b ? TRUE : FALSE; 
}
void					CRender::add_Occluder			(Fbox2&	bb_screenspace	)
{
	HOM.occlude			(bb_screenspace);
}
void					CRender::set_Object				(IRenderable*	O )	
{ 
	val_pObject				= O;
}
void					CRender::rmNear				()
{
	IRender_Target* T	=	getTarget	();
	D3D_VIEWPORT VP		=	{0,0,(float)T->get_width(),(float)T->get_height(),0,0.02f };

	RCache.set_Viewport(&VP);
}
void					CRender::rmFar				()
{
	IRender_Target* T	=	getTarget	();
	D3D_VIEWPORT VP		=	{0,0,(float)T->get_width(),(float)T->get_height(),0.99999f,1.f };

	RCache.set_Viewport(&VP);
}
void					CRender::rmNormal			()
{
	IRender_Target* T	=	getTarget	();
	D3D_VIEWPORT VP		= {0,0,(float)T->get_width(),(float)T->get_height(),0,1.f };

	RCache.set_Viewport(&VP);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CRender::CRender()
:m_bFirstFrameAfterReset(false)
{
	//m_torch_enabled = FALSE; // first frame torch light disabled
	m_hud_fov_mask = FALSE;
	//m_planar_env.set(0, 0, 0, 0);
	//m_planar_amb.set(0, 0, 0, 0);
	//m_planar_bias.set(0, 0, 0, 0);

	init_cacades();
}

CRender::~CRender()
{
}

#include "../xrEngine/GameFont.h"
void	CRender::Statistics	(CGameFont* _F)
{
	CGameFont&	F	= *_F;
	F.OutNext	(" **** LT:%2d,LV:%2d **** ",stats.l_total,stats.l_visible		);	stats.l_visible = 0;
	F.OutNext	("    S(%2d)   | (%2d)NS   ",stats.l_shadowed,stats.l_unshadowed);
	F.OutNext	("smap use[%2d], merge[%2d], finalclip[%2d]",stats.s_used,stats.s_merged-stats.s_used,stats.s_finalclip);
	stats.s_used = 0; stats.s_merged = 0; stats.s_finalclip = 0;
	F.OutSkip	();
	F.OutNext	(" **** Occ-Q(%03.1f) **** ",100.f*f32(stats.o_culled)/f32(stats.o_queries?stats.o_queries:1));
	F.OutNext	(" total  : %2d",	stats.o_queries	);	stats.o_queries = 0;
	F.OutNext	(" culled : %2d",	stats.o_culled	);	stats.o_culled	= 0;
	F.OutSkip	();
	u32	ict		= stats.ic_total + stats.ic_culled;
	F.OutNext	(" **** iCULL(%03.1f) **** ",100.f*f32(stats.ic_culled)/f32(ict?ict:1));
	F.OutNext	(" visible: %2d",	stats.ic_total	);	stats.ic_total	= 0;
	F.OutNext	(" culled : %2d",	stats.ic_culled	);	stats.ic_culled	= 0;
#ifdef DEBUG
	HOM.stats	();
#endif
}

/////////
#pragma comment(lib,"d3dx9.lib")

void CRender::addShaderOption(const char* name, const char* value)
{
	D3D_SHADER_MACRO macro = {name, value};
	m_ShaderOptions.push_back(macro);
}

template <typename T>
static HRESULT create_shader(
	LPCSTR const	pTarget,
	DWORD const* buffer,
	u32	const		buffer_size,
	LPCSTR const	file_name,
	T*& result,
	bool const		disasm
)
{
	result->sh = ShaderTypeTraits<T>::CreateHWShader(buffer, buffer_size);

	ID3DShaderReflection* pReflection = 0;

	HRESULT const _hr = D3DReflect(buffer, buffer_size, IID_ID3DShaderReflection, (void**)&pReflection);
	if (SUCCEEDED(_hr) && pReflection)
	{
		// Parse constant table data
		result->constants.parse(pReflection, ShaderTypeTraits<T>::GetShaderDest());

		_RELEASE(pReflection);
	}
	else
	{
		Msg("! D3DReflectShader %s hr == 0x%08x", file_name, _hr);
	}

	return				_hr;
}

static HRESULT create_shader				(
		LPCSTR const	pTarget,
		DWORD const*	buffer,
		u32	const		buffer_size,
		LPCSTR const	file_name,
		void*&			result,
		bool const		disasm
	)
{
	HRESULT		_result = E_FAIL;
	if (pTarget[0] == 'p') {
		SPS* sps_result = (SPS*)result;
		_result			= HW.pDevice->CreatePixelShader(buffer, buffer_size, 0, &sps_result->ps);
		if ( !SUCCEEDED(_result) ) {
			Log			("! PS: ", file_name);
			Msg			("! CreatePixelShader hr == 0x%08x", _result);
			return		E_FAIL;
		}

		ID3DShaderReflection *pReflection = 0;

		_result			= D3DReflect( buffer, buffer_size, IID_ID3DShaderReflection, (void**)&pReflection);

		//	Parse constant, texture, sampler binding
		//	Store input signature blob
		if (SUCCEEDED(_result) && pReflection)
		{
			//	Let constant table parse it's data
			sps_result->constants.parse(pReflection,RC_dest_pixel);

			_RELEASE(pReflection);
		}
		else
		{
			Log	("! PS: ", file_name);
			Msg	("! D3DReflectShader hr == 0x%08x", _result);
		}
	}
	else if (pTarget[0] == 'v') {
		SVS* svs_result = (SVS*)result;
		_result			= HW.pDevice->CreateVertexShader(buffer, buffer_size, 0, &svs_result->vs);

		if ( !SUCCEEDED(_result) ) {
			Log			("! VS: ", file_name);
			Msg			("! CreatePixelShader hr == 0x%08x", _result);
			return		E_FAIL;
		}

		ID3DShaderReflection *pReflection = 0;

		_result			= D3DReflect( buffer, buffer_size, IID_ID3DShaderReflection, (void**)&pReflection);
		
		//	Parse constant, texture, sampler binding
		//	Store input signature blob
		if (SUCCEEDED(_result) && pReflection)
		{
			//	TODO: DX10: share the same input signatures

			//	Store input signature (need only for VS)
			//CHK_DX( D3DxxGetInputSignatureBlob(pShaderBuf->GetBufferPointer(), pShaderBuf->GetBufferSize(), &_vs->signature) );
			ID3DBlob*	pSignatureBlob;
			CHK_DX		( D3DGetInputSignatureBlob(buffer, buffer_size, &pSignatureBlob) );
			VERIFY		(pSignatureBlob);

			svs_result->signature = dxRenderDeviceRender::Instance().Resources->_CreateInputSignature(pSignatureBlob);

			_RELEASE	(pSignatureBlob);

			//	Let constant table parse it's data
			svs_result->constants.parse(pReflection,RC_dest_vertex);

			_RELEASE	(pReflection);
		}
		else
		{
			Log			("! VS: ", file_name);
			Msg			("! D3DXFindShaderComment hr == 0x%08x", _result);
		}
	}
	else if (pTarget[0] == 'g') {
		SGS* sgs_result = (SGS*)result;
		_result			= HW.pDevice->CreateGeometryShader(buffer, buffer_size, 0, &sgs_result->gs);
		if ( !SUCCEEDED(_result) ) {
			Log			("! GS: ", file_name);
			Msg			("! CreateGeometryShader hr == 0x%08x", _result);
			return		E_FAIL;
		}

		ID3DShaderReflection *pReflection = 0;

		_result			= D3DReflect( buffer, buffer_size, IID_ID3DShaderReflection, (void**)&pReflection);

		//	Parse constant, texture, sampler binding
		//	Store input signature blob
		if (SUCCEEDED(_result) && pReflection)
		{
			//	Let constant table parse it's data
			sgs_result->constants.parse(pReflection,RC_dest_geometry);

			_RELEASE(pReflection);
		}
		else
		{
			Log	("! PS: ", file_name);
			Msg	("! D3DReflectShader hr == 0x%08x", _result);
		}
	}
	else if (pTarget[0] == 'c') {
		_result = create_shader	( pTarget, buffer, buffer_size, file_name, (SCS*&)result, disasm );
	}
	else if (pTarget[0] == 'h') {
		_result = create_shader	( pTarget, buffer, buffer_size, file_name, (SHS*&)result, disasm );
	}
	else if (pTarget[0] == 'd') {
		_result = create_shader	( pTarget, buffer, buffer_size, file_name, (SDS*&)result, disasm );
	}
	else {
		NODEFAULT;
	}

	if ( disasm )
	{
		ID3DBlob*		disasm	= 0;
		D3DDisassemble	(buffer, buffer_size, FALSE, 0, &disasm );
		//D3DXDisassembleShader		(LPDWORD(code->GetBufferPointer()), FALSE, 0, &disasm );
		string_path		dname;
		strconcat		(sizeof(dname),dname,"disasm\\",file_name,('v'==pTarget[0])?".vs":('p'==pTarget[0])?".ps":".gs" );
		IWriter*		W		= FS.w_open("$logs$",dname);
		W->w			(disasm->GetBufferPointer(),(u32)disasm->GetBufferSize());
		FS.w_close		(W);
		_RELEASE		(disasm);
	}

	return				_result;
}

//--------------------------------------------------------------------------------------------------------------
class	includer				: public ID3DInclude
{
public:
	HRESULT __stdcall	Open	(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
	{
		string_path				pname;
		strconcat				(sizeof(pname),pname,::Render->getShaderPath(),pFileName);
		IReader*		R		= FS.r_open	("$game_shaders$",pname);
		if (0==R)				{
			// possibly in shared directory or somewhere else - open directly
			R					= FS.r_open	("$game_shaders$",pFileName);
			if (0==R)			return			E_FAIL;
		}

		// duplicate and zero-terminate
		u32				size	= R->length();
		u8*				data	= xr_alloc<u8>	(size + 1);
		CopyMemory			(data,R->pointer(),size);
		data[size]				= 0;
		FS.r_close				(R);

		*ppData					= data;
		*pBytes					= size;
		return	D3D_OK;
	}
	HRESULT __stdcall	Close	(LPCVOID	pData)
	{
		xr_free	(pData);
		return	D3D_OK;
	}
};

#include <boost/crc.hpp>

HRESULT	CRender::shader_compile(
	LPCSTR							name,
	DWORD const*					pSrcData,
	UINT                            SrcDataLen,
	LPCSTR                          pFunctionName,
	LPCSTR                          pTarget,
	DWORD                           Flags,
	void*&							result)
{
	D3D_SHADER_MACRO				defines[128];
	int								def_it = 0;

	char	sh_name[MAX_PATH] = "";

	for (u32 i = 0; i < m_ShaderOptions.size(); ++i)
	{
		defines[def_it++] = m_ShaderOptions[i];
	}

	u32 len = xr_strlen(sh_name);

	NEWDEF_STRNAME_STRVAL("mix", "lerp");

	NEWDEF_COND_STRNAME(o.forceskinw, "SKIN_COLOR");
	NEWDEF_COND_STRNAME(o.forceskinw, "SKIN_COLOR");
	NEWDEF_COND_STRNAME(o.forceskinw, "SKIN_COLOR");

	if (HW.FeatureLevel == D3D_FEATURE_LEVEL_10_0) { NEWDEF_STRNAME("SM_4_0"); }
	if (HW.FeatureLevel == D3D_FEATURE_LEVEL_10_1) { NEWDEF_STRNAME("SM_4_1"); }
	if (HW.FeatureLevel >= D3D_FEATURE_LEVEL_11_0) { NEWDEF_STRNAME("SM_5_0"); }

	// nvidia 8000, 9000 and 200 series
	if (HW.FeatureLevel == D3D_FEATURE_LEVEL_10_0)
	{ 
		NEWDEF_STRNAME_STRVAL("iSample", "0"); 
	}

	NEWDEF_COND_STRNAME(o.forceskinw, "SKIN_COLOR");

	switch (m_skinning)
	{
	case 0: 
		NEWDEF_STRNAME("SKIN_0"); 
		break;
	case 1: 
		NEWDEF_STRNAME("SKIN_1"); 
		break;
	case 2: 
		NEWDEF_STRNAME("SKIN_2"); 
		break;
	case 3: 
		NEWDEF_STRNAME("SKIN_3"); 
		break;
	case 4: 
		NEWDEF_STRNAME("SKIN_4"); 
		break;
	default: 
		NEWDEF_STRNAME("SKIN_NONE");
	};

	switch (o.smapsize)
	{
	case 1024:
		NEWDEF_STRNAME_STRVAL("SMAP_size", "1024");
		break;
	case 2048:
		NEWDEF_STRNAME_STRVAL("SMAP_size", "2048");
		break;
	case 4096:
		NEWDEF_STRNAME_STRVAL("SMAP_size", "4096");
		break;
	default:
		NEWDEF_STRNAME_STRVAL("SMAP_size", "512");
	};

	NEWDEF_COND_STRNAME(opt(R__DBG_FLAT_WATER), "WATER_DISABLE_NORMAL");
	NEWDEF_COND_STRNAME(opt(R__DBG_NOALBEDO), "DISABLE_ALBEDO");

	NEWDEF_COND_STRNAME(o.gbd_opt, "USE_GBD_OPT");

	if (o.advanced_mode)
	{
		NEWDEF_STRNAME("USE_ADVANCED_RENDER_MODE");

		NEWDEF_COND_STRNAME(o.pt_downsample, "USE_PT_DOWNSAMPLE");

		NEWDEF_COND_STRNAME(o.hud_shadows, "USE_HUD_SHADOWS");

		NEWDEF_COND_STRNAME(o.cspecular, "USE_CSPECULAR");

		NEWDEF_COND_STRNAME(o.sun_il || o.spot_il, "USE_IL");

		NEWDEF_COND_STRNAME(o.aa_mode == AA_TAA || o.aa_mode == AA_TAA_V2 || o.txaa == TRUE, "USE_TAA");
		
		NEWDEF_COND_STRNAME(o.msaa_samples_reflections > 1 && o.msaa_samples_reflections == o.msaa_samples, "USE_MSAA_REFLECTIONS_WITHOUT_RESOLVE");
		
		if (o.aa_mode == AA_MSAA)
		{
			switch (r__aa)
			{
			case AA_MSAA_FXAA: case AA_MSAA2S: case AA_TXAA2S:
				NEWDEF_STRNAME_STRVAL("MSAA_SAMPLES", "2");
				break;
			case AA_MSAA4S: case AA_TXAA4S:
				NEWDEF_STRNAME_STRVAL("MSAA_SAMPLES", "4");
				break;
			case AA_MSAA8S:
				NEWDEF_STRNAME_STRVAL("MSAA_SAMPLES", "8");
				break;
			};
		}

		switch (o.ssr)
		{
		case TRUE:
			NEWDEF_STRNAME_STRVAL("REFLECTIONS_QUALITY", "6");
			NEWDEF_COND_STRNAME(o.ssr_replace, "PLANAR_SSR_MODE");
			break;
		case FALSE:
			NEWDEF_STRNAME("REFLECTIONS_QUALITY");
			break;
		};

		NEWDEF_COND_STRNAME(o.planar, "PLANAR_MODE");
		
		NEWDEF_COND_STRNAME(o.ssao, "USE_SSAO");
		NEWDEF_COND_STRNAME(r__ssao_mode == SSAO_PATH_TRACING, "USE_SSAO_PATH_TRACING");

		NEWDEF_COND_STRNAME(opt(R__USE_WATER_DIST), "USE_WATER_DISTORT");
	}
	else
	{
		NEWDEF_STRNAME("REFLECTIONS_QUALITY");
	}

	NEWDEF_COND_STRNAME(is_sun_static(), "DX11_STATIC_DEFFERED_RENDERER");

	NEWDEF_COND_STRNAME(opt(R__USE_SOFT_WATER), "USE_SOFT_WATER");
	NEWDEF_COND_STRNAME(opt(R__USE_SOFT_PARTICLES), "USE_SOFT_PARTICLES");

	NEWDEF_COND_STRNAME(is_sun_static() && !opt(R__USE_BUMP), "DX11_STATIC_DISABLE_BUMP_MAPPING");

	NEWDEF_COND_STRNAME(opt(R__USE_DOF), "USE_DOF");

	NEWDEF_COND_STRNAME(o.tessellation, "USE_TESSELATION");

	switch (r__smap_filter)
	{
	case 1:
		NEWDEF_STRNAME_STRVAL("SHADOW_FILTERING", "1");
		break;
	case 2:
		NEWDEF_STRNAME_STRVAL("SHADOW_FILTERING", "2");
		break;
	case 3:
		NEWDEF_STRNAME_STRVAL("SHADOW_FILTERING", "3");
		break;
	case 4:
		NEWDEF_STRNAME_STRVAL("SHADOW_FILTERING", "4");
		break;
	};

	switch (r__parallax_mode)
	{
	case 1:
		NEWDEF_STRNAME_STRVAL("DEFFER_IBM_MODE", "1");
		break;
	case 2:
		NEWDEF_STRNAME_STRVAL("DEFFER_IBM_MODE", "2");
		break;
	case 3:
		NEWDEF_STRNAME_STRVAL("DEFFER_IBM_MODE", "3");
		break;
	};

	sh_name[len] = 0;

	// finish
	defines[def_it].Name			=	0;
	defines[def_it].Definition		=	0;
	def_it							++; 

	if (!xr_strcmp(pFunctionName, "main"))
	{
			 if ('v' == pTarget[0])	pTarget = "vs_5_0";
		else if ('p' == pTarget[0])	pTarget = "ps_5_0";
		else if ('g' == pTarget[0])	pTarget = "gs_5_0";
		else if ('c' == pTarget[0])	pTarget = "cs_5_0";

		if (HW.FeatureLevel == D3D_FEATURE_LEVEL_10_0)
		{
				 if ('v' == pTarget[0])	pTarget = "vs_4_0";
			else if ('p' == pTarget[0])	pTarget = "ps_4_0";
			else if ('g' == pTarget[0])	pTarget = "gs_4_0";
			else if ('c' == pTarget[0])	pTarget = "cs_4_0";
		}

		if (HW.FeatureLevel == D3D_FEATURE_LEVEL_10_1)
		{
				 if	('v' == pTarget[0])	pTarget = "vs_4_1";
			else if ('p' == pTarget[0])	pTarget = "ps_4_1";
			else if ('g' == pTarget[0])	pTarget = "gs_4_1";
			else if ('c' == pTarget[0])	pTarget = "cs_4_1";
		}
	}

	HRESULT		_result = E_FAIL;

	string_path	folder_name, folder;

	xr_strcpy(folder, "r3\\precompiled\\r4\\");

	xr_strcat		( folder, name );
	xr_strcat		( folder, "." );

	char extension[3];
	strncpy_s		( extension, pTarget, 2 );
	xr_strcat		( folder, extension );

	FS.update_path	( folder_name, "$game_shaders$", folder );
	xr_strcat		( folder_name, "\\" );
	
	m_file_set.clear( );
	FS.file_list	( m_file_set, folder_name, FS_ListFiles | FS_RootOnly, "*");

	string_path file_name, file;

	xr_strcpy(file, "shaders_cache\\r4\\");

	xr_strcat(file, name);
	xr_strcat(file, ".");
	xr_strcat(file, extension);
	xr_strcat(file, "\\");
	xr_strcat(file, sh_name);

	FS.update_path(file_name, "$app_data_root$", file);

	if (m_need_adv_cache)
	{
		if (FS.exist(file_name))
		{
			IReader* file = FS.r_open(file_name);
			if (file->length() > 4)
			{
				u32 crc = 0;
				crc = file->r_u32();

				boost::crc_32_type		processor;
				processor.process_block(file->pointer(), ((char*)file->pointer()) + file->elapsed());
				u32 const real_crc = processor.checksum();

				if (real_crc == crc) {
					_result = create_shader(pTarget, (DWORD*)file->pointer(), file->elapsed(), file_name, result, false);
				}
			}
			file->close();
		}

		m_need_recomp = FAILED(_result);
	}

	if (m_need_recomp)
	{
		includer Includer;

		LPD3DBLOB pShaderBuf = NULL;
		LPD3DBLOB pErrorBuf = NULL;

		_result = 
			D3DCompile( 
				pSrcData, 
				SrcDataLen,
				"",
				defines, 
				&Includer, 
				pFunctionName,
				pTarget,
				Flags, 
				0,
				&pShaderBuf,
				&pErrorBuf
			);

		if (SUCCEEDED(_result))
		{
			IWriter* file = FS.w_open(file_name);

			boost::crc_32_type		processor;
			processor.process_block	( pShaderBuf->GetBufferPointer(), ((char*)pShaderBuf->GetBufferPointer()) + pShaderBuf->GetBufferSize() );
			u32 const crc			= processor.checksum( );

			file->w_u32				(crc);
			file->w					(pShaderBuf->GetBufferPointer(), (u32)pShaderBuf->GetBufferSize());
			FS.w_close				(file);

			_result					= create_shader(pTarget, (DWORD*)pShaderBuf->GetBufferPointer(), (u32)pShaderBuf->GetBufferSize(), file_name, result, false);

			// slow... only for debug shaders
			if (m_need_warnings)
			{
				if (pErrorBuf)
				{
					LPCSTR currentLog = (LPCSTR)pErrorBuf->GetBufferPointer();

					BOOL isUnique = TRUE;

					for (u32 i = 0; i < m_vecShaderErrorBuf.size(); i++)
						if (strstr(m_vecShaderErrorBuf[i], currentLog))
							isUnique = FALSE;

					if (isUnique)
					{
						Log(file_name);
						Log(currentLog);
						m_vecShaderErrorBuf.push_back(currentLog);
					}
				}
			}
		}
		else 
		{
			Log("! ", file_name);

			if (pErrorBuf)
				Log((LPCSTR)pErrorBuf->GetBufferPointer());
			else
				Msg("Can't compile shader hr=0x%08x", _result);
		}
	}

	return		_result;
}