#include "stdafx.h"
#pragma hdrstop

#include "blender_deffer_model.h"
#include "uber_deffer.h"

CBlender_deffer_model::CBlender_deffer_model	()	{	
	description.CLS		= B_MODEL;	
	description.version	= 2;
	oTessellation.Count         = 4;
	oTessellation.IDselected	= 0;
	oAREF.value			= 32;
	oAREF.min			= 0;
	oAREF.max			= 255;
	oBlend.value		= FALSE;
}
CBlender_deffer_model::~CBlender_deffer_model	()	{	}

void	CBlender_deffer_model::Save	(	IWriter& fs )
{
	IBlender::Save		(fs);
	xrPWRITE_PROP		(fs,"Use alpha-channel",	xrPID_BOOL,		oBlend);
	xrPWRITE_PROP		(fs,"Alpha ref",			xrPID_INTEGER,	oAREF);
	xrP_TOKEN::Item	I;
	xrPWRITE_PROP	(fs,"Tessellation",	xrPID_TOKEN, oTessellation);
	I.ID = 0; xr_strcpy(I.str,"NO_TESS");	fs.w		(&I,sizeof(I));
	I.ID = 1; xr_strcpy(I.str,"TESS_PN");	fs.w		(&I,sizeof(I));
	I.ID = 2; xr_strcpy(I.str,"TESS_HM");	fs.w		(&I,sizeof(I));
	I.ID = 3; xr_strcpy(I.str,"TESS_PN+HM");	fs.w		(&I,sizeof(I));
}
void	CBlender_deffer_model::Load	(	IReader& fs, u16 version )
{
	IBlender::Load		(fs,version);

	switch (version)	
	{
	case 0: 
		oAREF.value			= 32;
		oAREF.min			= 0;
		oAREF.max			= 255;
		oBlend.value		= FALSE;
		break;
	case 1:
	default:
		xrPREAD_PROP	(fs,xrPID_BOOL,		oBlend);
		xrPREAD_PROP	(fs,xrPID_INTEGER,	oAREF);
		break;
	}
	if (version>1)
	{
		xrPREAD_PROP(fs,xrPID_TOKEN,oTessellation);
	}
}

void	CBlender_deffer_model::Compile(CBlender_Compile& C)
{
	IBlender::Compile		(C);

	BOOL	bForward		= FALSE;
	if (oBlend.value && oAREF.value<16)	bForward	= TRUE;
	if (oStrictSorting.value)			bForward	= TRUE;

	if (bForward)			{
		// forward rendering
		LPCSTR	vsname,psname;
		switch(C.iElement) 
		{
		case 0: 	//
		case 1: 	//
			vsname = psname =	"model_def_lq"; 
			C.r_Pass			(vsname,psname,TRUE,TRUE,FALSE,TRUE, D3D_BLEND_SRC_ALPHA, D3D_BLEND_INV_SRC_ALPHA,	TRUE,oAREF.value);
			C.r_dx10Texture("s_base", C.L_textures[0]);
			C.r_dx10Sampler("smp_base");
			C.r_End				();
			break;
		default:
			break;
		}
	} else {
		BOOL	bAref		= oBlend.value;
		// deferred rendering
		// codepath is the same, only the shaders differ

		C.TessMethod = oTessellation.IDselected;
		switch(C.iElement) 
		{
		case SE_NORMAL_HQ: 			// deffer
			uber(C, true, "model", "base", bAref, 0, true); 
			C.r_Stencil	(TRUE, D3D_COMPARISON_ALWAYS,0xff,0x7f, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_KEEP);
			C.r_StencilRef (0x01);
			C.r_End			();
		break;
		case SE_NORMAL_LQ: 			// deffer
			uber(C, false, "model", "base", bAref, 0, true); 
			C.r_Stencil(TRUE, D3D_COMPARISON_ALWAYS,0xff,0x7f, D3D_STENCIL_OP_KEEP, D3D_STENCIL_OP_REPLACE, D3D_STENCIL_OP_KEEP);
			C.r_StencilRef (0x01);
			C.r_End			();
		break;
		case SE_SHADOW:		// smap
			if (bAref)
			{
				C.r_Pass	("shadow_direct_model_aref","shadow_direct_base_aref",	FALSE,TRUE,TRUE,FALSE, D3D_BLEND_ZERO, D3D_BLEND_ONE,TRUE,220);
				C.r_dx10Texture("s_base",C.L_textures[0]);
				C.r_dx10Sampler("smp_base");
				C.r_ColorWriteEnable(RImplementation.o.vsm, RImplementation.o.vsm, false, false);
				C.r_End			();
				break;
			} 
			else 
			{
				C.r_Pass	("shadow_direct_model", "shadow_direct_base",	FALSE,TRUE,TRUE,FALSE);
				C.r_dx10Texture("s_base", C.L_textures[0]);
				C.r_dx10Sampler("smp_base");
				C.r_ColorWriteEnable(RImplementation.o.vsm, RImplementation.o.vsm, false, false);
				C.r_End			();
				break;
			}
			break;

		case SE_RSM_FILL_RTS:	// RSM
			uber_rsm(C, "model", bAref);
			break;
		case SE_ZPREPASS:			// zprepass
			uber_zprepass(C, "model_multisampled", bAref);
			break;
		case SE_PLANAR:
			uber_planar(C, "model", bAref);
			break;

		}
	}
}
