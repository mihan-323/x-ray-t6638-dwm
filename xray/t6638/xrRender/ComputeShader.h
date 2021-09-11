////////////////////////////////////////////////////////////////////////////
//	Created		: 21.05.2009
//	Author		: Mykhailo Parfeniuk
//	Copyright (C) GSC Game World - 2009
////////////////////////////////////////////////////////////////////////////

#ifndef COMPUTESHADER_H_INCLUDED
#define COMPUTESHADER_H_INCLUDED

class ComputeShader
{
	friend class CSCompiler;
public:
	~ComputeShader();

	ComputeShader& set_c(shared_str name, const Fvector4& value);
	ComputeShader& set_c(shared_str name, float x, float y, float z, float w);

	void Dispatch(u32 dimx, u32 dimy, u32 dimz);

private:
	void Construct(
		ID3DComputeShader*		cs,
		ref_ctable				ctable,
		xr_vector<ID3DSamplerState*>&			Samplers,
		xr_vector<ID3DShaderResourceView*>&		Textures,
		xr_vector<ID3DUnorderedAccessView*>&	Outputs
	);

private:
	ID3DComputeShader*		m_cs;
	ref_ctable				m_ctable;
	xr_vector<ID3DSamplerState*>			m_Samplers;
	xr_vector<ID3DShaderResourceView*>		m_Textures;
	xr_vector<ID3DUnorderedAccessView*>		m_Outputs;
}; // class ComputeShader

#endif // #ifndef COMPUTESHADER_H_INCLUDED