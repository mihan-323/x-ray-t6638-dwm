#ifndef TESS_H_INCLUDED
#define TESS_H_INCLUDED

	// Output patch constant data.
	struct PNPatch
	{
		// Geometry cubic control points (excluding corners)
		float3 f3B210	: POSITION3;
		float3 f3B120	: POSITION4;
		float3 f3B021	: POSITION5;
		float3 f3B012	: POSITION6;
		float3 f3B102	: POSITION7;
		float3 f3B201	: POSITION8;
		float3 f3B111	: CENTER;
			
		// Normal quadratic control points (excluding corners)
		float3 f3N110	: NORMAL3;  	
		float3 f3N011	: NORMAL4;
		float3 f3N101	: NORMAL5;
	};

	struct HS_CONSTANT_DATA_OUTPUT
	{
		float	Edges[3]	: SV_TessFactor;
		float	Inside		: SV_InsideTessFactor;
		#ifdef TESS_PN	
			PNPatch	patch;
		#endif
	};

	float triLOD;

	void ComputeTessFactor(out float Edges[3] : SV_TessFactor, out float Inside : SV_InsideTessFactor)
	{
		Edges[0] = Edges[1] = Edges[2] = triLOD;
		Inside = triLOD;
	}

	void ComputePNPatch(float3 P[3], float3 N[3], out PNPatch patch)
	{
		// Compute the cubic geometry control points
		// Edge control points
		patch.f3B210 = (2.0f*P[0].xyz + P[1].xyz - dot(P[1].xyz-P[0].xyz, N[0])*N[0]) / 3.0f;
		patch.f3B120 = (2.0f*P[1].xyz + P[0].xyz - dot(P[0].xyz-P[1].xyz, N[1])*N[1]) / 3.0f;
		patch.f3B021 = (2.0f*P[1].xyz + P[2].xyz - dot(P[2].xyz-P[1].xyz, N[1])*N[1]) / 3.0f;
		patch.f3B012 = (2.0f*P[2].xyz + P[1].xyz - dot(P[1].xyz-P[2].xyz, N[2])*N[2]) / 3.0f;
		patch.f3B102 = (2.0f*P[2].xyz + P[0].xyz - dot(P[0].xyz-P[2].xyz, N[2])*N[2]) / 3.0f;
		patch.f3B201 = (2.0f*P[0].xyz + P[2].xyz - dot(P[2].xyz-P[0].xyz, N[0])*N[0]) / 3.0f;
		
		// Center control point
		float3 f3E = ( patch.f3B210 + patch.f3B120 + patch.f3B021 + patch.f3B012 + patch.f3B102 + patch.f3B201 ) / 6.0f;
		float3 f3V = ( P[0].xyz + P[1].xyz + P[2].xyz ) / 3.0f;
		patch.f3B111 = f3E + ( ( f3E - f3V ) / 2.0f );
		
		// Compute the quadratic normal control points, and rotate into world space
		float fV12 = 2.0f * dot( P[1].xyz - P[0].xyz, N[0] + N[1] ) / dot( P[1].xyz - P[0].xyz, P[1].xyz - P[0].xyz );
		patch.f3N110 = normalize( N[0] + N[1] - fV12 * ( P[1].xyz - P[0].xyz ));
		float fV23 = 2.0f * dot( P[2].xyz - P[1].xyz, N[1] + N[2] ) / dot( P[2].xyz - P[1].xyz, P[2].xyz - P[1].xyz );
		patch.f3N011 = normalize( N[1] + N[2] - fV23 * ( P[2].xyz - P[1].xyz ));
		float fV31 = 2.0f * dot( P[0].xyz - P[2].xyz, N[2] + N[0] ) / dot( P[0].xyz - P[2].xyz, P[0].xyz - P[2].xyz );
		patch.f3N101 = normalize( N[2] + N[0] - fV31 * ( P[0].xyz - P[2].xyz ));	
	}

	void ComputePatchVertex(float3 P[3], float3 N[3], float3 uvw, in PNPatch patch, out float3 Pos, out float3 Norm)
	{
		float u = uvw.y;
		float v = uvw.x;
		float w = uvw.z;

		Pos =	P[0] * w * w * w +
				P[1] * u * u * u +
				P[2] * v * v * v +
				patch.f3B210 * 3.0f * w * w * u +
				patch.f3B120 * 3.0f * w * u * u +
				patch.f3B201 * 3.0f * w * w * v +
				patch.f3B021 * 3.0f * u * u * v +
				patch.f3B102 * 3.0f * w * v * v +
				patch.f3B012 * 3.0f * u * v * v +
				patch.f3B111 * 6.0f * w * u * v;
			

		// Compute normal from quadratic control points and barycentric coords
		Norm =	N[0] * w * w +
				N[1] * u * u +
				N[2] * v * v +
				patch.f3N110 * w * u +
				patch.f3N011 * u * v +
				patch.f3N101 * w * v;
	}

	#ifdef USE_TDETAIL
		uniform SamplerState smp_bump_ds;
		uniform Texture2D s_tbumpX, s_tdetailBumpX; // smp_bump_ds

		static const float tess_height = 0.07;

		#ifdef TESS_4_DETAIL
			uniform Texture2D s_tmask, s_tdx_r, s_tdx_g, s_tdx_b, s_tdx_a;
		#endif

		void ComputeDisplacedVertex(inout float3 P, float3 N, float2 tc, float2 tcd)
		{
			#ifdef TESS_4_DETAIL
				float4 mask = s_tmask.SampleLevel(smp_bump_ds, tc, 0);

				float height = 0;
				height += s_tdx_r.SampleLevel(smp_bump_ds, tc * dt_params, 0).w * mask.x;
				height += s_tdx_g.SampleLevel(smp_bump_ds, tc * dt_params, 0).w * mask.y;
				height += s_tdx_b.SampleLevel(smp_bump_ds, tc * dt_params, 0).w * mask.z;
				height += s_tdx_a.SampleLevel(smp_bump_ds, tc * dt_params, 0).w * mask.w;

				P += N * height * tess_height * 1;
			#else
				float height = s_tbumpX.SampleLevel(smp_bump_ds, tc, 0).w;

				#ifdef USE_TDETAIL_BUMP
					height += s_tdetailBumpX.SampleLevel(smp_bump_ds, tcd, 0).w * 0.2 - 0.01;
				#endif

				P += N * height * tess_height;
			#endif
		}
	#endif
#endif
