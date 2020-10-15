#include "../Headers/MVP.hlsli"

struct VS_In
{
	float3 Position : POSITION;
	float4 Color	: COLOR;
};

struct VS_Out
{
    float4 Position : SV_POSITION;
    float4 Color : COLOR;
};

VS_Out main( VS_In Input )
{
    VS_Out Output;
    
    Output.Position = mul(Model, float4(Input.Position, 1));
    Output.Position = mul(Output.Position, View);
    Output.Position = mul(Output.Position, Projection);

    Output.Color = Input.Color;
    
    return Output;
}