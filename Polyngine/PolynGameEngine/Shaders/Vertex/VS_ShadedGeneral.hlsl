#include "../Headers/MVP.hlsli"

struct VS_In
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 Texture : TEXCOORD0;
};

struct VS_Out
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 Texture : TEXCOORD1;
    float3 WorldPos : WORLDPOSITION;
};

VS_Out main( VS_In Input )
{    
	// Setup Position and Normals.
    VS_Out Output = (VS_Out) 0;
    Output.Position = float4(Input.Position.xyz, 1);
    Output.Texture = Input.Texture;
    Output.Normal = mul(Model, float4(Input.Normal.xyz, 0)).xyz;
    Output.Normal = normalize(Output.Normal);

	// Get the matrices to match up with object position.
    Output.Position = mul(Model, Output.Position);
    Output.WorldPos = Output.Position.xyz;
    Output.Position = mul(View, Output.Position);
    Output.Position = mul(Projection, Output.Position);
	
	return Output;
}