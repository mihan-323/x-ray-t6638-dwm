/* #include "common.h" */

/* #define sum3(a) In[0].##a * 0.3333 + In[1].##a * 0.3334 + In[2].##a * 0.3333 */

/* // Geometry */
/* [maxvertexcount (6)] */
/* void main(triangle v2p_flat In[3], inout TriangleStream<v2p_flat> triStream) */
/* { */
    /* v2p_flat Out[6]; */

    /* for(int v=0; v<3; v++) */
    /* { */
		/* Out[v].tcdh = In[v].tcdh; */
		/* Out[v].position = In[v].position; */
		/* Out[v].N = In[v].N; */
		/* Out[v].id = In[v].id; */
		/* Out[v].hpos = In[v].hpos; */
		/* Out[v].w_pos = In[v].w_pos; */
        /* triStream.Append( Out[v] ); */
    /* } */
	
	/* //----------------------------------- */
	/* // новая точка */
	/* // центр основного треугольника */
	
	/* Out[3].tcdh = sum3(tcdh); */
	/* Out[3].position = sum3(position); */
	/* Out[3].N = sum3(N); */
	/* Out[3].id = sum3(id); */
	
	/* float4 w_pos = sum3(w_pos); */
	/* float4 Pp = mul(m_VP, w_pos); */
	/* Out[3].hpos = Pp; */
	/* Out[3].w_pos = w_pos; */
	
	/* triStream.Append( Out[3] ); */
	
	/* //----------------------------------- */
	/* // новая точка */
	/* // вверх по нормали от предыдущей */
	
	/* Out[4].tcdh = Out[3].tcdh; */
	/* Out[4].position = Out[3].position; */
	/* Out[4].N = Out[3].N; */
	/* Out[4].id = Out[3].id; */
	
	/* w_pos.xyz += Out[3].N * 1; */
	/* Pp = mul(m_VP, w_pos); */
	/* Out[4].hpos = Pp; */
	/* Out[4].w_pos = w_pos; */
	
	/* triStream.Append( Out[4] ); */
	
	/* //----------------------------------- */
	/* // новая точка */
	/* // вбок по Х от предыдущей */
	
	/* Out[5].tcdh = Out[4].tcdh; */
	/* Out[5].position = Out[4].position; */
	/* Out[5].N = Out[4].N; */
	/* Out[5].id = Out[4].id; */
	
	/* w_pos.x += DEVY; */
	/* Pp = mul(m_VP, w_pos); */
	/* Out[5].hpos = Pp; */
	/* Out[5].w_pos = w_pos; */
	
	/* triStream.Append( Out[5] ); */
	
	/* //----------------------------------- */
	
    /* triStream.RestartStrip(); */
/* } */
#include "common.h"

[maxvertexcount(3)]
void main(triangle v2p_TL I[3], inout TriangleStream<v2p_TL> triStream)
{
    v2p_TL O;

	O.Color = I[0].Color;
	O.HPos = I[0].HPos;
	O.Tex0 = I[0].Tex0;
	triStream.Append(O);

	O.Color = I[1].Color;
	O.HPos = I[1].HPos;
	O.Tex0 = I[1].Tex0;
	triStream.Append(O);

	O.Color = I[2].Color;
	O.HPos = I[2].HPos;
	O.Tex0 = I[2].Tex0;
	triStream.Append(O);

    triStream.RestartStrip();
}