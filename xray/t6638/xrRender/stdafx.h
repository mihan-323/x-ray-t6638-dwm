// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently

#pragma once

#pragma warning(disable:4995)
#include "../xrEngine/stdafx.h"
#pragma warning(disable:4995)
#include <d3dx9core.h>
#pragma warning(default:4995)
#pragma warning(disable:4714)
#pragma warning( 4 : 4018 )
#pragma warning( 4 : 4244 )
#pragma warning(disable:4237)

#ifdef CLEAR_SKY_BUILD
#include "stdintport.h"
#else
#include "stdintport/stdintport.h"
#endif

#include <D3D11.h>
#include <D3D11_3.h>
#include <D3Dx11core.h>
#include <D3DCompiler.h>

#ifdef CLEAR_SKY_BUILD
#define xr_strcpy strcpy_s
#define xr_strcat strcat_s
#define xr_sprintf sprintf_s
#define _BCL
#define RDEVICE Device
#endif

#define __GFSDK_DX11__

#ifdef __GFSDK_DX11__
#ifdef CLEAR_SKY_BUILD
#ifdef TXAA_BUILD
#include "GFSDK_TXAA.h"
#endif
#include "GFSDK_SSAO.h"
#else
#ifdef TXAA_BUILD
#include <GFSDK/GFSDK_TXAA.h>
#endif
#include <GFSDK/GFSDK_SSAO.h>
#endif
#endif

#include "xrD3DDefs.h"

#include "dxPixEventWrapper.h"

#include "HW.h"
#include "Shader.h"
#include "R_Backend.h"
#include "R_Backend_Runtime.h"

#include "resourcemanager.h"

#include "../xrEngine/vis_common.h"
#include "../xrEngine/render.h"
#include "../xrEngine/_d3d_extensions.h"
#include "../xrEngine/igame_level.h"
#include "blender.h"
#include "blender_clsid.h"
#include "../xrParticles/psystem.h"
#include "xrRender_console.h"
#include "render.h"
