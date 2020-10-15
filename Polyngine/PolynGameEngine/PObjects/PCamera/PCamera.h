#pragma once

#include "../PObject/PObject.h"
#include "../../PMath/PMath.h"

class PCamera : public PObject
{
private:
	float Cam_FOV = 90.0f;
	float Cam_RotationSpeed = 0.003f;

	bool Cam_bIsRendering = false;
	bool Cam_bActiveOnStart = false;

public:
	// Default Constructor requires manual InputManager, Matrix, and BeginPlay() setup and calls.
	PCamera();
	PCamera(std::string DebugName, float FOV = 90.0f, bool bAssignInput = false);

	void BeginPlay();
	void Update(float DeltaTime);
	void Destroy();
	void EndPlay();

	void RefreshAspectRatio(float Aspect, float NearPlane = 0.03f, float FarPlane = 1000.0f);
	void SetFieldOfView(float FieldOfView);
	void SetInputEnabled(bool bRead = true, bool bMovement = true, bool bRotation = true);
	bool GetIsActive();
	bool& GetIsActiveOnStart();
	float GetFieldOfView();
	float& GetFieldOfViewRef();
	void SetActive(bool bActive);
};

