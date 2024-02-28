#ifndef GATHER_PORT_H
#define GATHER_PORT_H
namespace GatherSoftwareImpl
{
	#ifndef SM_5_0
		float4 GatherRed  (Texture2D s_current,SamplerState smp_current,float2 tc,int mip=0,int2 offset=int2(0,0)){
			return s_current.GatherRed  (smp_current,tc,offset);}
		float4 GatherGreen(Texture2D s_current,SamplerState smp_current,float2 tc,int mip=0,int2 offset=int2(0,0)){
			return s_current.GatherGreen(smp_current,tc,offset);}
		float4 GatherBlue (Texture2D s_current,SamplerState smp_current,float2 tc,int mip=0,int2 offset=int2(0,0)){
			return s_current.GatherBlue (smp_current,tc,offset);}
		float4 GatherAlpha(Texture2D s_current,SamplerState smp_current,float2 tc,int mip=0,int2 offset=int2(0,0)){
			return s_current.GatherAlpha(smp_current,tc,offset);}
	#else
		#define _Gather(result,s_current,smp_current,tc,mip,offset,ch)\
				float4 tex_res;uint levels;float w,z,x,y;\
				s_current.GetDimensions(mip,tex_res.x,tex_res.y,levels);\
				tex_res.zw=rcp(tex_res.xy);\
				w=s_current.Sample(smp_current,tc+float2(0.0-0.5,0.0-0.5)*tex_res.zw,offset).##ch ;\
				z=s_current.Sample(smp_current,tc+float2(1.0-0.5,0.0-0.5)*tex_res.zw,offset).##ch ;\
				x=s_current.Sample(smp_current,tc+float2(0.0-0.5,1.0-0.5)*tex_res.zw,offset).##ch ;\
				y=s_current.Sample(smp_current,tc+float2(1.0-0.5,1.0-0.5)*tex_res.zw,offset).##ch ;\
				result=float4(x,y,z,w);
		float4 GatherRed  (Texture2D s_current,SamplerState smp_current,float2 tc,int mip=0,int2 offset=int2(0,0)){
			float4 t=0;_Gather(t,s_current,smp_current,tc,mip,offset,x);return t;}
		float4 GatherGreen(Texture2D s_current,SamplerState smp_current,float2 tc,int mip=0,int2 offset=int2(0,0)){
			float4 t=0;_Gather(t,s_current,smp_current,tc,mip,offset,y);return t;}
		float4 GatherBlue (Texture2D s_current,SamplerState smp_current,float2 tc,int mip=0,int2 offset=int2(0,0)){
			float4 t=0;_Gather(t,s_current,smp_current,tc,mip,offset,z);return t;}
		float4 GatherAlpha(Texture2D s_current,SamplerState smp_current,float2 tc,int mip=0,int2 offset=int2(0,0)){
			float4 t=0;_Gather(t,s_current,smp_current,tc,mip,offset,w);return t;}
	#endif
}
#endif
