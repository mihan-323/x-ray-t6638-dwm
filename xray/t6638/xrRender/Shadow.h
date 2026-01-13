//--------------------------------------------------------------------------------------
// This code contains NVIDIA Confidential Information and is disclosed 
// under the Mutual Non-Disclosure Agreement. 
// 
// Notice 
// ALL NVIDIA DESIGN SPECIFICATIONS AND CODE ("MATERIALS") ARE PROVIDED "AS IS" NVIDIA MAKES 
// NO REPRESENTATIONS, WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO 
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ANY IMPLIED WARRANTIES OF NONINFRINGEMENT, 
// MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. 
// 
// NVIDIA Corporation assumes no responsibility for the consequences of use of such 
// information or for any infringement of patents or other rights of third parties that may 
// result from its use. No license is granted by implication or otherwise under any patent 
// or patent rights of NVIDIA Corporation. No third party distribution is allowed unless 
// expressly authorized by NVIDIA.  Details are subject to change without notice. 
// This code supersedes and replaces all information previously supplied. 
// NVIDIA Corporation products are not authorized for use as critical 
// components in life support devices or systems without express written approval of 
// NVIDIA Corporation. 
// 
// Copyright © 2012, NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------


#pragma once

#include "gfsdk/gfsdk_shadowlib.h"
#include <string>

//#define TEST_PERF_MARKERS
//#define TEST_USER_DEFINED_CASCADES
#define TEST_PERF_COUNTERS

//--------------------------------------------------------------------------------------
// Example class showing how to use the NVSDK_Shadow library interface
//--------------------------------------------------------------------------------------
class Shadow
{
public:
	
	typedef GFSDK_ShadowLib_Status (__cdecl * GetDLLVersion)(
        GFSDK_ShadowLib_Version* __GFSDK_RESTRICT__ const pVersion );

	typedef GFSDK_ShadowLib_Status (__cdecl * Create)(
		const GFSDK_ShadowLib_Version* __GFSDK_RESTRICT__ const			pVersion, 
		GFSDK_ShadowLib_Context** __GFSDK_RESTRICT__ const				ppContext,
		const GFSDK_ShadowLib_DeviceContext* __GFSDK_RESTRICT__ const	pPlatformDevice,					
		gfsdk_new_delete_t*												customAllocator );
		
	void* m_GetDLLVersion_Proc;
	void* m_Create_Proc;

	GFSDK_ShadowLib_Version				m_ShadowLibVersion;
	GFSDK_ShadowLib_Context*			m_pShadowLibCtx;

	GFSDK_ShadowLib_DeviceContext		m_DeviceAndContext;
	GFSDK_ShadowLib_ShaderResourceView	m_ShadowBufferSRV;
		
	GFSDK_ShadowLib_Map*				m_pShadowMapHandle;
	GFSDK_ShadowLib_MapDesc				m_SMDesc;
	GFSDK_ShadowLib_BufferDesc			m_SBDesc;
	GFSDK_ShadowLib_MapRenderParams		m_SMRenderParams;
	GFSDK_ShadowLib_Buffer*				m_pShadowBufferHandle;
	GFSDK_ShadowLib_BufferRenderParams	m_SBRenderParams;
	gfsdk_float4x4						m_LightViewMatrices[GFSDK_ShadowLib_ViewType_Cascades_4];
	gfsdk_float4x4						m_LightProjectionMatrices[GFSDK_ShadowLib_ViewType_Cascades_4];
	GFSDK_ShadowLib_Frustum				m_RenderFrustum[GFSDK_ShadowLib_ViewType_Cascades_4];
	GFSDK_ShadowLib_ShaderResourceView	m_ShadowMapSRV;
			
	// Used to limit GUI sliders to sensible values
	float							m_LightSizeMax;
	float							m_PenumbraMaxThresholdMin;
	float							m_PenumbraMaxThresholdRange;
	

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	Shadow() 
	{
		m_pShadowLibCtx = NULL;
		memset( &m_DeviceAndContext, 0, sizeof( m_DeviceAndContext ) );
		memset( &m_ShadowBufferSRV, 0, sizeof( m_ShadowBufferSRV ) );
		m_pShadowMapHandle = NULL;
		m_pShadowBufferHandle = NULL;
		m_GetDLLVersion_Proc = NULL;
		m_Create_Proc = NULL;
				
		LoadDLL();
	}


	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	~Shadow()
	{
		ReleaseResources();
	}


	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	bool FindDLL(const char *filePath, std::string &fullPath)
	{
		FILE *fp = NULL;

		// loop N times up the hierarchy, testing at each level
		std::string upPath;
		for (int i = 0; i < 10; i++)
		{
			fullPath.assign(upPath);  // reset to current upPath.
			fullPath.append("bin/");
			fullPath.append(filePath);

			fopen_s(&fp, fullPath.c_str(), "rb");
			if (fp)
				break;

			upPath.append("../");
		}

		if (!fp)
		{
			fprintf(stderr, "Error opening file '%s'\n", filePath);
			return false;
		}

		fclose(fp);

		return true;
	}


	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void LoadDLL()
	{
		
#ifdef _WIN64
#ifdef _DEBUG
		std::string dllPath;
		if (!FindDLL("GFSDK_ShadowLib_DX11.debug.win64.dll", dllPath))
		{
			MessageBoxA(NULL, "GFSDK_ShadowLib_DX11.debug.win64.dll not found", "Error", MB_OK);
			exit(1);
		}
#else
		std::string dllPath;
		if (!FindDLL("GFSDK_ShadowLib_DX11.win64.dll", dllPath))
		{
			MessageBoxA(NULL, "GFSDK_ShadowLib_DX11.win64.dll not found", "Error", MB_OK);
			exit(1);
		}
#endif
#else
#ifdef _DEBUG
		std::string dllPath;
		if (!FindDLL("GFSDK_ShadowLib_DX11.debug.win32.dll", dllPath))
		{
			MessageBoxA(NULL, "GFSDK_ShadowLib_DX11.debug.win32.dll not found", "Error", MB_OK);
			exit(1);
		}
#else
		std::string dllPath;
		if (!FindDLL("GFSDK_ShadowLib_DX11.win32.dll", dllPath))
		{
			MessageBoxA(NULL, "GFSDK_ShadowLib_DX11.win32.dll not found", "Error", MB_OK);
			exit(1);
		}
#endif
#endif
		
		HMODULE module = LoadLibraryA(dllPath.c_str());
		if (!module)
		{
			MessageBoxA(NULL, "LoadLibraryA failed", "Error", MB_OK);
			exit(1);
		} 

		m_GetDLLVersion_Proc = GetProcAddress(module, "GFSDK_ShadowLib_GetDLLVersion");
		m_Create_Proc = GetProcAddress(module, "GFSDK_ShadowLib_Create");
	}
	

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
    bool CreateResources( ID3D11Device *pd3dDevice, ID3D11DeviceContext* pd3dDeviceContext )
	{
		// DLL version
		GFSDK_ShadowLib_Version DLLVersion;
		GFSDK_ShadowLib_Status retCode = ((GetDLLVersion)m_GetDLLVersion_Proc)( &DLLVersion );
		
		// Header version
		m_ShadowLibVersion.uMajor = GFSDK_SHADOWLIB_MAJOR_VERSION;
		m_ShadowLibVersion.uMinor = GFSDK_SHADOWLIB_MINOR_VERSION;

		// Do they match?
		if( DLLVersion.uMajor == m_ShadowLibVersion.uMajor && DLLVersion.uMinor == m_ShadowLibVersion.uMinor )
		{
			m_DeviceAndContext.pD3DDevice = pd3dDevice;
			m_DeviceAndContext.pDeviceContext = pd3dDeviceContext;

			retCode = ((Create)m_Create_Proc)( &m_ShadowLibVersion, &m_pShadowLibCtx, &m_DeviceAndContext, NULL );

			#ifdef TEST_PERF_MARKERS
				m_pShadowLibCtx->DevModeEnablePerfMarkers( true );
			#endif

			#ifdef TEST_PERF_COUNTERS
				m_pShadowLibCtx->DevModeEnablePerfCounters( true );
			#endif

			//if( retCode != GFSDK_ShadowLib_Status_Ok ) 
			//	assert( false );

			if (retCode != GFSDK_ShadowLib_Status_Ok)
				return false;

			return true;
		}
		else
		{
			//assert( false );
			return false;
		}
	}
    

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
    void ReleaseResources()
	{
		if( m_pShadowLibCtx != NULL )
		{
			m_pShadowLibCtx->Destroy();
			m_pShadowLibCtx = NULL;
		}
	}
	

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
    void SetScreenResolution( ID3D11Device* pd3dDevice, float FovyRad, UINT Width, UINT Height, UINT uSampleCount, ID3D11DepthStencilView* pReadOnlyDSV )
	{
		ChangeShadowSettings( pd3dDevice, Width, Height, uSampleCount, pReadOnlyDSV );
	}


	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void ChangeShadowSettings( ID3D11Device* pd3dDevice, UINT Width, UINT Height, UINT uSampleCount, ID3D11DepthStencilView* pReadOnlyDSV ) 
	{
		m_SBDesc.uResolutionWidth = Width;
		m_SBDesc.uResolutionHeight = Height;
		m_SBDesc.uViewportTop = 0;
		m_SBDesc.uViewportLeft = 0;
		m_SBDesc.uViewportBottom = Height;
		m_SBDesc.uViewportRight = Width;
		m_SBDesc.uSampleCount = uSampleCount;
		
		m_pShadowLibCtx->RemoveMap( &m_pShadowMapHandle );
		m_pShadowLibCtx->AddMap( &m_SMDesc, &m_SBDesc, &m_pShadowMapHandle );
				
		m_pShadowLibCtx->RemoveBuffer( &m_pShadowBufferHandle );
		m_pShadowLibCtx->AddBuffer( &m_SBDesc, &m_pShadowBufferHandle );
	}
	

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void SetMapRenderParams()
	{
		m_pShadowLibCtx->SetMapRenderParams( m_pShadowMapHandle, &m_SMRenderParams );
	}


	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void UpdateMapBounds()
	{
		m_pShadowLibCtx->UpdateMapBounds( m_pShadowMapHandle, &m_LightViewMatrices[0], &m_LightProjectionMatrices[0], &m_RenderFrustum[0] );

		#ifdef TEST_USER_DEFINED_CASCADES

			m_pShadowLibCtx->GetMapData( m_pShadowMapHandle, &m_ShadowMapSRV, &m_LightViewMatrices[0], &m_LightProjectionMatrices[0], &m_SMRenderParams.UserDefinedFrustum[0] );

			m_SMRenderParams.eCascadedShadowMapType = GFSDK_ShadowLib_CascadedShadowMapType_UserDefined;
						
			m_pShadowLibCtx->SetMapRenderParams( m_pShadowMapHandle, &m_SMRenderParams );

			m_pShadowLibCtx->UpdateMapBounds( m_pShadowMapHandle, &m_LightViewMatrices[0], &m_LightProjectionMatrices[0], &m_RenderFrustum[0] );

			m_SMRenderParams.eCascadedShadowMapType = GFSDK_ShadowLib_CascadedShadowMapType_SampleDistribution;

		#endif
	}


	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void InitializeMapRendering( GFSDK_ShadowLib_MapRenderType eMapRenderType )
	{
		m_pShadowLibCtx->InitializeMapRendering( m_pShadowMapHandle, eMapRenderType );
	}
	

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void BeginMapRendering(
		GFSDK_ShadowLib_MapRenderType	eMapRenderType,
		gfsdk_U32						uView )
	{
		m_pShadowLibCtx->BeginMapRendering( m_pShadowMapHandle, eMapRenderType, uView );
	}


	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void IncrementMapPrimitiveCounter(
		GFSDK_ShadowLib_MapRenderType	eMapRenderType,
		gfsdk_U32						uNumPrimitives )
	{
		m_pShadowLibCtx->IncrementMapPrimitiveCounter( m_pShadowMapHandle, eMapRenderType, uNumPrimitives );
	}


	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void EndMapRendering(
		GFSDK_ShadowLib_MapRenderType	eMapRenderType,
		gfsdk_U32						uView )
	{
		m_pShadowLibCtx->EndMapRendering( m_pShadowMapHandle, eMapRenderType, uView );
	}
	

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void RenderShadowBuffer( 
		ID3D11DeviceContext*		pd3dImmediateContext, 
		ID3D11ShaderResourceView*	pDepthStencilSRV,
		ID3D11ShaderResourceView*	pResolvedDepthStencilSRV )
	{
		m_pShadowLibCtx->ClearBuffer( m_pShadowBufferHandle );

		m_pShadowLibCtx->RenderBuffer(	m_pShadowMapHandle,
										m_pShadowBufferHandle,
										&m_SBRenderParams );
		
		m_pShadowLibCtx->FinalizeBuffer( m_pShadowBufferHandle, &m_ShadowBufferSRV );
	}


	//--------------------------------------------------------------------------------------
	// 
	//--------------------------------------------------------------------------------------
	void ModulateShadowBuffer( ID3D11RenderTargetView* pOutputRTV )
	{
		gfsdk_float3 v3ShadowColor;
		v3ShadowColor.x = 0.3f;
		v3ShadowColor.y = 0.3f;
		v3ShadowColor.z = 0.3f;

		GFSDK_ShadowLib_RenderTargetView ColorRTV;
		ColorRTV.pRTV = pOutputRTV;
		
		m_pShadowLibCtx->ModulateBuffer( m_pShadowBufferHandle, &ColorRTV, v3ShadowColor, GFSDK_ShadowLib_ModulateBufferMask_R );
	}


	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void DisplayShadowMaps( ID3D11DeviceContext* pd3dImmediateContext, ID3D11RenderTargetView* pOutputRTV, UINT Width, UINT Height  )
	{
		GFSDK_ShadowLib_RenderTargetView ColorRTV;
		ColorRTV.pRTV = pOutputRTV;
				
		float fMapResW = (float)m_SMDesc.uResolutionWidth;
		float fMapResH = (float)m_SMDesc.uResolutionHeight;
		
		float fWidthScale = Width / ( (float)m_SMDesc.eViewType * fMapResW );
		fWidthScale = ( fWidthScale > 1.0f ) ? ( 1.0f ) : ( fWidthScale );
				
		float fOneFifth = (float)Height / ( 5.0f );
		float fHeightScale = fOneFifth / fMapResH;
		fHeightScale = ( fHeightScale > 1.0f ) ? ( 1.0f ) : ( fHeightScale );

		float fScale = ( fHeightScale < fWidthScale ) ? ( fHeightScale ) : ( fWidthScale);

		fMapResW = floorf( fMapResW * fScale );
		fMapResH = floorf( fMapResH * fScale );

		for( unsigned int i = 0; i < (unsigned int)m_SMDesc.eViewType; i++ )
		{
			m_pShadowLibCtx->DevModeDisplayMap(	m_pShadowBufferHandle,
												&ColorRTV,
												m_pShadowMapHandle,
												i,
												i * (unsigned int)fMapResW + i,
												Height - (unsigned int)fMapResH,
												fScale );
		}
	}


	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void DisplayMapFrustums( ID3D11RenderTargetView* pOutputRTV, ID3D11DepthStencilView* pDSV )
	{
		gfsdk_float3 v3Color;
		v3Color.x = 1.0f;
		v3Color.y = 0.0f;
		v3Color.z = 0.0f;

		GFSDK_ShadowLib_RenderTargetView ColorRTV;
		ColorRTV.pRTV = pOutputRTV;

		GFSDK_ShadowLib_DepthStencilView DSV;
		DSV.pDSV = pDSV;
				
		unsigned int NumViews = m_SMDesc.eViewType;

		for( unsigned int i = 0; i < NumViews; i++ )
		{
			switch( i )
			{
				case 0:
					v3Color.x = 0.0f;
					v3Color.y = 1.0f;
					v3Color.z = 0.0f;
					break;
				case 1:
					v3Color.x = 1.0f;
					v3Color.y = 0.0f;
					v3Color.z = 0.0f;
					break;
				case 2:
					v3Color.x = 0.0f;
					v3Color.y = 0.0f;
					v3Color.z = 1.0f;
					break;
				case 3:
					v3Color.x = 1.0f;
					v3Color.y = 1.0f;
					v3Color.z = 0.0f;
					break;
			}

			m_pShadowLibCtx->DevModeDisplayMapFrustum(	m_pShadowBufferHandle,
														&ColorRTV,
														&DSV,
														m_pShadowMapHandle,
														i,
														v3Color );
		}
	}
		

	//--------------------------------------------------------------------------------------
	//
	//--------------------------------------------------------------------------------------
	void DisplayShadowBuffer( ID3D11RenderTargetView* pOutputRTV )
	{
		gfsdk_float2 v2Scale;
		v2Scale.x = 1.0f;
		v2Scale.y = 1.0f;

		GFSDK_ShadowLib_RenderTargetView ColorRTV;
		ColorRTV.pRTV = pOutputRTV;

		m_pShadowLibCtx->DevModeDisplayBuffer(	m_pShadowBufferHandle,
												&ColorRTV,
												v2Scale,
												NULL );
	}
};


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------