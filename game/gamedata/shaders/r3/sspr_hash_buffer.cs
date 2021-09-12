#include "common.h"
#define SSR_SKYBOX_USE
#include "reflections_dwm.h"

#ifdef SM_5_0

#define ALU_DIM 32
#define GROUP_THREAD_DIM 32 // 32 * 32 = 1024 threads

uniform RWTexture2D<uint> s_sspr : register(u0);

[numthreads(GROUP_THREAD_DIM, GROUP_THREAD_DIM, 1)]
void main(uint3 GID : SV_GroupID, uint3 GTID : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
	uint2 pos2d = uint2(GID.x * ALU_DIM + GTID.x, GID.y * ALU_DIM + GTID.y);

	if(is_in_quad(pos2d * screen_res.zw))
	{
		float h = planar_ssr_detect_water();
		uint2 pos2d_hit = planar_ssr_make_reflection(pos2d, h);
		uint pos2d_packed = pos2d.y << 16 | pos2d.x;
		InterlockedMax(s_sspr[pos2d_hit], pos2d_packed);
	}
}

#else

[numthreads(1, 1, 1)]
void main(uint3 GID : SV_GroupID, uint3 GTID : SV_GroupThreadID, uint GI : SV_GroupIndex)
{
}

#endif
