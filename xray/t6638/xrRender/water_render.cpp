#include "stdafx.h"
#include "fbasicvisual.h"
#include "water_render.h"

void CWaterRender::LoadVisuals(xr_vector<dxRender_Visual*>& Visuals)
{
	for each (dxRender_Visual* V in Visuals)
	{
		if (V->shader)
		{
			if (V->shader->E[0])
			{
				if (V->shader->E[0]->passes[0])
				{
					if (V->shader->E[0]->passes[0]->ps)
					{
						if (strcmp("water", V->shader->E[0]->passes[0]->ps->cName.c_str()) == 0)
						{
							//Msg("Catch water visual: (%f,%f,%f) : (%f,%f,%f)",
							//	V->vis.box.min.x, V->vis.box.min.y, V->vis.box.min.z,
							//	V->vis.box.max.x, V->vis.box.max.y, V->vis.box.max.z);
							RawVisuals.push_back(V);
						}
					}
				}
			}
		}
	}

	float max_h = 0.11f;
	for each (dxRender_Visual * V in RawVisuals)
	{
		float h = V->vis.box.max.y - V->vis.box.min.y;
		Msg("%c Load water, h=%.2f, max_h=%.2f",
			h > max_h ? '!' : '-', h, max_h);
	}
}

void CWaterRender::UnloadVisuals()
{
	RawVisuals.clear();
}
