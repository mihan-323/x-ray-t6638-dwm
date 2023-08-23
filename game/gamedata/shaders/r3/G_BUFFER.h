#ifndef	G_BUFFER_INCLUDED
#define	G_BUFFER_INCLUDED

	#ifdef MSAA_SAMPLES
		uniform Texture2DMS<float4, MSAA_SAMPLES> s_diffuse_ms;
		uniform Texture2DMS<float4, MSAA_SAMPLES> s_position;
		uniform Texture2DMS<float4, MSAA_SAMPLES> s_position_prev;
		uniform Texture2DMS<float4, MSAA_SAMPLES> s_accumulator;
	#else
		uniform Texture2D s_diffuse;
		uniform Texture2D s_position;
		uniform Texture2D s_position_prev;
		uniform Texture2D s_accumulator;
	#endif

	#ifdef MSAA_SAMPLES
		uniform Texture2DMS<float, MSAA_SAMPLES> s_depthms;
	#else
		uniform Texture2D<float> s_depth;
	#endif

	#ifdef USE_CSPECULAR
		#ifdef MSAA_SAMPLES
			uniform Texture2DMS<float4, MSAA_SAMPLES> s_accumulator_1;
		#else
			uniform Texture2D s_accumulator_1;
		#endif
	#endif

	uniform float4 depth_params; // near, far, 0, planar height
	uniform float3x4 m_v2w; // View to World

	namespace G_BUFFER
	{
		static const float sky = 1000.0;
		static const float eps = 0.01;
		static const float2 InvFocalLen = float2(1, screen_res.y * screen_res.z) * pos_decompression_params.x;

		#ifdef MSAA_SAMPLES
			static uint sampleid = 0;
			static uint atoc = (1 << MSAA_SAMPLES) - 1;
		#endif

		// point filtering by default
		static bool bilinear_f_use = false;

		struct GBD
		{
			float3  P; 		// position
			float   mtl; 	// mtl or sun
			float3  N; 		// normal
			float   hemi; 	// ambient occ
			float3  C;		// color
			float   gloss;	// gloss
			uint	mask;	// mask
		};

		struct DEFFER
		{
			float4	position	: SV_Target0; // xy=encoded normal, z = pz, w = encoded(m-id,hemi)
			float4	C			: SV_Target1; // r, g, b, gloss
			#if defined(MSAA_SAMPLES) && !defined(SM_4_0)
				uint atoc_out	: SV_Coverage;
			#endif
			// #if defined(USE_TAA) || defined(USE_TXAA)
				// float2 motion	: SV_Target2;
			// #endif
		};

		struct ACCUMULATOR
		{
			#ifdef USE_CSPECULAR
				float3 L : SV_Target0; // light
				float3 S : SV_Target1; // spec
			#else
				float4 LS : SV_Target; // light.spec
			#endif
		};

		void 	set_sampleid(uint id);
		void 	set_atoc(uint atoc_new);
		void	set_bilinear_f(bool use);

		float	sample0		 (Texture2D<float>	s_current, float2 tc);
		float	load		 (Texture2D<float> 	s_current, float2 pos2d);
		float	bilinear_load(Texture2D<float>	s_current, float2 pos2d);

		float4	sample0		 (Texture2D 		s_current, float2 tc);
		float4	load		 (Texture2D 		s_current, float2 pos2d);
		float4	bilinear_load(Texture2D 		s_current, float2 pos2d);

		#ifdef MSAA_SAMPLES
			float4  load		 (Texture2DMS<float4, MSAA_SAMPLES> s_current, float2 pos2d);
			float4	bilinear_load(Texture2DMS<float4, MSAA_SAMPLES> s_current, float2 pos2d);

			float   load		 (Texture2DMS<float , MSAA_SAMPLES> s_current, float2 pos2d);
			float	bilinear_load(Texture2DMS<float , MSAA_SAMPLES> s_current, float2 pos2d);
		#endif

		float load_depth_hw(float2 tc);
		float linearize_depth(float depth);
		float delinearize_depth(float depth);

		float4 load_history_packed(float2 tc);

		float pack_sign_8(float val, uint mask);
		void unpack_sign_8(inout float val, out uint mask);

		float2 	tc2p2d(float2 tc);

		int 	pack_float_2x16bit_to_int_32bit(float2 need_pack_f);
		float2 	unpack_int_32bit_to_float_2x16bit(int packed_s);

		int 	pack_float_2x8bit_to_int_16bit(float2 need_pack_f);
		float2 	unpack_int_16bit_to_float_2x8bit(int packed_s);

		uint 	pack_float_2x16bit_to_uint_32bit(float2 need_pack_f);
		float2 	unpack_uint_32bit_to_float_2x16bit(uint packed_s);

		uint 	pack_float_2x8bit_to_uint_16bit(float2 need_pack_f);
		float2 	unpack_uint_16bit_to_float_2x8bit(uint packed_s);

		DEFFER pack_full(float4 norm_hemi, float2 depth_mtl, float4 color_gloss, uint i_mask = 0x0, float2 motion = 0);

		uint load_hud_mask(float2 tc);
		uint load_hud_mask(float2 tc, float2 pos2d);

		GBD prepare();

		GBD load_full(float2 tc, float2 pos2d);
		GBD load_full(float2 tc);

		GBD load_P_N_hemi_mtl_mask(float2 tc, float2 pos2d);
		GBD load_P_N_hemi_mtl_mask(float2 tc);

		GBD load_P_N(float2 tc, float2 pos2d);
		GBD load_P_N(float2 tc);

		GBD load_P_mask(float2 tc, float2 pos2d);
		GBD load_P_mask(float2 tc);

		ACCUMULATOR pack_accum(float4 color, float light, float spec, float shadow = 1, float4 lmap = float4(1, 1, 1, 1));

		void load_accum(in float2 tc, out float3 L, out float3 S);
		void load_accum(in float2 tc, in float2 pos2d, out float3 L, out float3 S);

		float load_depth_wsky(float2 tc);
		float load_depth_wsky(float depth);
		float load_depth(float2 tc);

		float3 load_position(float2 tc);
		float3 load_position(float2 tc, float2 pos2d);

		float4 load_position_wsky(float2 tc);
		float4 load_position_wsky(float2 tc, float2 pos2d);

		float3 load_normal(float2 tc);
		float3 load_normal(float2 tc, float2 pos2d);

		float2 pack_normal(float3 norm);
		float3 unpack_normal(float4 packed_gbuffer);
		float3 unpack_normal(float2 norm);

		float pack_hemi_mtl(float hemi, float mtl);
		float pack_hemi_mtl_mask(float hemi, float mtl, uint mask);
		float unpack_mtl(float hemi_mtl);
		float unpack_hemi(float hemi_mtl);

		float3 unpack_position(float2 uv, float eye_z);
		float3 unpack_position(float2 uv, float2 pos2d, float eye_z);

		float2 vs_tc(float3 vs);
		float2 vs_tcxdepth(float3 vs);
		float3 tcdepth_vs(float3 tcdepth);
		float3 tcxdepthdepth_vs(float3 tcxdepthdepth);
		float3 vs_ws(float3 vs, float offset = 0);
		float3 ws_vs(float3 ws, float offset = 0);
		float2 ws_tc(float3 ws, float offset = 0);
		float4 ws_vp(float3 ws, float offset = 0);
		float4 vs_vp(float3 vs, float offset = 0);

		float load_depth_hw(float2 tc)
		{
			#ifdef MSAA_SAMPLES
				return bilinear_f_use ? bilinear_load(s_depthms, tc2p2d(tc)) : load(s_depthms, tc2p2d(tc));
			#else
				return sample0(s_depth, tc);
			#endif
		}

		float linearize_depth(float depth)
		{
			return 2.0 * depth_params.x * depth_params.y / (depth_params.y + depth_params.x - (depth * 2.0 - 1.0) * (depth_params.y - depth_params.x));
		}

		float delinearize_depth(float depth)
		{
			return 0.5 * ((depth_params.y + depth_params.x) / (depth_params.y - depth_params.x) - (2.0 * depth_params.x / depth) / (depth_params.y - depth_params.x) + 1.0);
		}

		float4 load_history_packed(float2 tc)
		{
			#ifdef MSAA_SAMPLES
				return bilinear_f_use ? bilinear_load(s_position_prev, tc2p2d(tc)) : load(s_position_prev, tc2p2d(tc));
			#else
				return sample0(s_position_prev, tc);
			#endif
		}

		float pack_sign_8(float val, uint mask)
		{
			float sign = mask ? -1 : 1;
			val = (saturate(val) + rcp(255) * 2) * sign;
			return val * 0.5 + 0.5;
		}

		void unpack_sign_8(inout float val, out uint mask)
		{
			if(!val) // it is not object (sky, etc)
			{
				mask = 0;
				return;
			}

			val = val * 2 - 1; // normalize
			mask = val < 0 ? 1 : 0;
			val = saturate(abs(val) - rcp(255) * 2);
		}

		float2 tc2p2d(float2 tc)
		{
			return tc * screen_res.xy;
		}

		void set_sampleid(uint id)
		{
			#ifdef MSAA_SAMPLES
				sampleid = id;
			#endif
		}

		void set_atoc(uint atoc_new)
		{
			#ifdef MSAA_SAMPLES
				atoc = atoc_new;
			#endif
		}

		void set_bilinear_f(bool use)
		{
			bilinear_f_use = use;
		}

		float4 sample0(Texture2D s_current, float2 tc)
		{
			return s_current.SampleLevel((SamplerState)(bilinear_f_use ? smp_rtlinear : smp_nofilter), tc, 0);
		}

		float sample0(Texture2D<float> s_current, float2 tc)
		{
			return s_current.SampleLevel((SamplerState)(bilinear_f_use ? smp_rtlinear : smp_nofilter), tc, 0);
		}

		float4 load(Texture2D s_current, float2 pos2d)
		{
			return s_current.Load(uint3(pos2d, 0));
		}

		float4 bilinear_load(Texture2D s_current, float2 pos2d)
		{
			#ifdef USE_BILINEAR_LOAD
				float4 filter = 0;
				int2 bias = abs(float2(pos2d - (uint2)pos2d)) > 0;
				filter += s_current.Load(uint3(pos2d + int2(0, 0) * bias, 0));
				filter += s_current.Load(uint3(pos2d + int2(0, 1) * bias, 0));
				filter += s_current.Load(uint3(pos2d + int2(1, 0) * bias, 0));
				filter += s_current.Load(uint3(pos2d + int2(1, 1) * bias, 0));
				return filter * 0.25;
			#else
				return s_current.Load(uint3(pos2d, 0));
			#endif
		}

		float load(Texture2D<float> s_current, float2 pos2d)
		{
			return s_current.Load(uint3(pos2d, 0));
		}

		float bilinear_load(Texture2D<float> s_current, float2 pos2d)
		{
			#ifdef USE_BILINEAR_LOAD
				float4 filter = 0;
				int2 bias = abs(float2(pos2d - (uint2)pos2d)) > 0;
				filter += s_current.Load(uint3(pos2d + int2(0, 0) * bias, 0));
				filter += s_current.Load(uint3(pos2d + int2(0, 1) * bias, 0));
				filter += s_current.Load(uint3(pos2d + int2(1, 0) * bias, 0));
				filter += s_current.Load(uint3(pos2d + int2(1, 1) * bias, 0));
				return filter * 0.25;
			#else
				return s_current.Load(uint3(pos2d, 0));
			#endif
		}

		#ifdef MSAA_SAMPLES
			float4 load(Texture2DMS<float4, MSAA_SAMPLES> s_current, float2 pos2d)
			{
			#ifdef SM_4_0
				switch(sampleid)
				{
				case  0: return s_current.Load(pos2d, 0);
				case  1: return s_current.Load(pos2d, 1);
				case  2: return s_current.Load(pos2d, 2);
				case  3: return s_current.Load(pos2d, 3);
				case  4: return s_current.Load(pos2d, 4);
				case  5: return s_current.Load(pos2d, 5);
				case  6: return s_current.Load(pos2d, 6);
				case  7: return s_current.Load(pos2d, 7);
				default: return s_current.Load(pos2d, 0);
				};
			#else
				return s_current.Load(pos2d, sampleid);
			#endif
			}

			float4 bilinear_load(Texture2DMS<float4, MSAA_SAMPLES> s_current, float2 pos2d)
			{
			#ifdef USE_BILINEAR_LOAD
				float4 filter = 0;
				int2 bias = abs(float2(pos2d - (uint2)pos2d)) > 0;
				filter += load(s_current, pos2d + int2(0, 0) * bias);
				filter += load(s_current, pos2d + int2(0, 1) * bias);
				filter += load(s_current, pos2d + int2(1, 0) * bias);
				filter += load(s_current, pos2d + int2(1, 1) * bias);
				return filter * 0.25;
			#else
				return load(s_current, pos2d);
			#endif
			}

			float load(Texture2DMS<float, MSAA_SAMPLES> s_current, float2 pos2d)
			{
			#ifdef SM_4_0
				switch(sampleid)
				{
				case  0: return s_current.Load(pos2d, 0);
				case  1: return s_current.Load(pos2d, 1);
				case  2: return s_current.Load(pos2d, 2);
				case  3: return s_current.Load(pos2d, 3);
				case  4: return s_current.Load(pos2d, 4);
				case  5: return s_current.Load(pos2d, 5);
				case  6: return s_current.Load(pos2d, 6);
				case  7: return s_current.Load(pos2d, 7);
				default: return s_current.Load(pos2d, 0);
				};
			#else
				return s_current.Load(pos2d, sampleid);
			#endif
			}

			float bilinear_load(Texture2DMS<float, MSAA_SAMPLES> s_current, float2 pos2d)
			{
			#ifdef USE_BILINEAR_LOAD
				float filter = 0;
				int2 bias = abs(float2(pos2d - (uint2)pos2d)) > 0;
				filter += load(s_current, pos2d + int2(0, 0) * bias);
				filter += load(s_current, pos2d + int2(0, 1) * bias);
				filter += load(s_current, pos2d + int2(1, 0) * bias);
				filter += load(s_current, pos2d + int2(1, 1) * bias);
				return filter * 0.25;
			#else
				return load(s_current, pos2d);
			#endif
			}
		#endif

		float2 pack_normal(float3 norm)
		{
			float2 res;
			res.x  = norm.z;
			res.y  = 0.5f * (norm.x + 1.0f) ;
			res.y *= (norm.y < 0.0f ? -1.0f : 1.0f);
			return res;
		}

		float pack_hemi_mtl(float hemi, float mtl)
		{
			return pack_float_2x8bit_to_int_16bit(float2(mtl, hemi));
		}

		float pack_hemi_mtl_mask(float hemi, float mtl, uint mask)
		{
			mtl = pack_sign_8(mtl, mask);
			return pack_hemi_mtl(hemi, mtl);
		}

		DEFFER pack_full(float4 norm_hemi, float2 depth_mtl, float4 color_gloss, uint i_mask, float2 motion)
		{
			DEFFER gbd;

			gbd.C = color_gloss; // color . gloss

			gbd.position = float4(pack_normal(norm_hemi.xyz), // normal
								  depth_mtl.x, // depth16
								  pack_hemi_mtl_mask(clamp(norm_hemi.w, 1.0/255.0, 254.0/255.0), depth_mtl.y, i_mask) // hemi & mtl & mask
								 );

			#if defined(MSAA_SAMPLES) && !defined(SM_4_0)
				gbd.atoc_out = atoc;
			#endif

			// #if defined(USE_TAA) || defined(USE_TXAA)
				// gbd.motion = motion;
			// #endif

			return gbd;
		}

		uint load_hud_mask(float2 tc)
		{
			return load_hud_mask(tc, tc2p2d(tc));
		}

		// HUD mask cant be load with bilinear filtering
		uint load_hud_mask(float2 tc, float2 pos2d)
		{
			#ifdef MSAA_SAMPLES
				float packed = bilinear_f_use ? bilinear_load(s_position, pos2d).w : load(s_position, pos2d).w;
			#else
				float packed = sample0(s_position, tc).w;
			#endif

			float2 hemi_mtl = unpack_int_16bit_to_float_2x8bit(packed);
			uint mask;
			unpack_sign_8(hemi_mtl.x, mask);
			return mask;
		}

		float3 unpack_normal(float2 norm)
		{
			float3 res;
			res.z = norm.x;
			res.x = (2.0f * abs(norm.y)) - 1.0f;
			res.y = (norm.y < 0 ? -1.0 : 1.0 ) * sqrt(abs(1 - res.x * res.x - res.z * res.z));
			return res;
		}

		// Reloader
		float3 unpack_normal(float4 packed_gbuffer)
		{
			return unpack_normal(packed_gbuffer.xy);
		}

		float3 unpack_position(float2 uv, float eye_z)
		{
			return float3((uv * float2(2.0, -2.0) - float2(1.0, -1.0)) * InvFocalLen, 1) * eye_z;
		}

		float3 unpack_position(float2 uv, float2 pos2d, float eye_z)
		{
			return float3(pos2d * pos_decompression_params.zw - pos_decompression_params.xy, 1) * eye_z;
		}

		float unpack_hemi(float hemi_mtl)
		{
			return unpack_int_16bit_to_float_2x8bit(hemi_mtl).y;
		}

		float unpack_mtl(float hemi_mtl)
		{
			float mtl_mask = unpack_int_16bit_to_float_2x8bit(hemi_mtl).x;
			uint mask;
			unpack_sign_8(mtl_mask, mask);
			return mtl_mask;
		}

		float3 load_normal(float2 tc)
		{
			#ifdef MSAA_SAMPLES
				float2 nxy = bilinear_f_use ? bilinear_load(s_position, tc2p2d(tc)).xy : load(s_position, tc2p2d(tc)).xy;
			#else
				float2 nxy = sample0(s_position, tc).xy;
			#endif
			return unpack_normal(nxy);
		}

		float3 load_normal(float2 tc, float2 pos2d)
		{
			#ifdef MSAA_SAMPLES
				float2 nxy = bilinear_f_use ? bilinear_load(s_position, pos2d).xy : load(s_position, pos2d).xy;
			#else
				float2 nxy = sample0(s_position, tc).xy;
			#endif
			return unpack_normal(nxy);
		}

		float load_depth(float2 tc)
		{
			#ifdef MSAA_SAMPLES
				return bilinear_f_use ? bilinear_load(s_position, tc2p2d(tc)).z : load(s_position, tc2p2d(tc)).z;
			#else
				return sample0(s_position, tc).z;
			#endif
		}

		float load_depth_wsky(float2 tc)
		{
			float depth = load_depth(tc);
			return (depth >= 0.01) ? (depth) : (sky);
		}

		float load_depth_wsky(float depth)
		{
			return (depth >= 0.01) ? (depth) : (sky);
		}

		ACCUMULATOR pack_accum(float4 color, float light, float spec, float shadow, float4 lmap)
		{
			ACCUMULATOR acc;

			float contrast = 0.75;
			spec /= (contrast + spec);

			#ifdef USE_CSPECULAR
				acc.L = light * color.xyz * shadow * lmap.xyz;
				acc.S = spec * color.xyz * shadow * lmap.w * color.w;
			#else
				acc.LS = float4(light.xxx, spec) * color * shadow * lmap;
			#endif

			return acc;
		}

		void load_accum(in float2 tc, out float3 L, out float3 S)
		{
			load_accum(tc, tc2p2d(tc), L, S);
		}

		void load_accum(in float2 tc, in float2 pos2d, out float3 L, out float3 S)
		{
			float4 accum = load(s_accumulator, pos2d);
			L = accum.xyz;

			#ifdef USE_CSPECULAR
				float4 accum1 = load(s_accumulator_1, pos2d);
				S = accum1.xyz;
			#else
				S = accum.www;
			#endif
		}

		float4 load_position_wsky(float2 tc)
		{
			float depth = load_depth(tc);
			float3 position = unpack_position(tc, (depth >= 0.01) ? (depth) : (sky));
			return float4(position, depth);
		}

		float4 load_position_wsky(float2 tc, float2 pos2d)
		{
			float depth = load_depth(tc);
			float3 position = unpack_position(tc, pos2d, (depth >= 0.01) ? (depth) : (sky));
			return float4(position, depth);
		}

		float3 load_position(float2 tc)
		{
			return unpack_position(tc, load_depth(tc));
		}

		float3 load_position(float2 tc, float2 pos2d)
		{
			return unpack_position(tc, pos2d, load_depth(tc));
		}

		// Faster
		GBD load_P_N_hemi_mtl_mask(float2 tc, float2 pos2d)
		{
			#ifdef MSAA_SAMPLES
				float4 packed = bilinear_f_use ? bilinear_load(s_position, pos2d) : load(s_position, pos2d);
			#else
				float4 packed = sample0(s_position, tc);
			#endif

			GBD gbd = prepare();

			float2 hemi_mtl = unpack_int_16bit_to_float_2x8bit(packed.w);
			unpack_sign_8(hemi_mtl.x, gbd.mask);
			gbd.hemi = hemi_mtl.y;
			gbd.mtl = hemi_mtl.x;
			//gbd.mtl = DEVX;
			gbd.P = unpack_position(tc, pos2d, packed.z);

			gbd.N = unpack_normal(packed.xy);
	
#if DISABLE_LIGHTMAP
			gbd.hemi = LIGHTMAP_CONST_VAL;
#endif
			return gbd;
		} 

		GBD load_P_N_hemi_mtl_mask(float2 tc)
		{
			return load_P_N_hemi_mtl_mask(tc, tc2p2d(tc));
		} 

		// PN
		GBD load_P_N(float2 tc, float2 pos2d)
		{
			#ifdef MSAA_SAMPLES
				float4 packed = bilinear_f_use ? bilinear_load(s_position, pos2d) : load(s_position, pos2d);
			#else
				float4 packed = sample0(s_position, tc);
			#endif

			GBD gbd = prepare();
			gbd.P = unpack_position(tc, pos2d, packed.z);
			gbd.N = unpack_normal(packed.xy);
	
			return gbd;
		} 

		GBD load_P_N(float2 tc)
		{
			return load_P_N(tc, tc2p2d(tc));
		} 

		// P & mask
		GBD load_P_mask(float2 tc, float2 pos2d)
		{
			#ifdef MSAA_SAMPLES
				float4 packed = bilinear_f_use ? bilinear_load(s_position, pos2d) : load(s_position, pos2d);
			#else
				float4 packed = sample0(s_position, tc);
			#endif

			GBD gbd = prepare();
			gbd.P = unpack_position(tc, pos2d, packed.z);
			gbd.N = unpack_normal(packed.xy);

			float2 hemi_mtl = unpack_int_16bit_to_float_2x8bit(packed.w);
			unpack_sign_8(hemi_mtl.x, gbd.mask);

			return gbd;
		} 

		GBD load_P_mask(float2 tc)
		{
			return load_P_mask(tc, tc2p2d(tc));
		} 

		GBD prepare()
		{
			GBD gbd = {float3(0, 0, 0),	0, float3(0, 0, 0), 0, float3(0, 0, 0), 0, 0x0};
			return gbd;
		}

		// Full
		GBD load_full(float2 tc, float2 pos2d)
		{
			#ifdef MSAA_SAMPLES
				GBD gbd = load_P_N_hemi_mtl_mask(tc, pos2d);
				float4 color = bilinear_f_use ? bilinear_load(s_diffuse_ms, pos2d) : load(s_diffuse_ms, pos2d);
			#else
				GBD gbd = load_P_N_hemi_mtl_mask(tc, pos2d);
				float4 color = sample0(s_diffuse, tc);
			#endif

			#ifdef DISABLE_ALBEDO
				color = 0.5;
			#endif

			gbd.C = color.xyz;
			gbd.gloss = color.w;

			return gbd;
		} 

		GBD load_full(float2 tc)
		{
			return load_full(tc, tc2p2d(tc));
		} 

		uint pack_float_2x16bit_to_uint_32bit(float2 need_pack_f)
		{
			uint2 need_pack;
			need_pack = (uint2)(need_pack_f * 0xFFFF);
			need_pack &= 0xFFFF;
			need_pack.y <<= 16;
			need_pack.y &= 0xFFFF0000;
			return need_pack.x | need_pack.y;
		}

		uint pack_float_2x8bit_to_uint_16bit(float2 need_pack_f)
		{
			uint2 need_pack;
			need_pack = (uint2)(need_pack_f * 0xFF);
			need_pack &= 0xFF;
			need_pack.y <<= 8;
			need_pack.y &= 0xFF00;
			return need_pack.x | need_pack.y;
		}

		float2 unpack_uint_32bit_to_float_2x16bit(uint packed)
		{
			uint2 unpacked;
			unpacked.x = packed & 0xFFFF;
			unpacked.y = packed & 0xFFFF0000;
			unpacked.y >>= 16;
			float2 unpacked_f = unpacked / (float)0xFFFF;
			return unpacked_f;
		}

		float2 unpack_uint_16bit_to_float_2x8bit(uint packed)
		{
			uint2 unpacked;
			unpacked.x = packed & 0xFF;
			unpacked.y = packed & 0xFF00;
			unpacked.y >>= 8;
			float2 unpacked_f = unpacked / (float)0xFF;
			return unpacked_f;
		}

		int pack_float_2x16bit_to_int_32bit(float2 need_pack_f)
		{
			uint2 need_pack;
			need_pack = (uint2)(need_pack_f * 0xFFFF);
			need_pack &= 0xFFFF;
			bool sign = (need_pack.y & 0x1) == 1;
			need_pack.y <<= 15;
			need_pack.y &= 0x7FFF0000; // Can't use first bit
			int need_pack_s = need_pack.x | need_pack.y;
			return sign ? -need_pack_s : need_pack_s;
		}

		int pack_float_2x8bit_to_int_16bit(float2 need_pack_f)
		{
			uint2 need_pack;
			need_pack = (uint2)(need_pack_f * 0xFF);
			need_pack &= 0xFF;
			bool sign = (need_pack.y & 0x1) == 1;
			need_pack.y <<= 7;
			need_pack.y &= 0x7F00; // Can't use first bit
			int need_pack_s = need_pack.x | need_pack.y;
			return sign ? -need_pack_s : need_pack_s;
		}

		float2 unpack_int_32bit_to_float_2x16bit(int packed_s)
		{
			int packed_s_absolute = abs(packed_s);
			uint2 unpacked;
			unpacked.x = packed_s_absolute & 0xFFFF;
			unpacked.y = packed_s_absolute & 0x7FFF0000;
			unpacked.y >>= 15;
			unpacked.y &= 0xFFFE;
			unpacked.y += packed_s < 0;
			float2 unpacked_f = unpacked / (float)0xFFFF;
			return unpacked_f;
		}

		float2 unpack_int_16bit_to_float_2x8bit(int packed_s)
		{
			int packed_s_absolute = abs(packed_s);
			uint2 unpacked;
			unpacked.x = packed_s_absolute & 0xFF;
			unpacked.y = packed_s_absolute & 0x7F00;
			unpacked.y >>= 7;
			unpacked.y &= 0xFE;
			unpacked.y += packed_s < 0;
			float2 unpacked_f = unpacked / (float)0xFF;
			return unpacked_f;
		}

		float2 vs_tc(float3 vs)
		{
			return (vs.xy / vs.zz + pos_decompression_params.xy) / pos_decompression_params.zw / screen_res.xy;
		}

		float2 vs_tcxdepth(float3 vs)
		{
			return (vs.xy + pos_decompression_params.xy * vs.zz) / pos_decompression_params.zw / screen_res.xy;
		}

		float3 tcdepth_vs(float3 tcdepth)
		{
			return float3(tcdepth.xy * screen_res.xy * pos_decompression_params.zw - pos_decompression_params.xy, 1) * tcdepth.z;
		}

		float3 tcxdepthdepth_vs(float3 tcxdepthdepth)
		{
			return float3(tcxdepthdepth.xy * screen_res.xy * pos_decompression_params.zw - pos_decompression_params.xy * tcxdepthdepth.z, tcxdepthdepth.z);
		}

		float3 vs_ws(float3 vs, float offset)
		{
			return mul((float3x4)m_v2w, float4(vs, offset));
		}

		float3 ws_vs(float3 ws, float offset)
		{
			return mul((float3x4)m_V, float4(ws, offset));
		}

		float2 ws_tc(float3 ws, float offset)
		{
			float4 project = mul((float4x4)m_VP, float4(ws, offset));

			float2 tc = project.xy / project.ww;
			tc = (tc + float2(1, -1)) * float2(0.5, -0.5); // [-1..1] в [0..1]

			return tc;
		}

		float4 vs_vp(float3 vs, float offset)
		{
			return mul(m_P, float4(vs, offset));
		}

		float4 ws_vp(float3 ws, float offset)
		{
			return mul(m_VP, float4(ws, offset));
		}
	}
#endif
