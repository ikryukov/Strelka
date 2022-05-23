/*
Copyright (c) 2022, NVIDIA CORPORATION. All rights reserved.

NVIDIA CORPORATION and its licensors retain all intellectual property
and proprietary rights in and to this software, related documentation
and any modifications thereto. Any use, reproduction, disclosure or
distribution of this software and related documentation without an express
license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "STL.hlsli"
#include "../Include/NRD.hlsli"

#include "../Resources/Clear_ui.resources.hlsli"

#include "../Include/Common.hlsli"

[numthreads( 16, 16, 1 )]
NRD_EXPORT void NRD_CS_MAIN( uint2 pixelPos : SV_DispatchThreadId )
{
    gTexture[ pixelPos ] = 0;
}
