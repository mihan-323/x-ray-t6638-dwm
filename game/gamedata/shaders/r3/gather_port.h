#ifndef GATHER_PORT_H
#define GATHER_PORT_H
namespace GatherSoftwareImpl
{
	#ifndef SM_5_0
		float4 GatherRed(Texture2D s_current, SamplerState smp_current, float2 tc, int mip = 0, int2 offset = int2(0, 0))
		{
			return s_current.GatherRed(smp_current, tc, offset);
		}

		float4 GatherGreen(Texture2D s_current, SamplerState smp_current, float2 tc, int mip = 0, int2 offset = int2(0, 0))
		{
			return s_current.GatherGreen(smp_current, tc, offset);
		}

		float4 GatherBlue(Texture2D s_current, SamplerState smp_current, float2 tc, int mip = 0, int2 offset = int2(0, 0))
		{
			return s_current.GatherBlue(smp_current, tc, offset);
		}

		float4 GatherAlpha(Texture2D s_current, SamplerState smp_current, float2 tc, int mip = 0, int2 offset = int2(0, 0))
		{
			return s_current.GatherAlpha(smp_current, tc, offset);
		}
	#else
		float4 GatherRed(Texture2D s_current, SamplerState smp_current, float2 tc, int mip = 0, int2 offset = int2(0, 0))
		{
			float4 tex_res;
			uint levels;
			s_current.GetDimensions(mip, tex_res.x, tex_res.y, levels);
			tex_res.zw = rcp(tex_res.xy);
			float w = s_current.Sample(smp_current, tc + float2(0.0 - 0.5, 0.0 - 0.5) * tex_res.zw, offset).x;
			float z = s_current.Sample(smp_current, tc + float2(1.0 - 0.5, 0.0 - 0.5) * tex_res.zw, offset).x;
			float x = s_current.Sample(smp_current, tc + float2(0.0 - 0.5, 1.0 - 0.5) * tex_res.zw, offset).x;
			float y = s_current.Sample(smp_current, tc + float2(1.0 - 0.5, 1.0 - 0.5) * tex_res.zw, offset).x;
			return float4(x, y, z, w);
		}

		float4 GatherGreen(Texture2D s_current, SamplerState smp_current, float2 tc, int mip = 0, int2 offset = int2(0, 0))
		{
			float4 tex_res;
			uint levels;
			s_current.GetDimensions(mip, tex_res.x, tex_res.y, levels);
			tex_res.zw = rcp(tex_res.xy);
			float w = s_current.Sample(smp_current, tc + float2(0.0 - 0.5, 0.0 - 0.5) * tex_res.zw, offset).y;
			float z = s_current.Sample(smp_current, tc + float2(1.0 - 0.5, 0.0 - 0.5) * tex_res.zw, offset).y;
			float x = s_current.Sample(smp_current, tc + float2(0.0 - 0.5, 1.0 - 0.5) * tex_res.zw, offset).y;
			float y = s_current.Sample(smp_current, tc + float2(1.0 - 0.5, 1.0 - 0.5) * tex_res.zw, offset).y;
			return float4(x, y, z, w);
		}

		float4 GatherBlue(Texture2D s_current, SamplerState smp_current, float2 tc, int mip = 0, int2 offset = int2(0, 0))
		{
			float4 tex_res;
			uint levels;
			s_current.GetDimensions(mip, tex_res.x, tex_res.y, levels);
			tex_res.zw = rcp(tex_res.xy);
			float w = s_current.Sample(smp_current, tc + float2(0.0 - 0.5, 0.0 - 0.5) * tex_res.zw, offset).z;
			float z = s_current.Sample(smp_current, tc + float2(1.0 - 0.5, 0.0 - 0.5) * tex_res.zw, offset).z;
			float x = s_current.Sample(smp_current, tc + float2(0.0 - 0.5, 1.0 - 0.5) * tex_res.zw, offset).z;
			float y = s_current.Sample(smp_current, tc + float2(1.0 - 0.5, 1.0 - 0.5) * tex_res.zw, offset).z;
			return float4(x, y, z, w);
		}

		float4 GatherAlpha(Texture2D s_current, SamplerState smp_current, float2 tc, int mip = 0, int2 offset = int2(0, 0))
		{
			float4 tex_res;
			uint levels;
			s_current.GetDimensions(mip, tex_res.x, tex_res.y, levels);
			tex_res.zw = rcp(tex_res.xy);
			float w = s_current.Sample(smp_current, tc + float2(0.0 - 0.5, 0.0 - 0.5) * tex_res.zw, offset).w;
			float z = s_current.Sample(smp_current, tc + float2(1.0 - 0.5, 0.0 - 0.5) * tex_res.zw, offset).w;
			float x = s_current.Sample(smp_current, tc + float2(0.0 - 0.5, 1.0 - 0.5) * tex_res.zw, offset).w;
			float y = s_current.Sample(smp_current, tc + float2(1.0 - 0.5, 1.0 - 0.5) * tex_res.zw, offset).w;
			return float4(x, y, z, w);
		}
	#endif
}
#endif
