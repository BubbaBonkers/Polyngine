#pragma once

#include "../../PLights/PLight.h"
#include "../../../PMath/PMath.h"

class PDirectionalLight :	public PLight
{
public:
	PDirectionalLight();
	PDirectionalLight(std::string DebugName, float Strength, float3 Dir, float4 Clr = { 1, 1, 1, 1 });

	void BeginPlay();
	void Update(float DeltaTime);
	void EndPlay();
	void Destroy();
};

