#ifndef	common_iostructs_h_included
#define	common_iostructs_h_included

////////////////////////////////////////////////////////////////
//	This file contains io structs:
//	v_name	:	input for vertex shader.
//	v2p_name:	output for vertex shader.
//	p_name	:	input for pixel shader.
////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
//	DWM new

struct v_water
{
	float4 P : POSITION;
	float4 N : NORMAL;
	float4 T : TANGENT;
	float4 B : BINORMAL;
	float3 C : COLOR0;
};

struct v2p_water
{
	float2 tbase : TEXCOORD0;
	float4 ttexgen : TEXCOORD1;
	float4 position_w : TEXCOORD2;
	float3 M1 : TEXCOORD3;
	float3 M2 : TEXCOORD4;
	float3 M3 : TEXCOORD5;
	float4 C : COLOR0;
	float  fog : FOG;
	float4 hpos : SV_Position;
};

////////////////////////////////////////////////////////////////
//	TL0uv
struct	v_TL0uv_positiont
{
	float4	P		: POSITIONT;
	float4	Color	: COLOR; 
};

struct	v_TL0uv
{
	float4	P		: POSITION;
	float4	Color	: COLOR; 
};

struct	v2p_TL0uv
{
	float4	Color	: COLOR;
	float4 	HPos	: SV_Position;	// Clip-space position 	(for rasterization)
};

struct	p_TL0uv
{
	float4	Color	: COLOR;
//	float4 	HPos	: SV_Position;	// Clip-space position 	(for rasterization)
};

////////////////////////////////////////////////////////////////
//	TL
struct	v_TL_positiont
{
	float4	P		: POSITIONT;
	float2	Tex0	: TEXCOORD0;
	float4	Color	: COLOR; 
};

struct	v_TL
{
	float4	P		: POSITION;
	float2	Tex0	: TEXCOORD0;
	float4	Color	: COLOR; 
};

struct	v2p_TL
{
	float2 	Tex0	: TEXCOORD0;
	float4	Color	: COLOR;
	float4 	HPos	: SV_Position;	// Clip-space position 	(for rasterization)
};

struct	p_TL
{
	float2 	Tex0	: TEXCOORD0;
	float4	Color	: COLOR;
//	float4 	HPos	: SV_Position;	// Clip-space position 	(for rasterization)
};

////////////////////////////////////////////////////////////////
//	TL2uv
struct	v_TL2uv
{
	float4	P		: POSITIONT;
	float2	Tex0	: TEXCOORD0;
	float2	Tex1	: TEXCOORD1;
	float4	Color	: COLOR; 
};

struct	v2p_TL2uv
{
	float2 	Tex0	: TEXCOORD0;
	float2	Tex1	: TEXCOORD1;
	float4	Color	: COLOR;
	float4 	HPos	: SV_Position;	// Clip-space position 	(for rasterization)
};

struct	p_TL2uv
{
	float2 	Tex0	: TEXCOORD0;
	float2	Tex1	: TEXCOORD1;
	float4	Color	: COLOR;
//	float4 	HPos	: SV_Position;	// Clip-space position 	(for rasterization)
};
////////////////////////////////////////////////////////////////
//	postpr
struct	v_postpr
{
	float4	P		: POSITIONT;
	float2 	Tex0	: TEXCOORD0;	// base1 (duality)	
	float2	Tex1	: TEXCOORD1;	// base2 (duality)
	float2	Tex2	: TEXCOORD2;	// base  (noise)
	float4	Color	: COLOR0;		// multiplier, color.w = noise_amount
	float4	Gray	: COLOR1;		// (.3,.3,.3.,amount)
};

struct	v2p_postpr
{
	float2 	Tex0	: TEXCOORD0;	// base1 (duality)	
	float2	Tex1	: TEXCOORD1;	// base2 (duality)
	float2	Tex2	: TEXCOORD2;	// base  (noise)
	float4	Color	: COLOR0;		// multiplier, color.w = noise_amount
	float4	Gray	: COLOR1;		// (.3,.3,.3.,amount)
	float4 	HPos	: SV_Position;	// Clip-space position 	(for rasterization)
};

struct	p_postpr
{
	float2 	Tex0	: TEXCOORD0;	// base1 (duality)	
	float2	Tex1	: TEXCOORD1;	// base2 (duality)
	float2	Tex2	: TEXCOORD2;	// base  (noise)
	float4	Color	: COLOR0;		// multiplier, color.w = noise_amount
	float4	Gray	: COLOR1;		// (.3,.3,.3.,amount)
//	float4 	HPos	: SV_Position;	// Clip-space position 	(for rasterization)
};
////////////////////////////////////////////////////////////////
//	build
struct	v_build
{
	float4	P		: POSITIONT;
	float2	Tex0	: TEXCOORD0;
	float2	Tex1	: TEXCOORD1;
	float2 	Tex2	: TEXCOORD2;
	float2	Tex3	: TEXCOORD3;
};

struct	v2p_build
{
	float2 	Tex0	: TEXCOORD0;
	float2	Tex1	: TEXCOORD1;
	float2 	Tex2	: TEXCOORD2;
	float2	Tex3	: TEXCOORD3;
	float4 	HPos	: SV_Position;	// Clip-space position 	(for rasterization)
};

struct	p_build
{
	float2 	Tex0	: TEXCOORD0;
	float2	Tex1	: TEXCOORD1;
	float2 	Tex2	: TEXCOORD2;
	float2	Tex3	: TEXCOORD3;
//	float4 	HPos	: SV_Position;	// Clip-space position 	(for rasterization)
};
////////////////////////////////////////////////////////////////
//	filter
struct	v_filter
{
	float4	P		: POSITIONT;
	float4 	Tex0	: TEXCOORD0;
	float4	Tex1	: TEXCOORD1;
	float4 	Tex2	: TEXCOORD2;
	float4	Tex3	: TEXCOORD3;
	float4 	Tex4	: TEXCOORD4;
	float4	Tex5	: TEXCOORD5;
	float4 	Tex6	: TEXCOORD6;
	float4	Tex7	: TEXCOORD7;
};

struct	v2p_filter
{
	float4 	Tex0	: TEXCOORD0;
	float4	Tex1	: TEXCOORD1;
	float4 	Tex2	: TEXCOORD2;
	float4	Tex3	: TEXCOORD3;
	float4 	Tex4	: TEXCOORD4;
	float4	Tex5	: TEXCOORD5;
	float4 	Tex6	: TEXCOORD6;
	float4	Tex7	: TEXCOORD7;
	float4 	HPos	: SV_Position;	// Clip-space position 	(for rasterization)
};

struct	p_filter
{
	float4 	Tex0	: TEXCOORD0;
	float4	Tex1	: TEXCOORD1;
	float4 	Tex2	: TEXCOORD2;
	float4	Tex3	: TEXCOORD3;
	float4 	Tex4	: TEXCOORD4;
	float4	Tex5	: TEXCOORD5;
	float4 	Tex6	: TEXCOORD6;
	float4	Tex7	: TEXCOORD7;
//	float4 	HPos	: SV_Position;	// Clip-space position 	(for rasterization)
};

////////////////////////////////////////////////////////////////
//	aa_AA
struct	v_aa_AA
{
	float4 P		:POSITIONT;
	float2 	Tex0	:TEXCOORD0;
	float2	Tex1	:TEXCOORD1;
	float2 	Tex2	:TEXCOORD2;
	float2	Tex3	:TEXCOORD3;
	float2	Tex4	:TEXCOORD4;
	float4	Tex5	:TEXCOORD5;
	float4	Tex6	:TEXCOORD6;
};

struct	v2p_aa_AA
{
	float2 	Tex0	:TEXCOORD0;
	float2	Tex1	:TEXCOORD1;
	float2 	Tex2	:TEXCOORD2;
	float2	Tex3	:TEXCOORD3;
	float2	Tex4	:TEXCOORD4;
	float4	Tex5	:TEXCOORD5;
	float4	Tex6	:TEXCOORD6;
	float4 	HPos	:SV_Position;	// Clip-space position 	(for rasterization)
};

struct	p_aa_AA
{
	float2 	Tex0	:TEXCOORD0;
	float2	Tex1	:TEXCOORD1;
	float2 	Tex2	:TEXCOORD2;
	float2	Tex3	:TEXCOORD3;
	float2	Tex4	:TEXCOORD4;
	float4	Tex5	:TEXCOORD5;
	float4	Tex6	:TEXCOORD6;
//	float4 	HPos	:SV_Position;	// Clip-space position 	(for rasterization)
};

struct	p_aa_AA_sun
{
	float2 	tc		:TEXCOORD0;
	float2	unused	:TEXCOORD1;
	float2 	LT		:TEXCOORD2;
	float2	RT		:TEXCOORD3;
	float2	LB		:TEXCOORD4;
	float2	RB		:TEXCOORD5;
//	float4 	HPos	:SV_Position;	// Clip-space position 	(for rasterization)
};

////////////////////////////////////////////////////////////////
//	dumb
struct 	v_dumb
{
	float4	P		:POSITION;	// Clip-space position 	(for rasterization)
};

struct 	v2p_dumb
{
	float4	HPos	:SV_Position;	// Clip-space position 	(for rasterization)
};

////////////////////////////////////////////////////////////////
//	Volume
struct 	v2p_volume
{
	float4 	tc		:TEXCOORD0;
	float4 	hpos	:SV_Position;	// Clip-space position 	(for rasterization)
};
struct 	v2p_volume_
{
	float4 	tc		:TEXCOORD0;
	float4 	tcJ		:TEXCOORD1;
	float4  tcb     : TEXCOORD2;
	float4 	hpos	:SV_Position;	// Clip-space position 	(for rasterization)
};
struct 	p_volume
{
	float4 	tc		:TEXCOORD0;
//	float4 	hpos	:SV_Position;	// Clip-space position 	(for rasterization)
};
////////////////////////////////////////////////////////////////
//	Static
struct         v_static
{
	float4	Nh		:NORMAL;	// (nx,ny,nz,hemi occlusion)
	float4	T		:TANGENT;	// tangent
	float4	B		:BINORMAL;	// binormal
	int2	tc		:TEXCOORD0;	// (u,v)
#ifdef	USE_LM_HEMI
	int2	lmh		:TEXCOORD1;	// (lmu,lmv)
#endif
//	float4	color	:COLOR0;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!
	float4	P		:POSITION;	// (float,float,float,1)
};

struct	v_static_color
{
	float4	Nh		:NORMAL;	// (nx,ny,nz,hemi occlusion)
	float4	T		:TANGENT;	// tangent
	float4	B		:BINORMAL;	// binormal
	int2	tc		:TEXCOORD0;	// (u,v)
#ifdef	USE_LM_HEMI
	int2	lmh		:TEXCOORD1;	// (lmu,lmv)
#endif
	float4	color	:COLOR0;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!
	float4	P		:POSITION;	// (float,float,float,1)
};

////////////////////////////////////////////////////////////////
//	Defer bumped
struct v2p_bumped
{
	#if (DX11_STATIC_DEFFERED_RENDERER == 1) && !defined(USE_LM_HEMI)
		float4	tcdh	: TEXCOORD0;	// Texture coordinates,    w=sun_occlusion
	#else
		float2	tcdh	: TEXCOORD0;	// Texture coordinates
	#endif
	float4	position: TEXCOORD1;	// position + hemi
	float3	M1		: TEXCOORD2;	// nmap 2 eye - 1
	float3	M2		: TEXCOORD3;	// nmap 2 eye - 2
	float3	M3		: TEXCOORD4;	// nmap 2 eye - 3
	#ifdef USE_TDETAIL
		float2	tcdbump	: TEXCOORD5;	// d-bump
	#endif
	#ifdef USE_LM_HEMI
		float2	lmh	: TEXCOORD6;	// lm-hemi
	#endif
	uint id : TEXCOORD7;
	float4	hpos	: SV_Position;
};
////////////////////////////////////////////////////////////////
//	Planar
struct	v2p_planar
{
	float2	tcdh 		: TEXCOORD0;	// base tc
	float4	position 	: TEXCOORD1;	// vs position, ws h
	float3  light_sun	: TEXCOORD2;	// sun light
	float3  light_spot 	: TEXCOORD3;	// actor torch light
	float3  diffuse	 	: TEXCOORD4;	// hemi
	float4	hpos	 	: SV_Position;
};
////////////////////////////////////////////////////////////////
//	Defer flat
struct	v2p_flat
{
	#if (DX11_STATIC_DEFFERED_RENDERER == 1) && !defined(USE_LM_HEMI)
		float4	tcdh	: TEXCOORD0;	// Texture coordinates,   w=sun_occlusion
	#else
		float2	tcdh	: TEXCOORD0;	// Texture coordinates
	#endif
	float4	position: TEXCOORD1;	// position + hemi
	float3	N		: TEXCOORD2;	// Eye-space normal        (for lighting)
	#ifdef USE_TDETAIL
		float2	tcdbump	: TEXCOORD3;	// d-bump
	#endif
	#ifdef USE_LM_HEMI
		float2	lmh		: TEXCOORD4;	// lm-hemi
	#endif
	// ---
	// ---
	uint id : TEXCOORD6;
	float4	hpos	: SV_Position;
};
////////////////////////////////////////////////////////////////
//	Shadow
struct	v_shadow_direct_aref
{
	float4	P		: POSITION;		// (float,float,float,1)
	int4	tc		: TEXCOORD0;	// (u,v,frac,???)
};

struct	v_shadow_direct
{
	float4	P		: POSITION;		// (float,float,float,1)
};

struct	v2p_shadow_direct_aref
{
	float2	tc0		: TEXCOORD1;	// Diffuse map for aref
#ifdef NEED_SHADOW_MULTISAMPLING
	float	depth	: TEXCOORD2;	// 
#endif
	float4	hpos	: SV_Position;	// Clip-space position         (for rasterization)
};

struct	v2p_shadow_direct
{
#ifdef NEED_SHADOW_MULTISAMPLING
	float	depth	: TEXCOORD2;	// 
#endif
	float4	hpos	: SV_Position;		// Clip-space position         (for rasterization)
};

////////////////////////////////////////////////////////////////
//	RSM

struct	v_rsmap_aref
{
	float4	Nh		: NORMAL;		// (nx,ny,nz,hemi occlusion)
	int4	tc		: TEXCOORD0;	// (u,v,frac,???)
	float4	P		: POSITION;		// (float,float,float,1)
};

struct	v2p_rsmap
{
	float3  P 		: TEXCOORD0;
	float3  N 		: TEXCOORD1;
	float2	tc0		: TEXCOORD2;		
	float4	hpos	: SV_Position;		// Clip-space position         (for rasterization)
};

struct	f_rsmap
{
	float4 P : SV_Target0;
	float4 N : SV_Target1;
	float4 C : SV_Target2;
};

////////////////////////////////////////////////////////////////
//	Model
struct	v_model
{
	float4	P		: POSITION;		// (float,float,float,1)
	float3	N		: NORMAL;		// (nx,ny,nz)
	float3	T		: TANGENT;		// (nx,ny,nz)
	float3	B		: BINORMAL;		// (nx,ny,nz)
	float2	tc		: TEXCOORD0;	// (u,v)
};

////////////////////////////////////////////////////////////////
//	Tree
struct	v_tree
{
	//float4	color	:COLOR0;	// (r,g,b,dir-occlusion)	//	Swizzle before use!!!
	float4	P		: POSITION;		// (float,float,float,1)
	float4	Nh		: NORMAL;		// (nx,ny,nz)
	float3	T		: TANGENT;		// tangent
	float3	B		: BINORMAL;		// binormal
	int4	tc		: TEXCOORD0;	// (u,v,frac,???)
};

////////////////////////////////////////////////////////////////
//	Details
struct        v_detail                    
{
        float4      pos                : POSITION;                // (float,float,float,1)
        int4        misc        : TEXCOORD0;        // (u(Q),v(Q),frac,matrix-id)
};

#endif	//	common_iostructs_h_included