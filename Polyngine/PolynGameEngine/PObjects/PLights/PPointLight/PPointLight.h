#pragma once

#include "../../PLights/PLight.h"
#include "../PLight.h"

class PPointLight :	public PLight
{
public:
	float Radius;

	PPointLight();
	PPointLight(std::string DebugName, float Strength, float Rad, float4 Clr = { 1.0f, 1.0f, 1.0f, 1.0f });

	void BeginPlay();
	void Update(float DeltaTime);
	void EndPlay();
	void Destroy();

	float GetRadius();
	void SetRadius(float Rad);
};

