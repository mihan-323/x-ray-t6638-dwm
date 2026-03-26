#include "common.h"
#include "tess.h"

HS_CONSTANT_DATA_OUTPUT PatchConstantsHS(InputPatch <v2p_bumped, 3> ip, uint PatchID : SV_PrimitiveID)
{	
    HS_CONSTANT_DATA_OUTPUT Output;

	Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = 1;
	Output.Inside = 1;

	#ifdef TESS_TERRAIN

		float near = 0.2;
		float far = 20.0;
		float inside = 0;
		
		float factor[3];
		factor[0] = pow(smoothstep(far, near, ip[0].position.z), 5);
		factor[1] = pow(smoothstep(far, near, ip[1].position.z), 5);
		factor[2] = pow(smoothstep(far, near, ip[2].position.z), 5);
		
		Output.Edges[0] += inside * factor[0];
		Output.Edges[1] += inside * factor[1];
		Output.Edges[2] += inside * factor[2];
		
		Output.Inside += inside * 0.3333333f * (factor[0] + factor[1] + factor[2]);

	#endif

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


