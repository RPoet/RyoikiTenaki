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

PSInput VSMain(float4 Position : POSITION, float4 Color : COLOR)
{
    PSInput Result;

    Result.Position = Position;
    Result.Position.x += Offset;

    Result.Color = Color;

    return Result;
}

float4 PSMain(PSInput Input) : SV_TARGET
{
    return Input.Color;
}

