#if !defined(TEMPORAL_VECTOR_INCLUDED_CHECK)
#define TEMPORAL_VECTOR_INCLUDED_CHECK

	cbuffer	taa_globals
	{
		float4 taa_jitter; // .xy - current jitter, .zw - previous jitter
	}

	// #define TAA_JITTER float4(taa_jitter * DEVX)
	#define TAA_JITTER float4(taa_jitter)

	// Add jitter for rasterizer pos
	void update_taa_vertex(inout float4 hpos)
	{
		#ifdef USE_TAA
			hpos.xy += TAA_JITTER.xy * hpos.ww;
			hpos.xy = hpos.xy * DEVX;
		#endif
	}

	// UV offsets
	float2 taa_offset_curr() { return TAA_JITTER.xy * float2(-0.5,  0.5); }
	//float2 taa_offset_prev() { return TAA_JITTER.zw * float2(-0.5,  0.5); }
#endif
