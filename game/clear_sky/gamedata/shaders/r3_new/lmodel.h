#ifndef	LMODEL_H
#define LMODEL_H
	#include "common.h"

	float4 plight_infinity(float mtl, float3 position, float3 normal, float3 lightvec)
	{
		float3 vec2eye = -normalize(position); // vector to eye

		float3 vec2light = -lightvec; // vector to light

		float3 h = normalize(vec2light + vec2eye); // float-angle-vector 

		float2 lightspec = s_material.SampleLevel(smp_material, float3(dot(vec2light, normal), dot(h, normal), mtl), 0); // light.specular

		return lightspec.xxxy;
	}

	float4 plight_local(float mtl, float3 position, float3 normal, float3 lposition, float lightrangersq, out float3 lightvec)
	{
		float3 light2point = position - lposition; // light to point

		float3 vec2eye = -normalize(position); // vector to eye

		lightvec = normalize(light2point);

		float3 vec2light = -lightvec; // vector to light

		float rsqr = dot(light2point, light2point); // squared distance 2 light

		float att = saturate(1 - rsqr * lightrangersq); // q-linear attenuate

		float3 h = normalize(vec2light + vec2eye); // float-angle-vector 

		float2 lightspec = s_material.SampleLevel(smp_material, float3(dot(vec2light, normal), dot(h, normal), mtl), 0); // light.specular

		return lightspec.xxxy * att;
	}
#endif
