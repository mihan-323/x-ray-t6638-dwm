//---------------------------------------------------------------
// noise.h
//---------------------------------------------------------------
#ifndef NOISE_H
#define NOISE_H
namespace noise
{
	float compress(float jitter, uint compressed)
	{
		return compressed ? jitter * 0.5 + 0.5 : jitter;
	}

	// 4x4
	static const float noise2D4x4linear[4][4] =
	{{0.06,0.12,0.18,0.25},
	 {0.31,0.37,0.43,0.50},
	 {0.56,0.62,0.68,0.75},
	 {0.81,0.87,0.94,1.00}};

	static const float noise2D4x4random[4][4] = 
	{{0.04,0.75,0.18,1.00},
	 {0.25,0.70,0.38,0.89},
	 {0.64,0.28,0.13,0.50},
	 {0.81,0.42,0.94,0.37}};

	float get_4(in uint2 pos2d, in uint mode = 0, in uint compressed = 0)
	{
		uint2 id = pos2d % 4;
		float jitter = mode ? noise2D4x4random[id.y][id.x] : noise2D4x4linear[id.y][id.x];
		return compress(jitter, compressed);
	}

	// 6x6
	static const float noise2D6x6linear[6][6] =
	{{0.028,0.067,0.085,0.114,0.142,0.171},
	 {0.200,0.228,0.257,0.285,0.314,0.342},
	 {0.372,0.400,0.429,0.457,0.485,0.514},
	 {0.542,0.571,0.600,0.628,0.657,0.685},
	 {0.714,0.742,0.771,0.800,0.828,0.857},
	 {0.885,0.914,0.942,0.958,0.987,1.000}};

	static const float noise2D6x6random[6][6] = 
	{{0.45,0.21,0.38,0.82,0.12,0.05},
	 {0.25,1.00,0.38,0.20,0.00,0.54},
	 {0.77,0.98,0.52,0.50,0.64,0.70},
	 {0.13,0.28,0.81,0.12,0.05,0.64},
	 {0.42,0.94,0.37,0.04,0.75,0.18},
	 {0.87,0.56,0.24,1.00,0.78,0.14}};

	float get_6(in uint2 pos2d, in uint mode = 0, in uint compressed = 0)
	{
		uint2 id = pos2d % 6;
		float jitter = mode ? noise2D6x6random[id.y][id.x] : noise2D6x6linear[id.y][id.x];
		return compress(jitter, compressed);
	}

	// Random
	float get_random(in float2 tc, in uint compressed = 0)
	{
		float jitter = frac(sin(dot(tc, float2(12.9898, 78.233))) * 43758.5453);
		return compress(jitter, compressed);
	}

	//----------------------------------------------------------------------------------------
	//  1 out, 1 in...
	float hash11(float p)
	{
		p = frac(p * .1031);
		p *= p + 33.33;
		p *= p + p;
		return frac(p);
	}

	//----------------------------------------------------------------------------------------
	//  1 out, 2 in...
	float hash12(float2 p)
	{
		float3 p3  = frac(float3(p.xyx) * .1031);
		p3 += dot(p3, p3.yzx + 33.33);
		return frac((p3.x + p3.y) * p3.z);
	}

	//----------------------------------------------------------------------------------------
	//  1 out, 3 in...
	float hash13(float3 p3)
	{
		p3  = frac(p3 * .1031);
		p3 += dot(p3, p3.zyx + 31.32);
		return frac((p3.x + p3.y) * p3.z);
	}

	//----------------------------------------------------------------------------------------
	//  2 out, 1 in...
	float2 hash21(float p)
	{
		float3 p3 = frac(float3(p.xxx) * float3(.1031, .1030, .0973));
		p3 += dot(p3, p3.yzx + 33.33);
		return frac((p3.xx+p3.yz)*p3.zy);

	}

	//----------------------------------------------------------------------------------------
	///  2 out, 2 in...
	float2 hash22(float2 p)
	{
		float3 p3 = frac(float3(p.xyx) * float3(.1031, .1030, .0973));
		p3 += dot(p3, p3.yzx+33.33);
		return frac((p3.xx+p3.yz)*p3.zy);

	}

	//----------------------------------------------------------------------------------------
	///  2 out, 3 in...
	float2 hash23(float3 p3)
	{
		p3 = frac(p3 * float3(.1031, .1030, .0973));
		p3 += dot(p3, p3.yzx+33.33);
		return frac((p3.xx+p3.yz)*p3.zy);
	}

	//----------------------------------------------------------------------------------------
	//  3 out, 1 in...
	float3 hash31(float p)
	{
	   float3 p3 = frac(float3(p.xxx) * float3(.1031, .1030, .0973));
	   p3 += dot(p3, p3.yzx+33.33);
	   return frac((p3.xxy+p3.yzz)*p3.zyx); 
	}


	//----------------------------------------------------------------------------------------
	///  3 out, 2 in...
	float3 hash32(float2 p)
	{
		float3 p3 = frac(float3(p.xyx) * float3(.1031, .1030, .0973));
		p3 += dot(p3, p3.yxz+33.33);
		return frac((p3.xxy+p3.yzz)*p3.zyx);
	}

	//----------------------------------------------------------------------------------------
	///  3 out, 3 in...
	float3 hash33(float3 p3)
	{
		p3 = frac(p3 * float3(.1031, .1030, .0973));
		p3 += dot(p3, p3.yxz+33.33);
		return frac((p3.xxy + p3.yxx)*p3.zyx);

	}

	//----------------------------------------------------------------------------------------
	// 4 out, 1 in...
	float4 hash41(float p)
	{
		float4 p4 = frac(float4(p.xxxx) * float4(.1031, .1030, .0973, .1099));
		p4 += dot(p4, p4.wzxy+33.33);
		return frac((p4.xxyz+p4.yzzw)*p4.zywx);
		
	}

	//----------------------------------------------------------------------------------------
	// 4 out, 2 in...
	float4 hash42(float2 p)
	{
		float4 p4 = frac(float4(p.xyxy) * float4(.1031, .1030, .0973, .1099));
		p4 += dot(p4, p4.wzxy+33.33);
		return frac((p4.xxyz+p4.yzzw)*p4.zywx);

	}

	//----------------------------------------------------------------------------------------
	// 4 out, 3 in...
	float4 hash43(float3 p)
	{
		float4 p4 = frac(float4(p.xyzx) * float4(.1031, .1030, .0973, .1099));
		p4 += dot(p4, p4.wzxy+33.33);
		return frac((p4.xxyz+p4.yzzw)*p4.zywx);
	}

	//----------------------------------------------------------------------------------------
	// 4 out, 4 in...
	float4 hash44(float4 p4)
	{
		p4 = frac(p4  * float4(.1031, .1030, .0973, .1099));
		p4 += dot(p4, p4.wzxy+33.33);
		return frac((p4.xxyz+p4.yzzw)*p4.zywx);
	}
}
#endif