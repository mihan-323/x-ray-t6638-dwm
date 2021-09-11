//---------------------------------------------------------------------------
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
// This file is automatically generated.  Please do not edit it directly.
//
// File name: D2D1Effects_1.h
//---------------------------------------------------------------------------
#ifdef _MSC_VER
#pragma once
#endif // #ifdef _MSC_VER

#ifndef _D2D1_EFFECTS_1_
#define _D2D1_EFFECTS_1_

#ifndef _D2D1_EFFECTS_
#include <d2d1effects.h>
#endif // #ifndef _D2D1_EFFECTS_

           
#include "winapifamily.h"

#pragma region Application Family
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)           
           
// Built in effect CLSIDs
DEFINE_GUID(CLSID_D2D1YCbCr,                    0x99503cc1, 0x66c7, 0x45c9, 0xa8, 0x75, 0x8a, 0xd8, 0xa7, 0x91, 0x44, 0x01);

/// <summary>
/// The enumeration of the YCbCr effect's top level properties.
/// </summary>
typedef enum D2D1_YCBCR_PROP
{
    
    /// <summary>
    /// Property Name: "ChromaSubsampling"
    /// Property Type: D2D1_YCBCR_CHROMA_SUBSAMPLING
    /// </summary>
    D2D1_YCBCR_PROP_CHROMA_SUBSAMPLING = 0,
    
    /// <summary>
    /// Property Name: "TransformMatrix"
    /// Property Type: D2D1_MATRIX_3X2_F
    /// </summary>
    D2D1_YCBCR_PROP_TRANSFORM_MATRIX = 1,
    
    /// <summary>
    /// Property Name: "InterpolationMode"
    /// Property Type: D2D1_YCBCR_INTERPOLATION_MODE
    /// </summary>
    D2D1_YCBCR_PROP_INTERPOLATION_MODE = 2,
    D2D1_YCBCR_PROP_FORCE_DWORD = 0xffffffff

} D2D1_YCBCR_PROP;

typedef enum D2D1_YCBCR_CHROMA_SUBSAMPLING
{
    D2D1_YCBCR_CHROMA_SUBSAMPLING_AUTO = 0,
    D2D1_YCBCR_CHROMA_SUBSAMPLING_420 = 1,
    D2D1_YCBCR_CHROMA_SUBSAMPLING_422 = 2,
    D2D1_YCBCR_CHROMA_SUBSAMPLING_444 = 3,
    D2D1_YCBCR_CHROMA_SUBSAMPLING_440 = 4,
    D2D1_YCBCR_CHROMA_SUBSAMPLING_FORCE_DWORD = 0xffffffff

} D2D1_YCBCR_CHROMA_SUBSAMPLING;

typedef enum D2D1_YCBCR_INTERPOLATION_MODE
{
    D2D1_YCBCR_INTERPOLATION_MODE_NEAREST_NEIGHBOR = 0,
    D2D1_YCBCR_INTERPOLATION_MODE_LINEAR = 1,
    D2D1_YCBCR_INTERPOLATION_MODE_CUBIC = 2,
    D2D1_YCBCR_INTERPOLATION_MODE_MULTI_SAMPLE_LINEAR = 3,
    D2D1_YCBCR_INTERPOLATION_MODE_ANISOTROPIC = 4,
    D2D1_YCBCR_INTERPOLATION_MODE_HIGH_QUALITY_CUBIC = 5,
    D2D1_YCBCR_INTERPOLATION_MODE_FORCE_DWORD = 0xffffffff

} D2D1_YCBCR_INTERPOLATION_MODE;


#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP) */
#pragma endregion

#endif // #ifndef _D2D1_EFFECTS_1_
