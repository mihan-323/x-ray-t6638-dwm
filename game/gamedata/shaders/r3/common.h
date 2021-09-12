#ifndef COMMON_H
#define COMMON_H

	#define USABLE_BIT_1	uint(0x00002000)
	#define USABLE_BIT_2	uint(0x00004000)
	#define USABLE_BIT_3	uint(0x00008000)
	#define USABLE_BIT_4	uint(0x00010000)
	#define USABLE_BIT_5	uint(0x00020000)
	#define USABLE_BIT_6	uint(0x00040000)
	#define USABLE_BIT_7	uint(0x00080000)
	#define USABLE_BIT_8	uint(0x00100000)
	#define USABLE_BIT_9	uint(0x00200000)
	#define USABLE_BIT_10	uint(0x00400000)
	#define USABLE_BIT_11	uint(0x00800000)   // At least two of those four bit flags must be mutually exclusive (i.e. all 4 bits must not be set together)
	#define USABLE_BIT_12	uint(0x01000000)   // This is because setting 0x47800000 sets all 5 FP16 exponent bits to 1 which means infinity
	#define USABLE_BIT_13	uint(0x02000000)   // This will be translated to a +/-MAX_FLOAT in the FP16 render target (0xFBFF/0x7BFF), overwriting the 
	#define USABLE_BIT_14	uint(0x04000000)   // mantissa bits where other bit flags are stored.
	#define USABLE_BIT_15	uint(0x80000000)
	#define MUST_BE_SET		uint(0x40000000)   // This flag *must* be stored in the floating-point representation of the bit flag to store

	#define pp_downsampling(tc, factor) \
		if(tc.x > factor || tc.y > factor) \
			discard; \
		else \
			tc /= factor

	#define M_PI 3.1415926

	#define step(y, x) (x > y)

	#define and &&
	#define or ||

	static const float3 LUMINANCE_VECTOR = float3(0.3f, 0.38f, 0.22f);

	static const float def_gloss   = 2.0f / 255;
	static const float def_aref    = 150.0f / 255;
	static const float def_hdr     = 9.0f;
	static const float def_distort = 0.05f;

	#define DEVX	developer_float4.x
	#define DEVY	developer_float4.y
	#define DEVZ	developer_float4.z
	#define DEVW	developer_float4.w
	uniform float4	developer_float4;

	#if (DX11_STATIC_DEFFERED_RENDERER == 1)
		#define xmaterial float(0.25)
	#else
		#define xmaterial float(L_material.w)
	#endif

	#ifndef SMAP_size
		#define SMAP_size 512
	#endif

	#include "noise.h"

	#include "poisson_disk.h"
	#include "common_define_2.h"

	#include "shared\common.h"

	#include "common_iostructs.h"
	#include "common_samplers.h"
	#include "common_cbuffers.h"
	#include "common_functions.h"
	#include "temporal_vector.h"
	#include "G_BUFFER.h"
	#include "shared\wmark.h"

#endif
