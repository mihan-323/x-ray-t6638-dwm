#include "stdafx.h"

#include "render.h"
#include "rendertarget.h"

#include "render_taa.h"

#pragma todo(Fix TAA matrices & backend)

#define TAA_FRAMES_COUNT 8

CTAA::CTAA()
{
	//M.m_V		= Device.mView;
	//M.m_P		= Device.mProject;
	M.m_VP		= Device.mFullTransform;
	//M.m_W		= Fidentity;
	//M.m_invW	= Fidentity;
	//M.m_WV		= M.m_V;
	//M.m_WVP		= M.m_VP;
}

// Set xforms into backend
void CTAA::set_xforms(const Fmatrix& World, const Fmatrix& View, const Fmatrix& Project)
{
	//M.m_V		= View;
	//M.m_P		= Project;
	M.m_VP		.mul(Project, View);
	//M.m_W		= World;
	//M.m_invW	.invert_b(World);
	//M.m_WV		.mul_43(View, World);
	//M.m_WVP		.mul(View, M.m_WV);

	//RCache.xforms_taa.m_v		= tM.m_V;
	//RCache.xforms_taa.m_p		= tM.m_P;
	//RCache.xforms_taa.m_vp		= tM.m_VP;
	//RCache.xforms_taa.m_w		= tM.m_W;
	//RCache.xforms_taa.m_invw	= tM.m_invW;
	//RCache.xforms_taa.m_wv		= tM.m_WV;
	//RCache.xforms_taa.m_wvp		= tM.m_WVP;
}

// Set xforms into backend
void CTAA::set_xforms(const taa_matrices& iM)
{
	M = iM;

	//RCache.xforms_taa.m_v		= tM.m_V;
	//RCache.xforms_taa.m_p		= tM.m_P;
	//RCache.xforms_taa.m_vp		= tM.m_VP;
	//RCache.xforms_taa.m_w		= tM.m_W;
	//RCache.xforms_taa.m_invw	= tM.m_invW;
	//RCache.xforms_taa.m_wv		= tM.m_WV;
	//RCache.xforms_taa.m_wvp		= tM.m_WVP;
}

// Get matrices from backend
taa_matrices CTAA::get_xforms()
{
	taa_matrices oM = {
		//RCache.xforms_taa.m_v,
		//RCache.xforms_taa.m_p,
		//RCache.xforms_taa.m_vp,
		//RCache.xforms_taa.m_w,
		//RCache.xforms_taa.m_invw,
		//RCache.xforms_taa.m_wv,
		//RCache.xforms_taa.m_wvp
	};

	oM.m_VP = tM.m_VP; // Hack

	return oM;
}

// Save last matrices from backend
void CTAA::fix_xforms()
{
	tM = M;
}

float CTAA::calc_jitter_x()
{
	static const float sequence[TAA_FRAMES_COUNT] = { 1, -1, 5, -3, -5, -7, 3, 7 }; // MSAA 8x pattern
	float scale = RImplementation.o.ssaa ? (float)RImplementation.Target->get_SSAA_params().w : (float)Device.dwWidth;
	float offset = sequence[Device.dwFrame % TAA_FRAMES_COUNT] / (4.0f * scale);
	return offset;
}

float CTAA::calc_jitter_y()
{
	static const float sequence[TAA_FRAMES_COUNT] = { -3, 3, 1, -5, 5, -1, 7, -7 }; // MSAA 8x pattern
	float scale = RImplementation.o.ssaa ? (float)RImplementation.Target->get_SSAA_params().h : (float)Device.dwWidth;
	float offset = sequence[Device.dwFrame % TAA_FRAMES_COUNT] / (4.0f * scale);
	return offset;
}

CTAA TAA;