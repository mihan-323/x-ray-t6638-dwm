#include "common.h"

uniform float4x4 m_texgen;

v2p_volume main (float4 P: POSITION)
{
	v2p_volume O;

	O.hpos = mul(m_WVP, P);
	O.tc = mul(m_texgen, P);

 	return	O;
}
