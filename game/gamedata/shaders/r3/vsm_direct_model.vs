#include "common.h"
#include "skin.h"

struct v2p_smap
{
	float2 tc : TEXCOORD0;
	float4 hpos : SV_Position;
};

v2p_smap _main(v_model I)
{
	v2p_smap O;
	O.tc = I.tc;
	O.hpos = mul(m_WVP, I.P);

 	return O;
}

#ifdef 	SKIN_NONE
v2p_smap main(v_model v) 			{ return _main(v); }
#endif

#ifdef 	SKIN_0
v2p_smap main(v_model_skinned_0 v) 	{ return _main(skinning_0(v)); }
#endif

#ifdef	SKIN_1
v2p_smap main(v_model_skinned_1 v) 	{ return _main(skinning_1(v)); }
#endif

#ifdef	SKIN_2
v2p_smap main(v_model_skinned_2 v) 	{ return _main(skinning_2(v)); }
#endif

#ifdef	SKIN_3
v2p_smap main(v_model_skinned_3 v) 	{ return _main(skinning_3(v)); }
#endif

#ifdef	SKIN_4
v2p_smap main(v_model_skinned_4 v) 	{ return _main(skinning_4(v)); }
#endif
