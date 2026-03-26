#ifndef	common_samplers_h_included
#define	common_samplers_h_included

//////////////////////////////////////////////////////////////////////////////////////////
// Jitter

uniform SamplerState smp_jitter;

uniform Texture2D jitter4;
uniform Texture2D jitter3;
uniform Texture2D jitter2;
uniform Texture2D jitter1;
uniform Texture2D jitter0;

//////////////////////////////////////////////////////////////////////////////////////////
// Geometry phase / deferring

uniform SamplerState smp_nofilter;	//	Use D3DTADDRESS_CLAMP,	D3DTEXF_POINT,			D3DTEXF_NONE,	D3DTEXF_POINT
uniform SamplerState smp_rtlinear;	//	Use D3DTADDRESS_CLAMP,	D3DTEXF_LINEAR,			D3DTEXF_NONE,	D3DTEXF_LINEAR
uniform SamplerState smp_linear;	//	Use	D3DTADDRESS_WRAP,	D3DTEXF_LINEAR,			D3DTEXF_LINEAR,	D3DTEXF_LINEAR
uniform SamplerState smp_base;		//	Use D3DTADDRESS_WRAP,	D3DTEXF_ANISOTROPIC, 	D3DTEXF_LINEAR,	D3DTEXF_ANISOTROPIC

uniform Texture2D s_base;
uniform Texture2D s_bump;
uniform Texture2D s_bumpX;
uniform Texture2D s_detail;
uniform Texture2D s_detailBump;
uniform Texture2D s_detailBumpX; //	Error for bump detail

uniform Texture2D s_hemi;

//////////////////////////////////////////////////////////////////////////////////////////
// Lighting/shadowing phase

uniform SamplerState smp_material;

uniform Texture3D<float2> s_material;

uniform Texture2D s_lmap; // 2D/???cube projector lightmap

//////////////////////////////////////////////////////////////////////////////////////////
// Combine phase

uniform Texture2D<float> s_tonemap;

uniform Texture2D s_bloom;
uniform Texture2D s_image;	// used in various post-processing

//////////////////////////////////////////////////////////////////////////////////////////
// Other

uniform TextureCube<float3> env_s0;
uniform TextureCube<float3> env_s1;

#endif