#pragma once

#include <DirectXMath.h>
#include "d3d11.h"
#include "../../PStatics/PGameplayStatics/PGameplayStatics.h"
#include "../../PSystem/PController/PController.h"
#include "../../PSystem/PAnimation/PAnim/PAnim.h"

using namespace PMath;

// A PObject is a base item in the game world. If something needs transform, three dimensions, or any sort of movement or interaction, chances are that it either is an object, or inherits from an object. Objects should not be used in place of StaticMeshes and SkeletalMeshes.
class PObject
{
public:
	// Debug information only for development. Not to be relayed to the user.
	std::string DisplayName;

	// Object space handling.
	view_t DefaultWorld;					// World and Projection matrices.
	DirectX::XMMATRIX LocalMatrix;			// Local Offset matrix.
	bool bShowInHierarchy = true;

	// If this object is attached to another, it will be set here.
	PObject* ParentObject = nullptr;

	// PController is the Controller that this object gets input from.
	PController* Controller = nullptr;

	// Animation component to play animations through.
	PAnim Animator;

	// Control information for direct manipulation.
	bool Ctrl_bIsVisible = true;			// Should this object render?
	bool Ctrl_bIsHiddenInGame = false;		// Should this object render in-game?
	bool Ctrl_bEnableInput = false;			// Should this object receive input?
	bool Ctrl_bStartWithController = false;
	float Ctrl_bEnableMovement = false;		// Should this object be allowed to move?
	float Ctrl_bEnableRotation = false;		// Should this object be allowed to rotate?
	float Ctrl_MovementSpeed = 11.0f;		// The speed of movement.
	float Ctrl_RotationRate = 4.5f;			// The rate of rotation.

	// Constructor.
	PObject();
	PObject(std::string DebugName, bool bVisible = true);

	// Begin, Update, and EndPlay for game state notification.
	//
	// BeginPlay is called once the constructor has finished execution, and often is for custom classes or implimentation that should execute when the object has been loaded into the world. This is the safest place to assume setup has completed.
	virtual void BeginPlay();

	// Update is called every time the frame changes.
	void virtual Update(float DeltaTime);

	// EndPlay is called once the object has been destroyed and memory has been cleaned up. This is the safest place to assume that cleanup has been completed.
	virtual void EndPlay();

	// Destroy is called just before EndPlay, and is responsible for cleaning up dynamic memory.
	virtual void Destroy();

	// Get object information.
	//
	std::string GetDisplayName();		// Return the DebugName for this object.
	void RefreshLocalLocation();		// Update the matrices based on attached parent.
	bool GetVisibility();				// Return whether (1) the object is visible or (0) hidden.
	bool GetHiddenInGame();				// Return whether (1) the object is hidden in-game, or (0) visible when in-game.
	float3 GetScale();					// Return the current scale of this object. 
	float3 GetLocalScale();				// Return the current scale in relation to the parent object.
	float3 GetLocation();				// Return the current location of the object.
	float3 GetLocalLocation();			// Return the current local location of the object.
	float3 GetRotation();				// Return the current rotation of this object.
	view_t& GetWorld();					// Return the current World information (view_t).
	DirectX::XMMATRIX& GetLocal();		// Return the current Local information (XMMATRIX).
	float3 GetUpVector();				// Return the current up vector.
	float3 GetRightVector();			// Return the current right vector.
	float3 GetForwardVector();			// Return the current forward vector.
	bool GetInputEnabled();				// Get whether input is accepted or not.

	// Set member information of this object.
	//
	void ScaleObject(float3 Scale);						// Scale the object up or down.
	void SetScale(float3 Scale);						// Set the object's scale.
	void ScaleObjectLocally(float3 Scale);				// Scale the local matrix for the object.
	DirectX::XMMATRIX SetLocation(float3 Location);		// Set the location of this object.
	DirectX::XMMATRIX SetLocalLocation(float3 Location);		// Set the location of this object.
	DirectX::XMMATRIX SetRotation(float3 Rotation);		// Set the rotation of this object.
	void SetVisibility(bool bVisible = true);			// Set the visibility of this object.
	void SetHiddenInGame(bool bVisible = true);			// Set the visibility of this in-game.
	void SetInputEnabled(bool bEnable = true);			// Set whether input is accepted or not.
	void SetMovementEnabled(bool bEnable = true);		// Set whether movement is accepted or not.

	// Add input from a controller.
	//
	void AddMovementInput(float3 Direction);		// Move in a direction.
	void AddLocalMovementInput(float3 Direction);	// Move locally about a parent object.
	void AddRotationInput(float3 Direction);		// Rotate in one or more directions.
	void AddLocalRotationInput(float3 Direction);	// Rotate in one or more directions locally.

	// Manage parent child attachment.
	//
	void AttachToObject(PObject* Obj);					// Attach this object to another object.
	void PossessController(PController* InController, bool bActivate = false);	// Assign a controller to this object. Activate to true if you want the object to have input be enabled as well.
};

