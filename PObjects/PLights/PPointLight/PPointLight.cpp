#include "PPointLight.h"

PPointLight::PPointLight()
{

}

// Strength is out of 1.
PPointLight::PPointLight(std::string DebugName, float Strength, float Rad, float4 Clr)
{
	DisplayName = DebugName;
	Intensity = std::clamp(Strength, 0.0f, 1.0f);
	Radius = Rad;
	Color = Clr;

	Ctrl_bIsVisible = true;				// Set the initial visibility.
	Ctrl_bEnableInput = false;			// Disable input.

	// World and Local Matrices.
	DefaultWorld.ViewMatrix = (float4x4_a&)DirectX::XMMatrixIdentity();
	LocalMatrix = DirectX::XMMatrixIdentity();
	ScaleObject(float3{ 1.0f, 1.0f, 1.0f });
	ScaleObjectLocally(float3{ 1.0f, 1.0f, 1.0f });

	// Call BeginPlay() to signal that setup is complete.
	BeginPlay();
}

void PPointLight::BeginPlay()
{
	PObject::BeginPlay();
}

void PPointLight::Update(float DeltaTime)
{
	PObject::Update(DeltaTime);
}

void PPointLight::Destroy()
{
	PObject::Destroy();

	EndPlay();
}

void PPointLight::EndPlay()
{
	PObject::EndPlay();
}

float PPointLight::GetRadius()
{
	return Radius;
}

void PPointLight::SetRadius(float Rad)
{
	Radius = Rad;
}
