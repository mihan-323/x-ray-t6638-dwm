#pragma once

class CWaterRender
{
private:
	xr_vector<dxRender_Visual*> RawVisuals;

public:
	void LoadVisuals(xr_vector<dxRender_Visual*>& Visuals);
	void UnloadVisuals();
};