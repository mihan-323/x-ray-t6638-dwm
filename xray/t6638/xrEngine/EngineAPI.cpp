// EngineAPI.cpp: implementation of the CEngineAPI class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EngineAPI.h"
#include "../xrcdb/xrXRC.h"

#include "securom_api.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void __cdecl dummy		(void)	{};

CEngineAPI::CEngineAPI	()
{
	hGame			= 0;
	hRender			= 0;
	hTuner			= 0;
	pCreate			= 0;
	pDestroy		= 0;
	tune_pause		= dummy	;
	tune_resume		= dummy	;
}

ENGINE_API int g_current_renderer = 0; // for xrGame torch offset

ENGINE_API bool is_enough_address_space_available	()
{
	SYSTEM_INFO		system_info;

	SECUROM_MARKER_HIGH_SECURITY_ON(12)

	GetSystemInfo	( &system_info );

	SECUROM_MARKER_HIGH_SECURITY_OFF(12)

	return			(*(u32*)&system_info.lpMaximumApplicationAddress) > 0x90000000;	
}

extern "C" 
{
	typedef RendererSupport _declspec(dllexport) SupportsDX11Rendering();
};

static LPCSTR renderer_lib		= "xrRender_R4.dll";

RendererSupport CEngineAPI::TestRenderer()
{
	RendererSupport support;

	support.dx11 = false;

	hRender = LoadLibrary(renderer_lib);

	if (hRender)
	{
		SupportsDX11Rendering* dx11_supports_test = (SupportsDX11Rendering*)GetProcAddress(hRender, "SupportsDX11Rendering");
		support = dx11_supports_test();
		if (!support.dx11) Msg("! DX11 supports test failed");
		FreeLibrary(hRender);
	}

	if (!hRender)
	{
		//Msg("! Cant load DLL: %s", renderer_lib);
		Msg("! Error: %s", Debug.error2string(GetLastError()));
	}

	return support;
}

extern xr_token* vid_quality_token;

#ifdef DEBUG
extern xr_token* feature_level_token;
#endif

void CEngineAPI::CreateRendererList(RendererSupport support)
{
	if (vid_quality_token != NULL)		
		return;

	{
		u32 size = 1;

		if (support.dx11) size += 2;

		vid_quality_token = xr_alloc<xr_token>(size);

		if (support.dx11)
		{
			vid_quality_token[0].id = 0;
			vid_quality_token[0].name = xr_strdup("renderer_r4a");

			vid_quality_token[1].id = 1;
			vid_quality_token[1].name = xr_strdup("renderer_r4");

		}

		vid_quality_token[size - 1].id = -1;
		vid_quality_token[size - 1].name = NULL;
	}

#ifdef DEBUG
	{
		u32 size = 1;

		switch (support.level) 
		{ 
		case D3D_FEATURE_LEVEL_10_0: size = 3; break; 
		case D3D_FEATURE_LEVEL_10_1: size = 4; break; 
		case D3D_FEATURE_LEVEL_11_0: size = 5; break;	
		case D3D_FEATURE_LEVEL_11_1: size = 6; break;
		case D3D_FEATURE_LEVEL_12_0: size = 7; break;
		case D3D_FEATURE_LEVEL_12_1: size = 8; break;
		}

		feature_level_token = xr_alloc<xr_token>(size);

		xr_token feature_level_token_full[] =
		{
			{"opt_auto", 0 },
			{"opt_10_0", D3D_FEATURE_LEVEL_10_0 },
			{"opt_10_1", D3D_FEATURE_LEVEL_10_1 },
			{"opt_11_0", D3D_FEATURE_LEVEL_11_0 },
			{"opt_11_1", D3D_FEATURE_LEVEL_11_1 },
			{"opt_12_0", D3D_FEATURE_LEVEL_12_0 },
			{"opt_12_1", D3D_FEATURE_LEVEL_12_1 },
		};

		Msg("* Feature level options: ");

		for (u32 i = 0; i < size - 1; i++)
		{
			feature_level_token[i].id = feature_level_token_full[i].id;
			feature_level_token[i].name = xr_strdup(feature_level_token_full[i].name);

			Msg("* \t%s", feature_level_token[i].name);
		}

		feature_level_token[size - 1].id = -1;
		feature_level_token[size - 1].name = NULL;
	}
#endif
}

extern ENGINE_API u32 renderer_value;
extern ENGINE_API BOOL need_rsStaticSun;

void CEngineAPI::InitializeRenderer(RendererSupport support)
{
	if (!support.dx11)
	{
		Log("! DX11 does not supported!");
		return;
	}

	hRender = LoadLibrary(renderer_lib);
}

void CEngineAPI::Initialize(void)
{
	Device.ConnectToRender();

	// game	
	{
		LPCSTR			g_name	= "xrGame.dll";
		Log				("Loading DLL:",g_name);
		hGame			= LoadLibrary	(g_name);
		if (0==hGame)	R_CHK			(GetLastError());
		R_ASSERT2		(hGame,"Game DLL raised exception during loading or there is no game DLL at all");
		pCreate			= (Factory_Create*)		GetProcAddress(hGame,"xrFactory_Create"		);	R_ASSERT(pCreate);
		pDestroy		= (Factory_Destroy*)	GetProcAddress(hGame,"xrFactory_Destroy"	);	R_ASSERT(pDestroy);
	}

	//////////////////////////////////////////////////////////////////////////
	// vTune
	tune_enabled		= FALSE;
	if (strstr(Core.Params,"-tune"))	{
		LPCSTR			g_name	= "vTuneAPI.dll";
		Log				("Loading DLL:",g_name);
		hTuner			= LoadLibrary	(g_name);
		if (0==hTuner)	R_CHK			(GetLastError());
		R_ASSERT2		(hTuner,"Intel vTune is not installed");
		tune_enabled	= TRUE;
		tune_pause		= (VTPause*)	GetProcAddress(hTuner,"VTPause"		);	R_ASSERT(tune_pause);
		tune_resume		= (VTResume*)	GetProcAddress(hTuner,"VTResume"	);	R_ASSERT(tune_resume);
	}
}

void CEngineAPI::Destroy	(void)
{
	if (hGame)				{ FreeLibrary(hGame);	hGame	= 0; }
	if (hRender)			{ FreeLibrary(hRender); hRender = 0; }
	pCreate					= 0;
	pDestroy				= 0;
	Engine.Event._destroy	();
	XRC.r_clear_compact		();
}

CEngineAPI::~CEngineAPI()
{
	// destroy quality token here
	if (vid_quality_token)
	{
		for (int i = 0; vid_quality_token[i].name; i++)
		{
			xr_free(vid_quality_token[i].name);
		}
		xr_free(vid_quality_token);
		vid_quality_token = NULL;
	}

#ifdef DEBUG
	if (feature_level_token)
	{
		for (int i = 0; feature_level_token[i].name; i++)
		{
			xr_free(feature_level_token[i].name);
		}
		xr_free(feature_level_token);
		feature_level_token = NULL;
	}
#endif
}
