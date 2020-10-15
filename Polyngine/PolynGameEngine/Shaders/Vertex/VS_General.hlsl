struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 Texture : TEXCOORD0;
};

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

VS_OUTPUT main(VS_INPUT Input)
{
	// Setup Position and Normals.
    VS_OUTPUT Output = (VS_OUTPUT) 0;
    Output.Position = float4(Input.Position.xyz, 1);
    Output.Texture = Input.Texture;
    Output.Normal = mul(WorldMatrix, float4(Input.Normal.xyz, 0)).xyz;
    Output.Normal = normalize(Output.Normal);

	// Get the matrices to match up with object position.
    Output.Position = mul(WorldMatrix, Output.Position);
    Output.Wposition = Output.Position;
    Output.Position = mul(ViewMatrix, Output.Position);
    Output.Position = mul(ProjectionMatrix, Output.Position);
	
	// Return the information for the Pixel Shader.
    return Output;
}