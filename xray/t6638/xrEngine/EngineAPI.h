// EngineAPI.h: interface for the CEngineAPI class.
//
//****************************************************************************
// Support for extension DLLs
//****************************************************************************

#if !defined(AFX_ENGINEAPI_H__CF21372B_C8B8_4891_82FC_D872C84E1DD4__INCLUDED_)
#define AFX_ENGINEAPI_H__CF21372B_C8B8_4891_82FC_D872C84E1DD4__INCLUDED_
#pragma once

#ifndef FEATURE_R1
#include "d3d11.h"
#endif

enum
{
	R_R4A,
	R_R4
};

struct RendererSupport
{
	bool dx11;
#ifdef DEBUG
	D3D_FEATURE_LEVEL level;
#endif
};

// Abstract 'Pure' class for DLL interface
class ENGINE_API DLL_Pure {
public:
	CLASS_ID				CLS_ID;

	DLL_Pure(void *params)	{CLS_ID=0; };
	DLL_Pure()				{CLS_ID=0; };
	virtual	DLL_Pure*		_construct		()	{ return this; 	}
	virtual ~DLL_Pure()		{};
};

// Class creation/destroying interface
extern "C" {
typedef DLL_API  DLL_Pure*	  __cdecl Factory_Create	(CLASS_ID	CLS_ID);
typedef DLL_API  void		  __cdecl Factory_Destroy	(DLL_Pure*	O);
};

// Tuning interface
extern "C" {
	typedef void __cdecl VTPause	(void);
	typedef void __cdecl VTResume	(void);
};

class ENGINE_API		CEngineAPI
{
private:
	HMODULE				hGame;
	HMODULE				hRender;
	HMODULE				hTuner;
public:
	BENCH_SEC_SCRAMBLEMEMBER1
	Factory_Create*		pCreate;
	Factory_Destroy*	pDestroy;
	BOOL				tune_enabled;
	VTPause*			tune_pause	;
	VTResume*			tune_resume	;

	RendererSupport		TestRenderer		();
	void				CreateRendererList	(RendererSupport support);
	void				InitializeRenderer	(RendererSupport support);
	void				Initialize			();
	void				Destroy				();

	CEngineAPI	();
	~CEngineAPI	();
};

#define NEW_INSTANCE(a)		Engine.External.pCreate(a)
#define DEL_INSTANCE(a)		{ Engine.External.pDestroy(a); a=NULL; }

#endif // !defined(AFX_ENGINEAPI_H__CF21372B_C8B8_4891_82FC_D872C84E1DD4__INCLUDED_)
