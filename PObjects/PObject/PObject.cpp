#include "PObject.h"

template<typename T>
void safe_release(T* t)
{
	if (t)
		t->Release();
}

// This is a container for a 3D model and its world interaction components. It requires a 3D model and can only hold one.
PObject::PObject()
{

}

PObject::PObject(std::string DebugName, bool bVisible)
{
	DisplayName = DebugName;			// Choose a display name for debug printing.
	Ctrl_bIsVisible = bVisible;			// Set the initial visibility.
	Ctrl_bEnableInput = false;			// Disable input.

	// World and Local Matrices.
	DefaultWorld.ViewMatrix = (float4x4_a&)DirectX::XMMatrixIdentity();
	LocalMatrix = DirectX::XMMatrixIdentity();
	ScaleObject(float3{ 1.0f, 1.0f, 1.0f });
	ScaleObjectLocally(float3{ 1.0f, 1.0f, 1.0f });

	// Call BeginPlay() to signal that setup is complete.
	BeginPlay();
}

// This is called as soon as this object is spawned in the
// game and begins playing.
void PObject::BeginPlay()
{

}

// This is called every frame (when not in-engine).
void PObject::Update(float DeltaTime)
{
	// Update the animation pipeline if need be.
	Animator.Update(DeltaTime);

	// Child's world matrix, in hierarchy, is equal to (child world = child local * parent word).
	RefreshLocalLocation();
}

// Destroy this object and call EndPlay().
void PObject::Destroy()
{
	SetInputEnabled(false);
	Controller = nullptr;
	ParentObject = nullptr;
	Ctrl_bEnableMovement = false;
	Ctrl_bEnableRotation = false;
}

// This is called just before this object is completely
// destroyed. The last thing this function does is delete
// the object.
void PObject::EndPlay()
{
	
}

// Return the human-readable display name of this object.
std::string PObject::GetDisplayName()
{
	return DisplayName;
}

void PObject::RefreshLocalLocation()
{
	// Child's world matrix, in hierarchy, is equal to (child world = child local * parent word).
	if (ParentObject != nullptr)
	{
		DefaultWorld.ViewMatrix = (float4x4_a&)DirectX::XMMatrixMultiply(LocalMatrix, (DirectX::XMMATRIX&)ParentObject->DefaultWorld.ViewMatrix);
	}
}

// Set to either show or hide this object.
void PObject::SetVisibility(bool bVisible)
{
	Ctrl_bIsVisible = bVisible;
}

// Set to either show or hide this object in-game.
void PObject::SetHiddenInGame(bool bVisible)
{
	Ctrl_bIsHiddenInGame = bVisible;
}

// Returns true if the object is currently visible, elsewise it returns 0.
bool PObject::GetVisibility()
{
	return Ctrl_bIsVisible;
}

// Returns true if the object is currently hidden in-game, elsewise it returns 0.
bool PObject::GetHiddenInGame()
{
	return Ctrl_bIsHiddenInGame;
}

// Returns the view_t for this object, containing the World Matrix
// and the Projection Matrix.
view_t& PObject::GetWorld()
{
	return DefaultWorld;
}

// Returns the Local Matrix for this object.
DirectX::XMMATRIX& PObject::GetLocal()
{
	return LocalMatrix;
}

// Return this object's world location.
float3 PObject::GetLocation()
{
	DirectX::XMMATRIX WorldMatrix = (DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix);
	DirectX::XMVECTOR LocationVector = WorldMatrix.r[3];

	return float3({ DirectX::XMVectorGetX(LocationVector), DirectX::XMVectorGetY(LocationVector), DirectX::XMVectorGetZ(LocationVector) });
}

float3 PObject::GetLocalLocation()
{
	DirectX::XMVECTOR LocationVector = LocalMatrix.r[3];

	return float3({ DirectX::XMVectorGetX(LocationVector), DirectX::XMVectorGetY(LocationVector), DirectX::XMVectorGetZ(LocationVector) });
}

// Pitch, yaw, roll.
float3 PObject::GetRotation()
{
	XMVECTOR Loc;
	XMVECTOR Rot;
	XMVECTOR Sca;

	XMMatrixDecompose(&Sca, &Rot, &Loc, (XMMATRIX&)DefaultWorld.ViewMatrix);

	return { Rot.m128_f32[0], Rot.m128_f32[1], Rot.m128_f32[2] };
}

// Return this object's current Up Vector.
float3 PObject::GetUpVector()
{
	DirectX::XMMATRIX WorldMatrix = (DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix);
	float3 UpVector = { DirectX::XMVectorGetX(WorldMatrix.r[1]), DirectX::XMVectorGetY(WorldMatrix.r[1]), DirectX::XMVectorGetZ(WorldMatrix.r[1]) };

	return UpVector;
}

// Return this object's current Right Vector.
float3 PObject::GetRightVector()
{
	DirectX::XMMATRIX WorldMatrix = (DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix);
	float3 RightVector = { DirectX::XMVectorGetX(WorldMatrix.r[0]), DirectX::XMVectorGetY(WorldMatrix.r[0]), DirectX::XMVectorGetZ(WorldMatrix.r[0]) };

	return RightVector;
}

// Return this object's current Forward Vector.
float3 PObject::GetForwardVector()
{
	DirectX::XMMATRIX WorldMatrix = (DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix);
	float3 ForwardVector = { DirectX::XMVectorGetX(WorldMatrix.r[2]), DirectX::XMVectorGetY(WorldMatrix.r[2]), DirectX::XMVectorGetZ(WorldMatrix.r[2]) };

	return ForwardVector;
}

// Add local movement input in the direction the actor is facing.
void PObject::AddMovementInput(float3 Direction)
{
	DirectX::XMMATRIX TranslationMatrix = DirectX::XMMatrixTranslation(Direction.x, Direction.y, Direction.z);
	DefaultWorld.ViewMatrix = (float4x4_a&)DirectX::XMMatrixMultiply(TranslationMatrix, (DirectX::XMMATRIX&)DefaultWorld.ViewMatrix);
}

// Add local movement to child objects.
void PObject::AddLocalMovementInput(float3 Direction)
{
	DirectX::XMMATRIX TranslationMatrix = DirectX::XMMatrixTranslation(Direction.x, Direction.y, Direction.z);
	LocalMatrix = DirectX::XMMatrixMultiply(LocalMatrix, TranslationMatrix);
	RefreshLocalLocation();
}

// Add local rotation offset in the control direction of the object.
void PObject::AddRotationInput(float3 Direction)
{
	DirectX::XMMATRIX& WorldMatrix = (DirectX::XMMATRIX&)DefaultWorld.ViewMatrix;

	float3 StartLocation = GetLocation();

	DirectX::XMMATRIX RotationMatrix = (DirectX::XMMatrixMultiply(WorldMatrix, DirectX::XMMatrixRotationY(Direction.y)));
	RotationMatrix.r[3].m128_f32[0] = StartLocation.x;
	RotationMatrix.r[3].m128_f32[1] = StartLocation.y;
	RotationMatrix.r[3].m128_f32[2] = StartLocation.z;

	RotationMatrix = (DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(Direction.x), RotationMatrix));
	RotationMatrix.r[3].m128_f32[0] = StartLocation.x;
	RotationMatrix.r[3].m128_f32[1] = StartLocation.y;
	RotationMatrix.r[3].m128_f32[2] = StartLocation.z;

	RotationMatrix = (DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationZ(Direction.z), RotationMatrix));
	RotationMatrix.r[3].m128_f32[0] = StartLocation.x;
	RotationMatrix.r[3].m128_f32[1] = StartLocation.y;
	RotationMatrix.r[3].m128_f32[2] = StartLocation.z;

	RefreshLocalLocation();

	DefaultWorld.ViewMatrix = (float4x4_a&)RotationMatrix;
}

// Add local rotation to child objects.
void PObject::AddLocalRotationInput(float3 Direction)
{
	DirectX::XMMATRIX& LocalWorld = LocalMatrix;

	float3 StartLocation = GetLocalLocation();

	DirectX::XMMATRIX RotationMatrix = (DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationY(Direction.y), LocalWorld));
	RotationMatrix.r[3].m128_f32[0] = StartLocation.x;
	RotationMatrix.r[3].m128_f32[1] = StartLocation.y;
	RotationMatrix.r[3].m128_f32[2] = StartLocation.z;

	RotationMatrix = (DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationX(Direction.x), RotationMatrix));
	RotationMatrix.r[3].m128_f32[0] = StartLocation.x;
	RotationMatrix.r[3].m128_f32[1] = StartLocation.y;
	RotationMatrix.r[3].m128_f32[2] = StartLocation.z;

	RotationMatrix = (DirectX::XMMatrixMultiply(DirectX::XMMatrixRotationZ(Direction.z), RotationMatrix));
	RotationMatrix.r[3].m128_f32[0] = StartLocation.x;
	RotationMatrix.r[3].m128_f32[1] = StartLocation.y;
	RotationMatrix.r[3].m128_f32[2] = StartLocation.z;

	RefreshLocalLocation();

	LocalMatrix = (XMMATRIX&)RotationMatrix;
}

// Set this object's location to another location.
DirectX::XMMATRIX PObject::SetLocation(float3 Location)
{
	((DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix)).r[3].m128_f32[0] = Location.x;
	((DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix)).r[3].m128_f32[1] = Location.y;
	((DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix)).r[3].m128_f32[2] = Location.z;

	return (DirectX::XMMATRIX&)DefaultWorld.ViewMatrix;
}

DirectX::XMMATRIX PObject::SetLocalLocation(float3 Location)
{
	((DirectX::XMMATRIX&)(LocalMatrix)).r[3].m128_f32[0] = Location.x;
	((DirectX::XMMATRIX&)(LocalMatrix)).r[3].m128_f32[1] = Location.y;
	((DirectX::XMMATRIX&)(LocalMatrix)).r[3].m128_f32[2] = Location.z;

	RefreshLocalLocation();

	return (DirectX::XMMATRIX&)LocalMatrix;
}

// Pitch, yaw, roll.
DirectX::XMMATRIX PObject::SetRotation(float3 Rotation)
{
	/*((DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix)).r[0].m128_f32[0] = Rotation.x;
	((DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix)).r[1].m128_f32[1] = Rotation.y;
	((DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix)).r[2].m128_f32[2] = Rotation.z;*/

	return (DirectX::XMMATRIX&)DefaultWorld.ViewMatrix;
}

void PObject::ScaleObject(float3 Scale)
{
	DefaultWorld.ViewMatrix = (float4x4_a&)DirectX::XMMatrixMultiply((DirectX::XMMATRIX&)DefaultWorld.ViewMatrix, DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z));
}

void PObject::SetScale(float3 Scale)
{
	((DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix)).r[0].m128_f32[0] = Scale.x;
	((DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix)).r[1].m128_f32[1] = Scale.y;
	((DirectX::XMMATRIX&)(DefaultWorld.ViewMatrix)).r[2].m128_f32[2] = Scale.z;
}

void PObject::ScaleObjectLocally(float3 Scale)
{
	LocalMatrix = DirectX::XMMatrixMultiply(LocalMatrix, DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z));
	RefreshLocalLocation();
}

float3 PObject::GetScale()
{
	return float3{ DefaultWorld.ViewMatrix[0].x, DefaultWorld.ViewMatrix[1].y, DefaultWorld.ViewMatrix[2].z };
}

float3 PObject::GetLocalScale()
{
	return float3{ LocalMatrix.r[0].m128_f32[0], LocalMatrix.r[1].m128_f32[1], LocalMatrix.r[3].m128_f32[3] };
}

void PObject::AttachToObject(PObject* Obj)
{
	ParentObject = Obj;
	SetInputEnabled(false);
	SetMovementEnabled(false);
	RefreshLocalLocation();
}

void PObject::PossessController(PController* InController, bool bActivate)
{
	if (InController)
	{
		Controller = InController;

		if (bActivate)
		{
			SetInputEnabled(true);
		}
	}
}

void PObject::SetInputEnabled(bool bEnable)
{
	Ctrl_bEnableInput = bEnable;
}

void PObject::SetMovementEnabled(bool bEnable)
{
	Ctrl_bEnableMovement = bEnable;
	Ctrl_bEnableRotation = bEnable;
}

bool PObject::GetInputEnabled()
{
	return (Ctrl_bEnableInput && Ctrl_bEnableMovement);
}
