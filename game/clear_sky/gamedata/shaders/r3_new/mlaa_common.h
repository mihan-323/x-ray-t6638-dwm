#if !defined(MLAA_COMMON_H)
#define MLAA_COMMON_H
	#include "common.h"

	#define MLAA_USE_GATHER 	1
	#define MLAA_USE_STENCIL 	0
	#define MLAA_SHOW_EDGES 	0

	Texture2D<uint>  s_edge_mask	: register(t1);
	Texture2D<uint2> s_edge_count	: register(t2);

	// This constant defines the luminance intensity difference to check for when testing any two pixels for an edge.
	// The higher the value the fewer edges wil be detected.
	static const float kMLAAThreshold = 0.2;

	// Set the number of bits to use when storing the horizontal and vertical counts
	// This number should be half the number of bits in the color channels used
	// E.g. with a RT format of DXGI_R8G8_int this number should be 8/2 = 4
	// Longer edges can be detected by increasing this number; however this requires a 
	// larger bit depth format, and also makes the edge length detection function slower
	static const uint kNumCountBits = 4;

	// The maximum edge length that can be detected
	static const uint kMaxEdgeLength = ((1 << (kNumCountBits - 1)) - 1);

	// Various constants used by the shaders below
	static const uint kUpperMask				= (1 << 0);
	static const uint kUpperMask_BitPosition	= 0;
	static const uint kRightMask				= (1 << 1);
	static const uint kRightMask_BitPosition	= 1;
	static const uint kStopBit					= (1 << (kNumCountBits - 1));
	static const uint kStopBit_BitPosition		= (kNumCountBits - 1);
	static const uint kNegCountShift			= (kNumCountBits);
	static const uint kPosCountShift			= (00);
	static const uint kCountShiftMask			= ((1 << kNumCountBits) - 1);

	static const int3 kZero		= int3( 0,  0, 0);
	static const int3 kUp		= int3( 0, -1, 0);
	static const int3 kDown		= int3( 0,  1, 0);
	static const int3 kRight	= int3( 1,  0, 0);
	static const int3 kLeft		= int3(-1,  0, 0);

	bool CompareColors(float a, float b)
	{
		return ( abs(a - b)  > kMLAAThreshold );
	}
	bool2 CompareColors2(float2 a, float2 b)
	{
		return ( abs(a - b)  > kMLAAThreshold );
	}

	// Check if the specified bit is set
	bool IsBitSet(uint Value, const uint uBitPosition)
	{
		return ((Value & (1<<uBitPosition)) ? true : false);
	}	
	
	uint RemoveStopBit(uint a)
	{
		return a & (kStopBit-1);
	}
	
	uint DecodeCountNoStopBit(uint count, uint shift)
	{
		return RemoveStopBit((count >> shift) & kCountShiftMask);
	}
	
	uint DecodeCount(uint count, uint shift)
	{
		return (count >> shift) & kCountShiftMask;
	}
	
	uint EncodeCount(uint negCount, uint posCount)
	{
		return ((negCount & kCountShiftMask) << kNegCountShift) | (posCount & kCountShiftMask);
	}
	
	uint EncodeMaskColor(uint mask)
	{
		return uint(mask);		
	}
	
	uint DecodeMaskColor(uint mask)
	{
		return uint(mask);		
	}
	
	uint4 DecodeMaskColor4(uint4 mask)
	{
		return uint4(mask);		
	}	
	
	uint EncodeCountColor(uint count)
	{
		return uint(count);
	}
	
	uint DecodeCountColor(uint count)
	{
		return uint(count);
	}
	
	uint2 DecodeCountColor2(uint2 count)
	{
		return uint2(count);
	}
#endif