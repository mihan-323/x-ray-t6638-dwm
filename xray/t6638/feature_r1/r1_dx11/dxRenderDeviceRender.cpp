#include "stdafx.h"
#include "dxRenderDeviceRender.h"

#include "ResourceManager.h"

dxRenderDeviceRender::dxRenderDeviceRender()
	:	Resources(0)
{
	;
}

void dxRenderDeviceRender::Copy(IRenderDeviceRender &_in)
{
	*this = *(dxRenderDeviceRender*)&_in;
}

void dxRenderDeviceRender::setGamma(float fGamma)
{
	m_Gamma.Gamma(fGamma);
}

void dxRenderDeviceRender::setBrightness(float fGamma)
{
	m_Gamma.Brightness(fGamma);
}

void dxRenderDeviceRender::setContrast(float fGamma)
{
	m_Gamma.Contrast(fGamma);
}

void dxRenderDeviceRender::updateGamma()
{
	m_Gamma.Update();
}

void dxRenderDeviceRender::OnDeviceDestroy( BOOL bKeepTextures)
{
	m_WireShader.destroy();
	m_SelectionShader.destroy();

	Resources->OnDeviceDestroy( bKeepTextures);
	RCache.OnDeviceDestroy();
}

void dxRenderDeviceRender::ValidateHW()
{
	HW.Validate();
}

void dxRenderDeviceRender::DestroyHW()
{
	xr_delete					(Resources);
	HW.DestroyDevice			();
}

void  dxRenderDeviceRender::Reset( HWND hWnd, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2)
{
#ifdef DEBUG
	_SHOW_REF("*ref -CRenderDevice::ResetTotal: DeviceREF:",HW.pDevice);
#endif // DEBUG	

	Resources->reset_begin	();
	Memory.mem_compact		();
	HW.Reset				(hWnd);


	dwWidth					= HW.m_ChainDesc.BufferDesc.Width;
	dwHeight				= HW.m_ChainDesc.BufferDesc.Height;


	fWidth_2				= float(dwWidth/2);
	fHeight_2				= float(dwHeight/2);
	Resources->reset_end	();

#ifdef DEBUG
	_SHOW_REF("*ref +CRenderDevice::ResetTotal: DeviceREF:",HW.pDevice);
#endif // DEBUG
}

void dxRenderDeviceRender::SetupStates()
{
	HW.Caps.Update			();

}

void dxRenderDeviceRender::OnDeviceCreate(LPCSTR shName)
{
	// Signal everyone - device created
	RCache.OnDeviceCreate		();
	m_Gamma.Update				();
	Resources->OnDeviceCreate	(shName);
	::Render->create			();
	Device.Statistic->OnDeviceCreate	();

//#ifndef DEDICATED_SERVER
	if (!g_dedicated_server)
	{
		m_WireShader.create			("editor\\wire");
		m_SelectionShader.create	("editor\\selection");

		DUImpl.OnDeviceCreate			();
	}
//#endif
}

void dxRenderDeviceRender::Create( HWND hWnd, u32 &dwWidth, u32 &dwHeight, float &fWidth_2, float &fHeight_2, bool move_window)
{
	HW.CreateDevice		(hWnd, move_window);


	dwWidth					= HW.m_ChainDesc.BufferDesc.Width;
	dwHeight				= HW.m_ChainDesc.BufferDesc.Height;


	fWidth_2			= float(dwWidth/2)			;
	fHeight_2			= float(dwHeight/2)			;

	Resources			= xr_new<CResourceManager>		();
}

void dxRenderDeviceRender::SetupGPU( BOOL bForceGPU_SW, BOOL bForceGPU_NonPure, BOOL bForceGPU_REF)
{
	HW.Caps.bForceGPU_SW		= bForceGPU_SW;
	HW.Caps.bForceGPU_NonPure	= bForceGPU_NonPure;
	HW.Caps.bForceGPU_REF		= bForceGPU_REF;
}

void dxRenderDeviceRender::overdrawBegin()
{

	//	TODO: DX10: Implement overdrawBegin
	VERIFY(!"dxRenderDeviceRender::overdrawBegin not implemented.");

}

void dxRenderDeviceRender::overdrawEnd()
{

	//	TODO: DX10: Implement overdrawEnd
	VERIFY(!"dxRenderDeviceRender::overdrawBegin not implemented.");

}

void dxRenderDeviceRender::DeferredLoad(BOOL E)
{
	Resources->DeferredLoad(E);
}

void dxRenderDeviceRender::ResourcesDeferredUpload()
{
	Resources->DeferredUpload();
}

void dxRenderDeviceRender::ResourcesGetMemoryUsage(u32& m_base, u32& c_base, u32& m_lmaps, u32& c_lmaps)
{
	if (Resources)
		Resources->_GetMemoryUsage(m_base, c_base, m_lmaps, c_lmaps);
}

void dxRenderDeviceRender::ResourcesStoreNecessaryTextures()
{
	dxRenderDeviceRender::Instance().Resources->StoreNecessaryTextures();
}

void dxRenderDeviceRender::ResourcesDumpMemoryUsage()
{
	dxRenderDeviceRender::Instance().Resources->_DumpMemoryUsage();
}

dxRenderDeviceRender::DeviceState dxRenderDeviceRender::GetDeviceState()
{
	HW.Validate		();
	

	HRESULT	_hr = HW.m_pSwapChain->Present(0, DXGI_PRESENT_TEST);


	if (FAILED(_hr))
	{

		if(DXGI_ERROR_DEVICE_RESET == _hr)
			return dsNeedReset;

	}

	return dsOK;
}

BOOL dxRenderDeviceRender::GetForceGPU_REF()
{
	return HW.Caps.bForceGPU_REF;
}

u32 dxRenderDeviceRender::GetCacheStatPolys()
{
	return RCache.stat.polys;
}

void dxRenderDeviceRender::Begin()
{
	RCache.OnFrameBegin		();
	RCache.set_CullMode		(D3D_CULL_FRONT);
	RCache.set_CullMode		(D3D_CULL_BACK);
	if (HW.Caps.SceneMode)	overdrawBegin	();
}

void dxRenderDeviceRender::Clear()
{
	RCache.clear_CurrentDepthStencilView();

	if (psDeviceFlags.test(rsClearBB)) 
		RCache.clear_CurrentRenderTargetView(rgba_black);
}

void DoAsyncScreenshot();

void dxRenderDeviceRender::End()
{
	VERIFY	(HW.pDevice);

	if (HW.Caps.SceneMode)	overdrawEnd();

	RCache.OnFrameEnd	();
	Memory.dbg_check		();

	DoAsyncScreenshot();

	HW.m_pSwapChain->Present( 0, 0 );

}

void dxRenderDeviceRender::ResourcesDestroyNecessaryTextures()
{
	Resources->DestroyNecessaryTextures();
}

void dxRenderDeviceRender::ClearTarget()
{
	RCache.clear_CurrentRenderTargetView(rgba_black);
}

void dxRenderDeviceRender::SetCacheXform(Fmatrix &mView, Fmatrix &mProject)
{
	RCache.set_xform_view(mView);
	RCache.set_xform_project(mProject);
}

bool dxRenderDeviceRender::HWSupportsShaderYUV2RGB()
{
	u32		v_dev	= CAP_VERSION(HW.Caps.raster_major, HW.Caps.raster_minor);
	u32		v_need	= CAP_VERSION(2,0);
	return (v_dev>=v_need);
}

void  dxRenderDeviceRender::OnAssetsChanged()
{
	Resources->m_textures_description.UnLoad();
	Resources->m_textures_description.Load();
}
