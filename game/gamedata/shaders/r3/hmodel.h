#ifndef HMODEL_H
#define HMODEL_H

	#include "common.h"

	#ifdef USE_SSAO_PATH_TRACING
		uniform Texture2D<float> s_ssao_pt;
	#else
		uniform Texture2D<float> s_ssao;
	#endif

	uniform float4 env_color; // color.w = lerper

	#ifdef USE_IL
		uniform Texture2D s_rsm;
	#endif

	void hmodel(out float3 hdiffuse, out float3 hspecular, G_BUFFER::GBD gbd, float2 tc)
	{
		#if (DX11_STATIC_DEFFERED_RENDERER == 1)
			gbd.mtl = xmaterial;
		#endif

		float ssao = 1;

		// Apply SSAO
		#ifdef USE_SSAO
			#ifdef USE_SSAO_PATH_TRACING
				ssao = s_ssao_pt.Sample(smp_nofilter, tc);
			#else
				ssao = s_ssao.Sample(smp_nofilter, tc);
			#endif
		#endif

		// hscale - something like diffuse reflection
		float3	nw		= G_BUFFER::vs_ws( gbd.N );
		float	hscale	= gbd.hemi * 1;	//. *        (.5h + .5h*nw.y);

		#ifdef USE_GAMMA_22
			hscale	= (hscale*hscale);        // make it more linear
		#endif

		// reflection vector
		float3	v2PntL	= normalize( gbd.P );
		float3	v2Pnt	= G_BUFFER::vs_ws( v2PntL );
		float3	vreflect= reflect( v2Pnt, nw );
		float	hspec	= .5h + .5h * dot( vreflect, v2Pnt );

		// material	// sample material
		float2	lightspec	= s_material.SampleLevel( smp_material, float3( hscale, hspec, gbd.mtl ), 0 );

		// diffuse color
		float3	e0d		= env_s0.SampleLevel(smp_base, nw, 0);
		float3	e1d		= env_s1.SampleLevel(smp_base, nw, 0);
		float3	env_d	= env_color.xyz * lerp( e0d, e1d, env_color.w );
				env_d	*= env_d;	// contrast
				hdiffuse = env_d * lightspec.xxx + L_ambient.rgb;

		// specular color
		vreflect.y      = vreflect.y*2-1;	// fake remapping
		float3	e0s		= env_s0.SampleLevel(smp_base, vreflect, 0);
		float3	e1s		= env_s1.SampleLevel(smp_base, vreflect, 0);
		float3	env_s	= env_color.xyz * lerp( e0s, e1s, env_color.w);
				env_s	*=env_s;	// contrast

		hspecular	= env_s*lightspec.y*gbd.gloss;                //*h*m*s        ;        //env_s        *light.w         * s;

		// float ssao = 1;

		// Apply SSAO
		// #ifdef USE_SSAO
			// #ifdef USE_SSAO_PATH_TRACING
				// ssao = s_ssao_pt.Sample(smp_nofilter, tc);
			// #else
				// ssao = s_ssao.Sample(smp_nofilter, tc);
			// #endif

			hspecular *= ssao;
			hdiffuse *= ssao;
		// #endif

		// Apply RSM
		#ifdef USE_IL
			#if RSM_DEBUG_VIEW == 1
				hdiffuse = 0; // test
				ssao = 1;
			#endif

			// float4 rsm = s_rsm.Sample(smp_rtlinear, tc) * ssao;
			//hdiffuse = lerp(hdiffuse + rsm.xyz * 0.25, rsm.xyz, rsm.w);
			// hdiffuse = max(rsm, hdiffuse);
		#endif

		#if SSAO_DEBUG_VIEW == 1
			hdiffuse = ssao;
			// hdiffuse = ssao * gbd.hemi;
		#endif
	}

#endif
