// HOM.h: interface for the CHOM class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "../xrEngine/IGame_Persistent.h"

class occTri;

class CHOM  
#ifdef DEBUG
	: public pureRender
#endif
{
private:
	xrXRC					xrc;
	CDB::MODEL*				m_pModel;
	occTri*					m_pTris;
	BOOL					bEnabled;
	Fmatrix					m_xform;
	Fmatrix					m_xform_01;
#ifdef DEBUG
	u32						tris_in_frame_visible	;
	u32						tris_in_frame			;
#endif

	xrCriticalSection		MT;
	volatile u32			MT_frame_rendered;

	void					Render_DB	(CFrustum&	base);
public:
	void					Load		();
	void					Unload		();
	void					Render		(CFrustum&	base);
	void					Render_ZB	();
//	void					Debug		();

	void					occlude		(Fbox2&		space) { }
	void					Disable		();
	void					Enable		();

	bool MT_Synced() const
	{
		return MT_frame_rendered == Device.dwFrame;
	}

	void	__stdcall		MT_RENDER	();
	ICF	void				MT_SYNC		()			{ 
		if (MT_Synced())
			return;

		MT_RENDER			(); 
	}

	BOOL					visible		(vis_data&	vis);
	BOOL					visible		(Fbox3&		B);
	BOOL					visible		(sPoly&		P);
	BOOL					visible		(Fbox2&		B, float depth);	// viewport-space (0..1)

	CHOM	();
	~CHOM	();

#ifdef DEBUG
	virtual void			OnRender	();
			void			stats		();
#endif
};
