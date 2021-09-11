////////////////////////////////////////////////////////////////////////////
//	Created		: 22.05.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CSCompiler.h"
#include "ComputeShader.h"
#include "dxRenderDeviceRender.h"

CSCompiler::CSCompiler(ComputeShader& target):
	m_Target(target), m_cs(0)
{
}

CSCompiler& CSCompiler::begin(const char* name)
{
	compile(name);
	return *this;
}

CSCompiler& CSCompiler::defSampler(LPCSTR ResourceName)
{
	D3D_SAMPLER_DESC	desc;
	ZeroMemory(&desc, sizeof(desc));

	//	Use D3D_TEXTURE_ADDRESS_CLAMP,	D3DTEXF_POINT,			D3DTEXF_NONE,	D3DTEXF_POINT 
	if (0==xr_strcmp(ResourceName,"smp_nofilter"))
	{
		desc.AddressU = desc.AddressV = desc.AddressW = D3D_TEXTURE_ADDRESS_CLAMP;
		desc.Filter = D3D_FILTER_MIN_MAG_MIP_POINT;
		return defSampler(ResourceName, desc);
	}

	//	Use D3D_TEXTURE_ADDRESS_CLAMP,	D3DTEXF_LINEAR,			D3DTEXF_NONE,	D3DTEXF_LINEAR 
	if (0==xr_strcmp(ResourceName,"smp_rtlinear"))
	{
		desc.AddressU = desc.AddressV = desc.AddressW = D3D_TEXTURE_ADDRESS_CLAMP;
		desc.Filter = D3D_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		return defSampler(ResourceName, desc);
	}

	//	Use	D3D_TEXTURE_ADDRESS_WRAP,	D3DTEXF_LINEAR,			D3DTEXF_LINEAR,	D3DTEXF_LINEAR
	if (0==xr_strcmp(ResourceName,"smp_linear"))
	{
		desc.AddressU = desc.AddressV = desc.AddressW = D3D_TEXTURE_ADDRESS_WRAP;
		desc.Filter = D3D_FILTER_MIN_MAG_MIP_LINEAR;
		return defSampler(ResourceName, desc);
	}

	//	Use D3D_TEXTURE_ADDRESS_WRAP,	D3DTEXF_ANISOTROPIC, 	D3DTEXF_LINEAR,	D3DTEXF_ANISOTROPIC
	if (0==xr_strcmp(ResourceName,"smp_base"))
	{
		desc.AddressU = desc.AddressV = desc.AddressW = D3D_TEXTURE_ADDRESS_WRAP;
		desc.Filter = D3D_FILTER_ANISOTROPIC;
		desc.MaxAnisotropy = 8;
		return defSampler(ResourceName, desc);
	}

	//	Use D3D_TEXTURE_ADDRESS_CLAMP,	D3DTEXF_LINEAR,			D3DTEXF_NONE,	D3DTEXF_LINEAR
	if (0==xr_strcmp(ResourceName,"smp_material"))
	{
		desc.AddressU = desc.AddressV = D3D_TEXTURE_ADDRESS_CLAMP;
		desc.AddressW = D3D_TEXTURE_ADDRESS_WRAP;
		desc.Filter = D3D_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		return defSampler(ResourceName, desc);
	}

	if (0==xr_strcmp(ResourceName,"smp_smap"))
	{
		desc.AddressU = desc.AddressV = desc.AddressW = D3D_TEXTURE_ADDRESS_CLAMP;
		desc.Filter = D3D_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		desc.ComparisonFunc = D3D_COMPARISON_LESS_EQUAL;
		return defSampler(ResourceName, desc);
	}

	if (0==xr_strcmp(ResourceName,"smp_jitter"))
	{
		desc.AddressU = desc.AddressV = desc.AddressW = D3D_TEXTURE_ADDRESS_WRAP;
		desc.Filter = D3D_FILTER_MIN_MAG_MIP_POINT;
		return defSampler(ResourceName, desc);
	}

	VERIFY(0);

	return *this;
}

CSCompiler& CSCompiler::defSampler(LPCSTR ResourceName, const D3D_SAMPLER_DESC& def)
{
	VERIFY(ResourceName);

	ref_constant C			= m_constants.get(ResourceName);
	if (!C)					return	*this;

	R_ASSERT				(C->type == RC_sampler);
	u32 stage				= C->samp.index;

	if (stage >= m_Samplers.size())
		m_Samplers.resize(stage+1);

	R_CHK(HW.pDevice->CreateSamplerState(&def, &m_Samplers[stage]));

	return *this;
}

void fix_texture_name(LPSTR);

CSCompiler& CSCompiler::defOutput(LPCSTR ResourceName,	ref_rt rt)
{
	VERIFY(ResourceName);
	if (!rt) return *this;

	ref_constant C			= m_constants.get(ResourceName);
	if (!C)					return *this;

	R_ASSERT				(C->type == RC_dx11UAV);
	u32 stage				= C->samp.index;

	if (stage >= m_Textures.size())
		m_Textures.resize(stage+1);

	m_Outputs[stage] = rt->pUAView; //!!!dangerous view can be deleted

	return *this;
}

CSCompiler& CSCompiler::defTexture(LPCSTR ResourceName,	ref_texture texture)
{
	VERIFY(ResourceName);
	if (!texture) return *this;

	// Find index
	ref_constant C			= m_constants.get(ResourceName);
	if (!C)					return *this;

	R_ASSERT				(C->type == RC_dx10texture);
	u32 stage				= C->samp.index;

	if (stage >= m_Textures.size())
		m_Textures.resize(stage+1);

	m_Textures[stage] = texture->get_SRView(); //!!!dangerous view can be deleted

	return *this;
}

void CSCompiler::end()
{
	for (size_t i=0; i<m_Textures.size(); ++i)
		m_Textures[i]->AddRef();

	for (size_t i=0; i<m_Outputs.size(); ++i)
		m_Outputs[i]->AddRef();

	//Samplers create by us, thou they should not be AddRef'ed

	m_Target.Construct(m_cs, DEV->_CreateConstantTable(m_constants), m_Samplers, m_Textures, m_Outputs);
}

void CSCompiler::compile(const char* name)
{
	if (0==stricmp(name, "null"))
	{
		m_cs = 0;
		return;
	}

	string_path					cname;
	strconcat					(sizeof(cname),cname,::Render->getShaderPath(),name,".cs");
	FS.update_path				(cname,	"$game_shaders$", cname);

	IReader* file				= FS.r_open(cname);
	R_ASSERT2					( file, cname );

	// Select target

	std::string						c_target = "cs_5_0";
	std::string						c_entry = "main";

	u32	const size = file->length();
	char* const data = (LPSTR)_alloca(size + 1);
	CopyMemory(data, file->pointer(), size);
	data[size] = 0;

	if (HW.FeatureLevel == D3D_FEATURE_LEVEL_10_0) c_target = "cs_4_0";
	if (HW.FeatureLevel == D3D_FEATURE_LEVEL_10_1) c_target = "cs_4_1";
	if (HW.FeatureLevel >= D3D_FEATURE_LEVEL_11_0) c_target = "cs_5_0";

	HRESULT	const _hr			= ::Render->shader_compile(name,(DWORD const*)file->pointer(),file->length(), c_entry.c_str(), c_target.c_str(), D3D_SHADER_PACK_MATRIX_ROW_MAJOR, (void*&)m_cs );

	FS.r_close					( file );

	VERIFY(SUCCEEDED(_hr));

	CHECK_OR_EXIT				(
		!FAILED(_hr),
		make_string("Your video card doesn't meet game requirements.\n\nTry to lower game settings.")
	);
}
