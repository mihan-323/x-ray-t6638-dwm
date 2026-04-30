#include "stdafx.h"
#include "render.h"

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

	u32 frames_split = r__cubemap_split; // [1..6]
	u32 frame_id = Device.dwFrame % frames_split;
	u32 frame_faces = 6 / frames_split;

	u32 frame_start = frame_id * frame_faces;
	u32 frame_end = frame_start + frame_faces;

	if (frames_split < 1 || frames_split > 6)
	{
		Msg("!Failed to draw cubemap, invalid split : %d", frames_split);
		frame_start = 0;
		frame_end = 0;
	}

	static Fmatrix m_view, m_view_proj;
	for (u32 face = frame_start;
		face < frame_end; face++)
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
