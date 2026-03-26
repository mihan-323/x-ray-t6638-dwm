#include "common.h"
#include "tess.h"

HS_CONSTANT_DATA_OUTPUT PatchConstantsHS(InputPatch <v2p_bumped, 3> ip, uint PatchID : SV_PrimitiveID)
{	
    HS_CONSTANT_DATA_OUTPUT Output;

	Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = 1;
	Output.Inside = 1;

	#ifdef TESS_PN
		ComputeTessFactor(Output.Edges, Output.Inside);

		float3 N[3] =
		{
			normalize(float3(ip[0].M1.z, ip[0].M2.z, ip[0].M3.z)),
			normalize(float3(ip[1].M1.z, ip[1].M2.z, ip[1].M3.z)),
			normalize(float3(ip[2].M1.z, ip[2].M2.z, ip[2].M3.z))
		};
		
		float3 P[3] = 
		{
			ip[0].position.xyz,
			ip[1].position.xyz,
			ip[2].position.xyz
		};
		
		ComputePNPatch(P, N, Output.patch);
		
		//Discard back facing patches
		// #ifndef TESS_HM
			// bool doDiscard = (N[0].z>0.1) && (N[1].z>0.1) && (N[2].z>0.1) 
					// && (Output.patch.f3N110.z>0.1) && (Output.patch.f3N011.z>0.1) && (Output.patch.f3N101.z>0.1)
					// && (P[0].z>5) && (P[1].z>5) && (P[2].z>5);
					
			// if (doDiscard)
				// Output.Edges[0]= Output.Edges[1]=Output.Edges[2]=Output.Inside=-1;
		// #endif
	#endif

	// dynamic tess
	#ifdef TESS_HM
		static float near = 0.2;
		static float far = 20.0;
		static float inside = 50;
		
		float factor[3];
		factor[0] = pow(smoothstep(far, near, ip[0].position.z), 5);
		factor[1] = pow(smoothstep(far, near, ip[1].position.z), 5);
		factor[2] = pow(smoothstep(far, near, ip[2].position.z), 5);
		
		// Output.Edges[0] += inside * factor[0];
		// Output.Edges[1] += inside * factor[1];
		// Output.Edges[2] += inside * factor[2];
		
		Output.Edges[0] += inside;
		Output.Edges[1] += inside;
		Output.Edges[2] += inside;
		
		Output.Inside += inside * 0.3333333f * (factor[0] + factor[1] + factor[2]);
	#endif
	
	//	Data for interpolation in screen space
	//	float w0 = mul(m_P, float4(ip[2].position.xyz, 1)).w;
	//	float w1 = mul(m_P, float4(ip[1].position.xyz, 1)).w;
	//	float w2 = mul(m_P, float4(ip[0].position.xyz, 1)).w;
		
	//	Output.www = float3(w0, w1, w2);

    return Output;
}

[domain("tri")]
[partitioning("pow2")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchConstantsHS")]
v2p_bumped main(InputPatch <v2p_bumped, 3> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID)
{
    v2p_bumped ouput;

	ouput.tcdh		= ip[i].tcdh;
	ouput.position	= ip[i].position;
	ouput.M1		= ip[i].M1;
	ouput.M2		= ip[i].M2;
	ouput.M3		= ip[i].M3;

	#ifdef USE_TDETAIL
		ouput.tcdbump	= ip[i].tcdbump;
	#endif

	#ifdef USE_LM_HEMI
		ouput.lmh		= ip[i].lmh;
	#endif

    return ouput;
}


