//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

cbuffer View : register(b0)
{
    float4x4 ViewToWorldMatrix;
    float4x4 WorldToViewMatrix;
    float4x4 ProjMatrix;
    float4x4 WorldToClip;

    float DeltaTime;
    float WorldTime;
    float Offset;
    float Pad;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

PSInput VSMain(float3 Position : POSITION, float2 UV : TEXCOORD)
{
    PSInput Result;
    //Result.Position = mul( float4(Position.xyz, 1), WorldToClip );
    Result.Position = mul( WorldToClip, float4(Position.xyz, 1) ); 
    Result.Color = float4(UV, 0, 1);
    return Result;
}

float4 PSMain(PSInput Input) : SV_TARGET
{
    return float4(Input.Position.z * 100, 0, 0, 1);
}

