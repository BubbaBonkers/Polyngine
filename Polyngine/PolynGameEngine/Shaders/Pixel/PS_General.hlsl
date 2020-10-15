Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 Texture : TEXCOORD1;
    float3 Wposition : WORLDPOSITION;
};

cbuffer ConstantBuffer : register(b0) // b for Buffer, and 0 for slot 0 in GPU.
{
    float4x4 WorldMatrix;
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
    float4x4 CameraWorldMatrix;
};

float4 main(VS_OUTPUT InputPixel) : SV_TARGET
{
    // Get pixel color for texture.
    float4 FinalColor = txDiffuse.Sample(samLinear, InputPixel.Texture);
    float OriginalAlpha = FinalColor.a;
    
    // Check for alpha elimination.
    if (FinalColor.a < 0.05f)
    {
        discard;
    }
    
    // Get the ambient lighting.
    float4 Ambient = ((float4(1, 1, 1, 1)) * 1);
	
	// Add the ambient lighting to the object.
    FinalColor = ((Ambient) * FinalColor);
    FinalColor.a = OriginalAlpha;
    
	// Return the fully lighted object.
    return FinalColor;
}