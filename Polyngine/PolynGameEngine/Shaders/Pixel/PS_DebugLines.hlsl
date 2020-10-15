#include "../Headers/MVP.hlsli"

struct VS_Out
{
	float4 Position : SV_POSITION;
	float4 Color	: COLOR;
};

float4 main(VS_Out Input) : SV_TARGET
{
	return Input.Color;
}