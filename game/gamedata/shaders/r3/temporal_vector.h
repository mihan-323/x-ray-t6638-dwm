#if !defined(TEMPORAL_VECTOR_INCLUDED_CHECK)
#define TEMPORAL_VECTOR_INCLUDED_CHECK

	cbuffer	taa_globals
	{
		float4 taa_jitter; // .xy - current jitter, .zw - previous jitter
	}

	// Add jitter for rasterizer pos
	void update_taa_vertex(inout float4 hpos)
	{
		#ifdef USE_TAA
			hpos.xy += taa_jitter.xy * hpos.ww;
		#endif
	}

	// UV offsets
	float2 taa_offset_curr() { return taa_jitter.xy * float2(-0.5,  0.5); }
	float2 taa_offset_prev() { return taa_jitter.zw * float2(-0.5,  0.5); }
#endif
