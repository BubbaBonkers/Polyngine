#include "PDirectionalLight.h"

PDirectionalLight::PDirectionalLight()
{

}

PDirectionalLight::PDirectionalLight(std::string DebugName, float Strength, float3 Dir, float4 Clr)
{
	DisplayName = DebugName;
	Intensity = Strength;
	Color = Clr;
	Ctrl_bIsVisible = true;				// Set the initial visibility.
	Ctrl_bEnableInput = false;			// Disable input.

	// World and Local Matrices.
	DefaultWorld.ViewMatrix = (float4x4_a&)DirectX::XMMatrixIdentity();
	LocalMatrix = DirectX::XMMatrixIdentity();
	ScaleObject(float3{ 1.0f, 1.0f, 1.0f });
	ScaleObjectLocally(float3{ 1.0f, 1.0f, 1.0f });
	AddRotationInput(Dir);

	// Call BeginPlay() to signal that setup is complete.
	BeginPlay();
}

void PDirectionalLight::BeginPlay()
{
	PObject::BeginPlay();
}

void PDirectionalLight::Update(float DeltaTime)
{
	PObject::Update(DeltaTime);
}

void PDirectionalLight::Destroy()
{
	PObject::Destroy();

	EndPlay();
}

void PDirectionalLight::EndPlay()
{
	PObject::EndPlay();
}
