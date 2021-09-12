#include "common.h"

[maxvertexcount(3)]
void main(triangle v2p_TL I[3], inout TriangleStream<v2p_TL> triStream)
{
    v2p_TL O;

	O.Color = float4(1, 0, 0, 1);
	O.HPos = I[0].HPos;
	O.Tex0 = I[0].Tex0;
	triStream.Append(O);

	O.Color = float4(0, 1, 0, 1);
	O.HPos = I[1].HPos;
	O.Tex0 = I[1].Tex0;
	triStream.Append(O);

	O.Color = float4(0, 0, 1, 1);
	O.HPos = I[2].HPos;
	O.Tex0 = I[2].Tex0;
	triStream.Append(O);

    triStream.RestartStrip();
}