#include "common.h"
#include "skin.h"

v2p_rsmap _main(v_model I)
{
	v2p_rsmap	O ;

	O.P = mul(m_W, I.P);
	O.N = mul(m_W, I.N);
	O.tc0 = I.tc;
	O.hpos = mul(m_WVP, I.P);

 	return O;
}

#ifdef 	SKIN_NONE
v2p_rsmap 	main(v_model v) 			{ return _main(v); }
#endif

#ifdef 	SKIN_0
v2p_rsmap 	main(v_model_skinned_0 v) 	{ return _main(skinning_0(v)); }
#endif

#ifdef	SKIN_1
v2p_rsmap 	main(v_model_skinned_1 v) 	{ return _main(skinning_1(v)); }
#endif

#ifdef	SKIN_2
v2p_rsmap 	main(v_model_skinned_2 v) 	{ return _main(skinning_2(v)); }
#endif

#ifdef	SKIN_3
v2p_rsmap 	main(v_model_skinned_3 v) 	{ return _main(skinning_3(v)); }
#endif

#ifdef	SKIN_4
v2p_rsmap 	main(v_model_skinned_4 v) 	{ return _main(skinning_4(v)); }
#endif
