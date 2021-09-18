#pragma once

#define FEATURE_LEVELS_DEBUG

namespace RenderCreationParams
{
	struct RendererSupport
	{
		bool dx11;
#ifdef FEATURE_LEVELS_DEBUG
		D3D_FEATURE_LEVEL level;
#endif
	};

	enum
	{
		R_R4A,
		R_R4
	};

	static const D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};

	static const UINT count = sizeof levels / sizeof levels[0];

	static const D3D_FEATURE_LEVEL levels12[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
	};

	static const UINT count12 = sizeof levels12 / sizeof levels12[0];
}