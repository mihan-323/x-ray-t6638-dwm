#ifndef MSAA_MARK_EDGES_H
#define MSAA_MARK_EDGES_H
#ifdef MSAA_SAMPLES
	#include "common.h"

	uint msaa_mark_edges(in float2 tc, in float4 pos2d)
	{
		float3 Ng = G_BUFFER::load_normal(tc, pos2d.xy);
		float3 N = Ng;

		[unroll(MSAA_SAMPLES)]
		for(uint i = 1; i < MSAA_SAMPLES; i++)
		{
			G_BUFFER::set_sampleid(i);
			float3 Nc = G_BUFFER::load_normal(tc, pos2d.xy);

			N += Nc;
		}

		N /= (float)MSAA_SAMPLES;

		return N == Ng;
	}

	uint calc_atoc_level_old(float alpha, uint2 pos2d)
	{
		float off = float((pos2d.x | pos2d.y) & 3);

		#if MSAA_SAMPLES == 2
			alpha = saturate(alpha - off * ((0.3333 / MSAA_SAMPLES) / 3.0));

					if(alpha < 0.3333)	return   0;
			else	if(alpha < 0.6666)	return   1;
			else						return   3;
		#elif MSAA_SAMPLES == 4
			alpha = saturate(alpha - off * ((0.2 / MSAA_SAMPLES) / 3.0));

					if(alpha < 0.2000)	return   0;
			else 	if(alpha < 0.4000)	return   1;
			else 	if(alpha < 0.6000)	return   3;
			else 	if(alpha < 0.8000)	return   7;
			else 						return  15;
		#elif MSAA_SAMPLES == 8
			alpha = saturate(alpha - off * ((0.1111 / MSAA_SAMPLES) / 3.0));

					if(alpha < 0.1111)	return   0;
			else 	if(alpha < 0.2222)	return   1;
			else 	if(alpha < 0.3333)	return   3;
			else 	if(alpha < 0.4444)	return   7;
			else 	if(alpha < 0.5555)	return  15;
			else 	if(alpha < 0.6666)	return  31;
			else 	if(alpha < 0.7777)	return  63;
			else 	if(alpha < 0.8888)	return 127;
			else 						return 255;
		#endif

		return 1;
	}

	// https://bgolus.medium.com/anti-aliased-alpha-test-the-esoteric-alpha-to-coverage-8b177335ae4f
	uint calc_atoc_level(float base, uint2 pos2d)
	{
		float2 dx = ddx((float2)pos2d);
		float2 dy = ddy((float2)pos2d);
		float delta_max_sqr = max(dot(dx, dx), dot(dy, dy));
		float mip_new = max(0.0, 0.5 * log2(delta_max_sqr));
		float alpha = base + base * max(0, mip_new) * 0.25;
		float absdfull = abs(ddx(alpha)) + abs(ddy(alpha));
		alpha = (alpha - def_aref) / max(absdfull, 0.0001) + 0.5;
		alpha = saturate(alpha);
		uint levels = uint(alpha * MSAA_SAMPLES);
		uint mask = 0;
		for(uint i = 0; i < levels && i < MSAA_SAMPLES; i++) mask |= 1 << i;
		return mask;
	}
#endif
#endif
