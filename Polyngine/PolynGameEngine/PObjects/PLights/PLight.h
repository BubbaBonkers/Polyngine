#pragma once

#include "../../PObjects/PObject/PObject.h"
#include "../../PMath/PMath.h"

// This document holds universal traits to all lights that do not reside in all objects. It is included in all lights, but has no functionality on its own. If you want to add a light, add a type of light, such as point, directional, or spot.
class PLight : public PObject
{
public:
	float Intensity;
	PMath::float4 Color;

	void BeginPlay() 
	{ 
		Ctrl_bIsVisible = true;
		Ctrl_bEnableInput = false;
		DefaultWorld.ViewMatrix = (float4x4_a&)DirectX::XMMatrixIdentity();
		LocalMatrix = DirectX::XMMatrixIdentity();

		PObject::BeginPlay();
	};
	void Update(float DeltaTime) { PObject::Update(DeltaTime); };
	void EndPlay() { PObject::EndPlay(); };
	void Destroy() { PObject::Destroy(); };

	void ToggleVisibility() { Ctrl_bIsVisible = !Ctrl_bIsVisible; };
	float GetIntensity() { return Intensity; };
	void SetIntensity(float Int) { Intensity = Int; };
	float4 GetColor() { return Color; };
	void SetColor(float4 Clr) { Color = Clr; };
};
