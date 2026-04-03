#include "stdafx.h"
#include "render.h"

void CRender::planar_render(ref_texture t_env_0, ref_texture t_env_1, Fvector4 env, Fvector4 amb)
{
	PIX_EVENT(render_planar_reflections);
	//if (!DEVX) return; // debug

	// save params
	Fmatrix m_view_projet_saved;
	m_view_projet_saved.set(Device.mFullTransform);

	Fmatrix m_view_saved;
	m_view_saved.set(RCache.get_xform_view());

	Fvector camera_position_saved;
	camera_position_saved.set(Device.vCameraPosition);

	Fvector camera_direction_saved;
	camera_direction_saved.set(Device.vCameraDirection);

	Fvector camera_top_saved;
	camera_top_saved.set(Device.vCameraTop);

	CFrustum frustum_saved = ViewBase;

	phase = PHASE_PLANAR;

	// prepare render space & set render targets
	float h = r__dbg_planar_h;

	// find nearest water height to camera
	if (0)
	{
		int id = 0;
		float dist = 10000.0f;

		bool need_hit = true;
		extern xr_vector<Fbox> water_level_bbox;
		for (int i = 0; i < water_level_bbox.size(); i++)
		{
			if (water_level_bbox[i].contains(Device.vCameraPosition))
			{
				need_hit = false;
				id = i;
				dist = -1.0f;
				break;
			}
		}

		if (need_hit)
		{
			Fvector hit_position;
			for (int i = 0; i < water_level_bbox.size(); i++)
			{
				Fvector bbox_center, camera_to_bbox_center;
				water_level_bbox[i].getcenter(bbox_center);
				camera_to_bbox_center.sub(bbox_center, Device.vCameraPosition);
				Fbox::ERP_Result hit = water_level_bbox[i].Pick2(
					Device.vCameraPosition, camera_to_bbox_center, hit_position);

				if (hit == Fbox::rpNone)
					continue;

				float dist1 = Device.vCameraPosition.distance_to(hit_position);

				if (dist1 < dist)
				{
					id = i;
					dist = dist1;
				}
			}
		}

		float h_max = water_level_bbox[id].max.y;
		float h_min = water_level_bbox[id].min.y;

		static int id_s = 0;
		if (id_s != id)
		{
			id_s = id;
			Msg("Catch nearest water to planar --> %d (%f:%f) --> %f",
				id_s, h_min, h_max, dist);
		}

		h = 0.5f * (h_max + h_min);
	}

	Device.vCameraPosition.y = -(Device.vCameraPosition.y - 2 * h);
	//Device.vCameraPosition_saved.y = Device.vCameraPosition.y; // for details
	Device.vCameraDirection.y = -Device.vCameraDirection.y;
	Device.vCameraTop.y = -Device.vCameraTop.y;

	Fmatrix m_view_new;
	m_view_new.build_camera_dir(Device.vCameraPosition, Device.vCameraDirection, Device.vCameraTop);
	RCache.set_xform_view(m_view_new);

	HOM.Disable();

	r_pmask(true, false);

	Fmatrix m_view_project_new;
	m_view_project_new.mul(Device.mProject, m_view_new);

	Device.mFullTransform.set(m_view_project_new);
	//Device.mFullTransform_saved.set(Device.mFullTransform);

	ViewBase.CreateFromMatrix(Device.mFullTransform, FRUSTUM_P_LRTB + FRUSTUM_P_FAR);

	u32 s = o.msaa_samples;
	u32 s1 = o.msaa_samples_reflections;

	// choose MSAA mode
	Target->u_setrt(s1 > 1 ?
		Target->rt_Planar_color_ms :
		Target->rt_Planar_color);

	Target->u_setzb(Target->rt_Planar_depth);

	RCache.clear_CurrentRenderTargetView(rgba_black);
	RCache.clear_CurrentDepthStencilView();

	D3D_VIEWPORT VP = { 0, 0, (float)Target->get_width(), (float)Target->get_height(), 0, 1.0f };
	RCache.set_Viewport(&VP);

	RCache.set_Stencil(FALSE);
	RCache.set_CullMode(D3D_CULL_BACK);

	float torch_state = 0;
	if (m_torch_enabled == TRUE) torch_state = 1;

	// calculate shadow map matrice
	Fmatrix xf_project;
	xf_project.set(m_planar_shadow_project);

	Fmatrix xf_invview;
	xf_invview.invert(m_view_new);

	Fmatrix m_shadow;
	m_shadow.mul(xf_project, xf_invview);
	m_shadow.mulB_44(m_planar_shadow_bias_t);

	// setup matrices
	m_planar_shadow.set(m_shadow);
	m_planar_vp_camera.set(m_view_projet_saved);

	// setup constants
	m_planar_env.set(env);
	m_planar_amb.set(amb);
	m_planar_bias.set(r__planar_bias_n, r__planar_bias_d, torch_state, h);

	// render sky
	g_pGamePersistent->Environment().RenderSky();
	g_pGamePersistent->Environment().RenderClouds();

	// render geometry & lighting
	render_main(m_view_project_new, false);

	r_dsgraph_render_graph(0);

	if (Details && opt(R__USE_PLANAR_DETAILS))
	{
		//Details->UpdateVisibleM(RDEVICE.vCameraPosition_saved, RDEVICE.mFullTransform_saved);
		Details->UpdateVisibleM(Device.vCameraPosition, Device.mFullTransform);
		Details->Render();
	}

	// reset render space & render params
	m_planar_env.set(0, 0, 0, 0);
	m_planar_amb.set(0, 0, 0, 0);
	m_planar_bias.set(0, 0, 0, 0);

	HOM.Enable();

	phase = PHASE_NORMAL;

	ViewBase = frustum_saved;

	Device.vCameraPosition.set(camera_position_saved);
	//Device.vCameraPosition_saved.set(Device.vCameraPosition);
	Device.vCameraDirection.set(camera_direction_saved);
	Device.vCameraTop.set(camera_top_saved);

	Device.mFullTransform.set(m_view_projet_saved);
	//Device.mFullTransform_saved.set(Device.mFullTransform);

	RCache.set_xform_view(m_view_saved);

	// Reset viewport from device params
	//VP.Width = (float)HW.m_ChainDesc.BufferDesc.Width;
	//VP.Height = (float)HW.m_ChainDesc.BufferDesc.Height;

	// Reset viewport from SSAA params
	VP.Width = (FLOAT)Target->rt_Generic_0->dwWidth;
	VP.Height = (FLOAT)Target->rt_Generic_0->dwHeight;

	RCache.set_Viewport(&VP);

	// MSAA partial
	if (s1 > 1 && s1 != s)
	{
		HW.pContext->ResolveSubresource(Target->rt_Planar_color->pTexture->surface_get(), 0,
			Target->rt_Planar_color_ms->pTexture->surface_get(), 0, DXGI_FORMAT_R8G8B8A8_UNORM);
	}
}

void CRender::planar_save_shadow(Fmatrix xf_project, Fmatrix bias_t)
{
	if (HW.FeatureLevel <= D3D_FEATURE_LEVEL_10_0) 
		return;

	HW.pContext->CopyResource(Target->rt_Planar_shadow->pTexture->surface_get(), Target->rt_smap_depth->pTexture->surface_get());

	m_planar_shadow_project.set(xf_project);
	m_planar_shadow_bias_t.set(bias_t);
}

Fvector cube_dir_array[6] =
{
	{ 1.0f,  0.0f,  0.0f},  // +X
	{-1.0f,  0.0f,  0.0f},  // -X
	{ 0.0f,  1.0f,  0.0f},  // +Y
	{ 0.0f, -1.0f,  0.0f},  // -Y
	{ 0.0f,  0.0f,  1.0f},  // +Z
	{ 0.0f,  0.0f, -1.0f},  // -Z
};

Fvector cube_top_array[6] =
{
	{ 0.0f,  1.0f,  0.0f},  // +X
	{ 0.0f,  1.0f,  0.0f},  // -X
	{ 0.0f,  0.0f, -1.0f},  // +Y
	{ 0.0f,  0.0f,  1.0f},  // -Y
	{ 0.0f,  1.0f,  0.0f},  // +Z
	{ 0.0f,  1.0f,  0.0f},  // -Z
};

void CRender::cubemap_render(Fvector4 env, Fvector4 amb)
{
	PIX_EVENT(render_cubemap_reflections);

	static Fmatrix m_view_proj_saved, m_view_saved, m_proj_saved;
	m_view_proj_saved.set(Device.mFullTransform);
	m_view_saved.set(RCache.get_xform_view());
	m_proj_saved.set(RCache.get_xform_project());

	static Fvector camera_direction_saved, camera_top_saved;
	camera_direction_saved.set(Device.vCameraDirection);
	camera_top_saved.set(Device.vCameraTop);

	CFrustum frustum_saved = ViewBase;

	phase = PHASE_PLANAR;

	static Fmatrix m_proj;
	m_proj.build_projection(deg2rad(90.0f), 1.0f, 0.1f, 100.0f);
	RCache.set_xform_project(m_proj);

	HOM.Disable();
	r_pmask(true, false);

	u32 size = o.cubemap_edge_size;
	D3D11_VIEWPORT VP = { 0.0f, 0.0f, (float)size, (float)size, 0.0f, 1.0f };
	RCache.set_Viewport(&VP);

	RCache.set_Stencil(FALSE);
	RCache.set_CullMode(D3D11_CULL_BACK);

	static Fmatrix m_view, m_view_proj;
	for (u32 face = 0; face < 6; face++)
	{
		PIX_EVENT(render_cubemap_reflections_face);

		Target->u_setrt(size, size, Target->rt_Cubemap[face], Target->rt_Cubemap_depth[face]);
		Target->u_setzb(Target->rt_Cubemap_depth_stencil);

		RCache.clear_CurrentDepthStencilView();

		m_view.build_camera_dir(Device.vCameraPosition, cube_dir_array[face], cube_top_array[face]);
		RCache.set_xform_view(m_view);

		m_view_proj.mul(m_proj, m_view);
		Device.mFullTransform.set(m_view_proj);

		ViewBase.CreateFromMatrix(Device.mFullTransform, FRUSTUM_P_LRTB + FRUSTUM_P_FAR);

		// calculate shadow map matrices
		Fmatrix xf_project;
		xf_project.set(m_planar_shadow_project);

		Fmatrix xf_invview;
		xf_invview.invert(m_view);

		Fmatrix m_shadow;
		m_shadow.mul(xf_project, xf_invview);
		m_shadow.mulB_44(m_planar_shadow_bias_t);

		m_planar_shadow.set(m_shadow);
		m_planar_vp_camera.set(m_view_proj_saved);

		// setup constants
		m_planar_env.set(env);
		m_planar_amb.set(amb);

		m_planar_bias.set(0.0f, 0.0f, 0.0f, 0.0f);

		// draw
		g_pGamePersistent->Environment().RenderSky();
		g_pGamePersistent->Environment().RenderClouds();

		render_main(m_view_proj, false);
		r_dsgraph_render_graph(0);

		if (Details && opt(R__USE_PLANAR_DETAILS))
		{
#ifdef CLEAR_SKY_BUILD
			Details->mFullTransform_saved.set(Device.mFullTransform);
#else
			Device.mFullTransform_saved.set(Device.mFullTransform);
#endif
			Details->UpdateVisibleM(Device.vCameraPosition, Device.mFullTransform);
			Details->Render();
		}
	}

	HOM.Enable();

	phase = PHASE_NORMAL;

	ViewBase = frustum_saved;

	Device.vCameraDirection.set(camera_direction_saved);
	Device.vCameraTop.set(camera_top_saved);

	Device.mFullTransform.set(m_view_proj_saved);

#ifdef CLEAR_SKY_BUILD
	Details->mFullTransform_saved.set(Device.mFullTransform);
#else
	Device.mFullTransform_saved.set(Device.mFullTransform);
#endif

	RCache.set_xform_view(m_view_saved);
	RCache.set_xform_project(m_proj_saved);

	VP.Width = (FLOAT)Target->rt_Generic_0->dwWidth;
	VP.Height = (FLOAT)Target->rt_Generic_0->dwHeight;
	RCache.set_Viewport(&VP);
}
