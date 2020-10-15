#include "PCamera.h"

PCamera::PCamera()
{
	BeginPlay();
}

PCamera::PCamera(std::string DebugName, float FOV, bool bAssignInput)
{
	DisplayName = DebugName;

	DefaultWorld.ViewMatrix = (float4x4_a&)DirectX::XMMatrixIdentity();
	LocalMatrix = DirectX::XMMatrixIdentity();
	SetFieldOfView(FOV);

	XMVECTOR EyePosition = XMVectorSet(0.0f, 15.0f, -15.0f, 1.0f);
	XMVECTOR Focus = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DefaultWorld.ViewMatrix = (PMath::float4x4_a&)XMMatrixInverse(nullptr, XMMatrixLookAtLH(EyePosition, Focus, Up));

	SetInputEnabled(bAssignInput, bAssignInput, bAssignInput);

	BeginPlay();
}

// This is called as soon as this object is spawned in the
// game and begins playing.
void PCamera::BeginPlay()
{
	PObject::BeginPlay();
}

// This is called every frame.
void PCamera::Update(float DeltaTime)
{
	// Handle input, but only if a controller is attached.
	if (Controller)
	{
		if (Controller->IsInputDown(PInputManager::PInputMap::PIN_MOUSE_R))
		{
			if (Ctrl_bEnableRotation)
			{
				PMath::float2 P_MouseDelta = Controller->GetMouseDelta();

				if (P_MouseDelta.x != 0 || P_MouseDelta.y != 0)
				{
					AddRotationInput(PMath::float3{ (P_MouseDelta.y * Cam_RotationSpeed), (P_MouseDelta.x * Cam_RotationSpeed), 0.0f });
				}
			}

			if (Ctrl_bEnableMovement)
			{
				if (Controller->IsInputDown(PInputManager::PInputMap::PIN_W))
				{
					AddMovementInput(float3{ 0, 0, (Ctrl_MovementSpeed * DeltaTime) });
				}
				else if (Controller->IsInputDown(PInputManager::PInputMap::PIN_S))
				{
					AddMovementInput(float3{ 0, 0, -(Ctrl_MovementSpeed * DeltaTime) });
				}

				if (Controller->IsInputDown(PInputManager::PInputMap::PIN_A))
				{
					AddMovementInput(float3{ -(Ctrl_MovementSpeed * DeltaTime), 0, 0 });
				}
				else if (Controller->IsInputDown(PInputManager::PInputMap::PIN_D))
				{
					AddMovementInput(float3{ (Ctrl_MovementSpeed * DeltaTime), 0, 0 });
				}

				if (Controller->IsInputDown(PInputManager::PInputMap::PIN_MOUSE_WHEEL_UP))
				{
					if (Ctrl_MovementSpeed < 100.0f)
					{
						Ctrl_MovementSpeed += DeltaTime * 15;
					}
				}
				else if (Controller->IsInputDown(PInputManager::PInputMap::PIN_MOUSE_WHEEL_DOWN))
				{
					if (Ctrl_MovementSpeed > 2.0f)
					{
						Ctrl_MovementSpeed -= DeltaTime * 15;
					}
				}

				if (Controller->IsInputDown(PInputManager::PInputMap::PIN_Q))
				{
					AddMovementInput(float3{ 0.0f, DeltaTime, 0.0f });
				}
				else if (Controller->IsInputDown(PInputManager::PInputMap::PIN_E))
				{
					AddMovementInput(float3{ 0.0f, -DeltaTime, 0.0f });
				}
			}
		}
		else if (Ctrl_bEnableMovement)
		{
			if (Controller->IsInputDown(PInputManager::PInputMap::PIN_MOUSE_WHEEL_UP))
			{
				AddMovementInput(float3{ 0.0f, 0.0f, ((Ctrl_MovementSpeed * 0.25f) * DeltaTime) });
			}
			else if (Controller->IsInputDown(PInputManager::PInputMap::PIN_MOUSE_WHEEL_DOWN))
			{
				AddMovementInput(float3{ 0.0f, 0.0f, -((Ctrl_MovementSpeed * 0.25f) * DeltaTime) });
			}
		}
	}

	PObject::Update(DeltaTime);
}

void PCamera::Destroy()
{
	PObject::Destroy();

	EndPlay();
}

// This is called just before this object is completely
// destroyed.
void PCamera::EndPlay()
{
	PObject::EndPlay();
}

void PCamera::RefreshAspectRatio(float Aspect, float NearPlane, float FarPlane)
{
	DefaultWorld.ProjectionMatrix = (PMath::float4x4_a&)DirectX::XMMatrixPerspectiveFovLH(PDegrees_Radians(Cam_FOV), Aspect, NearPlane, FarPlane);
}

void PCamera::SetFieldOfView(float FieldOfView)
{
	if (Controller != nullptr)
	{
		RECT rect;
		GetClientRect(Controller->GetHWND(), &rect);

		Cam_FOV = FieldOfView;
		RefreshAspectRatio(((float)rect.right / (float)rect.bottom));
	}
}

void PCamera::SetInputEnabled(bool bRead, bool bMovement, bool bRotation)
{
	Ctrl_bEnableInput = bRead;
	Ctrl_bEnableMovement = bMovement;
	Ctrl_bEnableRotation = bRotation;

	if (bRead || bMovement || bRotation)
	{
		SetFieldOfView(Cam_FOV);
	}
}

void PCamera::SetActive(bool bActive)
{
	Cam_bIsRendering = bActive;

	if (!bActive)
	{
		SetInputEnabled(false, false, false);
	}

	SetFieldOfView(Cam_FOV);
}

bool PCamera::GetIsActive()
{
	return Cam_bIsRendering;
}

bool& PCamera::GetIsActiveOnStart()
{
	return Cam_bActiveOnStart;
}

float PCamera::GetFieldOfView()
{
	return Cam_FOV;
}

float& PCamera::GetFieldOfViewRef()
{
	return Cam_FOV;
}
