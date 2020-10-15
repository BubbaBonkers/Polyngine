#include "PEnvironment.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "../PDebugLines/PDebugLineRender.h"
#include "../GUIToolbox/ImGui/imgui.h"
#include "../GUIToolbox/ImGui/imgui_impl_win32.h"
#include "../GUIToolbox/ImGui/imgui_impl_dx11.h"

// DO NOT edit this document directly. Always make a copy in a location within the "Assets" folder
// and create a custom derived class of the Environment. When you make a copy, be sure to make calls
// to the derived functions for any that exist in the parent.
PEnvironment::PEnvironment()
{
	CurrentState = ERenderStates::DEBUG;
	PCamera* RendCam = CreateCamera(72.0f, true, true, "RenderCamera");
	RendCam->bShowInHierarchy = false;
}

// This is called once as soon as the Environment has been constructed and initialized in the world.
void PEnvironment::BeginPlay()
{
	// Set the environment in the Gameplay Statics.
	PGameplayStatics::SetEnvironment(this);

	// Create the generic engine delay timer and set it to 3 seconds. Used for engine actions as to prevent overloading the engine with commands (ex. Enter/Exit Fullscreen, create a new level, load a level, etc...).
	CreateStopwatch("EngineDelay", 3.0f, true);

	// Create the framerate update delay timer and set it to 0.1 seconds. This is how often any framerate display will be updated in order to make seeing the number easier.
	CreateStopwatch("FPSTimer", 0.1f, true);

	// Print that the Environment was created.
	PrintToConsole("Environment was created and initialized.", 1);
	
	// Get the Render Camera to test against.
	PCamera* RendCam = GetRenderCamera();

	// Posses the render camera with a controller.
	if (RendCam && CurrentState == ERenderStates::DEBUG)
	{
		RendCam->PossessController(CreateController());
	}

	PrintToConsole("Ready for use.");


	// ------------------------------------------------------------------------------------------
	// Below is the code for RUNTIME MATERIALS created objects.
	// ------------------------------------------------------------------------------------------
	CurrentLevelName = "Runtime Materials Lab";
	AmbientLightIntensity = 0.0f;
	bFlag_ShowDebugGrid = false;
	bFlag_ShowCollisionBoxes = false;
	SelectedObject = nullptr;

	CreateStopwatch("RedLightFlash", 2.2f, true);

	PSkeletalMesh* NewMesh = CreateSkeletalMesh("Meshes/BattleMage.mesh", "Textures/BattleMage/BattleMage_D.dds", "Textures/BattleMage/BattleMage_S.dds", "Textures/BattleMage/BattleMage_E.dds", true, "BattleMageMesh", nullptr, { 0.85f, 0.85f, 0.85f });
	NewMesh->AddMovementInput({ 4.5f, 0.0f, 0.0f });
	NewMesh->SetBoundingBoxOffset({ 0.0f, 3.0f, 0.0f });
	NewMesh->SetBoundingBoxExtents({ 1.5f, 3.0f, 1.5f });
	NewMesh->PlayAnimation("Animations/BattleMage/Testing/Idle.anim");

	PStaticMesh* Floor = CreatePrimitive(EPrimitives::PLANE, true, "WorldFloor", nullptr, { 15.0f, 1.0f, 15.0f });
	Floor->LoadTexture("Textures/OceanTile/OceanTile_D.dds", Device, 0);
	Floor->LoadTexture("Textures/OceanTile/OceanTile_S.dds", Device, 2);
	Floor->SetBoundingBoxOffset({ 0.0f, 0.0f, 0.0f });
	Floor->SetBoundingBoxExtents({ 15.0f, 0.1f, 15.0f });

	PPointLight* NewLight = CreatePointLight(0.78f, 10.0f, { 1.0f, 1.0f, 1.0f, 1.0f }, "WhiteLight");
	NewLight->AddMovementInput({ 2.81f, 4.0f, 2.5f });

	NewLight = CreatePointLight(1.0f, 50.0f, { 0.95f, 0.08f, 0.05f, 1.0f }, "RedLight");
	NewLight->AddMovementInput({ 6.8f, 6.0f, -1.0f });

	NewLight = CreatePointLight(1.0f, 50.0f, { 0.05f, 0.1f, 0.9f, 1.0f }, "BlueLight");
	NewLight->AddMovementInput({ 0.75f, 6.0f, -1.0f });
}

// Called once per frame.
void PEnvironment::Update(float DeltaTime)
{
	// ------------------------------------------------------------------------------------------
	// Below is the code for RUNTIME MATERIALS created objects.
	// ------------------------------------------------------------------------------------------	
	PStopwatch* Watch = GetStopwatchByName("RedLightFlash");
	if (Watch && Watch->Test())
	{
		PPointLight* RedLight = dynamic_cast<PPointLight*>(GetObjectByName("RedLight"));

		if (RedLight)
		{
			RedLight->ToggleVisibility();
		}
	}

	// Stopwatch updates.
	//
	// This will run in any Render State, as Stopwatches control gameplay and engine features.
	for (unsigned int i = 0; i < WorldStopwatches.size(); ++i)
	{
		PStopwatch* CurrWatch = WorldStopwatches[i].Watch;

		if (CurrWatch)
		{
			CurrWatch->Update(DeltaTime);
		}

		if (WorldStopwatches[i].Name == "FPSTimer")
		{
			if (CurrWatch->Test())
			{
				FPSRef = 1.0f / DeltaTime;
			}
		}
	}

	// Controller updates.
	//
	// This will run in any render state, as controllers also control render camera and features.
	for (unsigned int i = 0; i < WorldControllers.size(); ++i)
	{
		PController* CurrController = WorldControllers[i];

		if (CurrController)
		{
			CurrController->Update(DeltaTime);
			CurrController->RenderState = CurrentState;
		}
	}

	if (CurrentState == ERenderStates::DEBUG)
	{
		GetRenderCamera()->Update(DeltaTime);
	}

	// Object updates.
	//
	// Objects include: PObject, PCamera, PStaticMesh, PLight, PPointLight, and PDirectionalLight.
	// This will run only in multiple modes and has a flow control to decipher which.
	// It should only run if one of the conditions within the loop is decided to be true ahead of time
	// to save on cycles if the loop would run and do nothing.
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		PObject* CurrObject = WorldObjects[i];

		if (CurrObject != nullptr)
		{
			// Be sure the camera used for rendering the editor viewport is updated in Debug mode.
			if (CurrentState == ERenderStates::DEBUG)
			{
				CurrObject->RefreshLocalLocation();
			}

			// Update the object if it should be updated and is NOT in debug mode.
			if ((CurrentState != ERenderStates::DEBUG) && CurrObject->GetDisplayName() != "RenderCamera")
			{
				CurrObject->Update(DeltaTime);
			}
			else if (CurrentState == ERenderStates::DEBUG)
			{
				CurrObject->Animator.Update(DeltaTime);
			}

			PStaticMesh* CurrMesh = dynamic_cast<PStaticMesh*>(CurrObject);
			if (CurrMesh != nullptr)
			{
				CurrMesh->Col_BoundingBox.Center = CurrMesh->GetLocation() + CurrMesh->Col_BBOffset;
			}

			// This will only run if the editor setting is set to show the Matrices and the state is not in Ship mode.
			if (CurrentState != ERenderStates::SHIP)
			{
				PSkeletalMesh* Skeleton = dynamic_cast<PSkeletalMesh*>(CurrObject);
				if (Skeleton && Skeleton->Animator.GetReady())
				{
					DebugLines::AddSkeleton(Skeleton);
					DebugLines::AddBindPoseSkeleton(Skeleton);
				}

				if (bFlag_ShowDebugMatrices)
				{
					// Draw debug matrices and lines for this object based on its type.
					PCamera* TestCam = dynamic_cast<PCamera*>(CurrObject);
					if (dynamic_cast<PDirectionalLight*>(CurrObject))
					{
						DebugLines::AddObject(CurrObject, DebugLines::DrawType::DIRECTIONAL);
					}
					else if (dynamic_cast<PPointLight*>(CurrObject))
					{
						DebugLines::AddObject(CurrObject, DebugLines::DrawType::POINT);
					}
					else if (!(TestCam && TestCam->GetInputEnabled()))
					{
						DebugLines::AddObject(CurrObject);
					}
					else if (TestCam && !TestCam->GetInputEnabled())
					{
						DebugLines::AddObject(CurrObject, DebugLines::DrawType::CAMERA);
					}
				}

				if (bFlag_ShowCollisionBoxes)
				{
					if (CurrMesh)
					{
						DebugLines::AddBoxCollider(CurrMesh->Col_BoundingBox);
					}
				}
			}
		}
	}

	// Check for quick keys in renderer.
	//
	// When the renderer is in debug mode.
	if ((CurrentState == ERenderStates::DEBUG) && (InputManager != nullptr))
	{
		// Delete button to delete the currently selected object in the Environment.
		if (InputManager->IsInputDown(PInputManager::PInputMap::PIN_DELETE) && (SelectedObject != nullptr))
		{
			DestroyObject(SelectedObject);
		}

		// Quick save level hot-key to save the current level if it already exists.
		if (CurrentState == ERenderStates::DEBUG && (InputManager->IsInputDown(PInputManager::PInputMap::PIN_CONTROL) && InputManager->IsInputDown(PInputManager::PInputMap::PIN_S)) && CurrentLevel != "")
		{
			PStopwatch* EngineDelayWatch = GetStopwatchByName("EngineDelay");

			if (EngineDelayWatch)
			{
				if (EngineDelayWatch->Test())
				{
					SaveLevel(CurrentLevel);
				}
			}
		}
	}

	// Check to exit the game testing mode.
	if (CurrentState == ERenderStates::TEST && (InputManager != nullptr) && (InputManager->IsInputDown(PInputManager::PInputMap::PIN_ESCAPE) && InputManager->IsInputDown(PInputManager::PInputMap::PIN_SHIFT)))
	{
		EndPlaytest();
	}
}

// This is called once, and is the last thing to happen in the Environment before it gets destroyed.
void PEnvironment::EndPlay()
{
	ClearEnvironment();
}

// This will call EndPlay() and clean up any memory necessary before the Environment is destroyed.
void PEnvironment::Destroy()
{
	EndPlay();
}

// Print some text to the output log. OutString is the string that will be printed, Status will decide the
// color and type of the string to print, and the Source is the class creating the log.
//
// Status:
//	0. Text
//	1. Success
//	2. Failure
//	3. Warning
//	4. System Command
void PEnvironment::PrintToConsole(std::string OutString, int Status, std::string Source)
{
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOut, 15);

	if (CurrentState != ERenderStates::SHIP)
	{
		std::cout << Source << " >> ";

		switch (Status)
		{
			case 0:
			{
				SetConsoleTextAttribute(hOut, 15);
				break;
			}
			case 1:
			{
				SetConsoleTextAttribute(hOut, 2);
				std::cout << "SUCCESS:: ";
				break;
			}
			case 2:
			{
				SetConsoleTextAttribute(hOut, 4);
				std::cout << "FAILURE:: ";
				break;
			}
			case 3:
			{
				SetConsoleTextAttribute(hOut, 6);
				std::cout << "WARNING:: ";
				break;
			}
			case 4:
			{
				SetConsoleTextAttribute(hOut, 11);
				std::cout << "SYS:: ";
				break;
			}
		}

		std::cout << OutString << "\n";
		
		POutput TempOut = { OutString, Status, Source };
		OutputLogData.insert(OutputLogData.begin(), TempOut);
	}
}

// Update all cameras in the world given a new Aspect Ratio.
void PEnvironment::RefreshCameraAspectRatios(float Aspect)
{
	std::vector<PCamera*> WorldCameras = GetCameras();
	for (unsigned int i = 0; i < WorldCameras.size(); ++i)
	{
		WorldCameras[i]->RefreshAspectRatio(Aspect);
	}
}

// Create a stopwatch for use in the game or engine world.
PStopwatch* PEnvironment::CreateStopwatch(std::string Name, float Time, bool bShouldRecycle)
{
	PStopwatch* NewWatch = new PStopwatch(Time, bShouldRecycle);
	StopwatchCont NewCont = { Name, NewWatch };

	WorldStopwatches.push_back(NewCont);

	return NewWatch;
}

// Create a controller to gather input from the Input Manager for an object.
PController* PEnvironment::CreateController()
{
	PController* NewController = nullptr;

	if (InputManager)
	{
		NewController = new PController(InputManager);
		WorldControllers.push_back(NewController);
	}
	else
	{
		PrintToConsole("Could not create the controller because no Input Manager exists.", 4);
	}

	if (!NewController || !InputManager)
	{
		PrintToConsole("Could not create new Controller.", 2);
	}

	return NewController;
}

// Create a basic object with 3D space orientation.
PObject* PEnvironment::CreateObject(bool bVisible, std::string DebugName)
{
	std::string FinalName = DebugName;
	if (DebugName == "" || GetObjectByName(DebugName))
	{
		FinalName = "Object_" + std::to_string(WorldObjects.size()) + "_" + std::to_string(rand() % 6666);
	}

	PObject* NewObject = new PObject(FinalName, bVisible);
	WorldObjects.push_back(NewObject);

	if (!NewObject)
	{
		PrintToConsole("Could not create Object: " + DebugName, 2);
	}

	SelectedObject = NewObject;

	return NewObject;
}

// Create a camera with the ability to see the game world, just as eyes.
PCamera* PEnvironment::CreateCamera(float FOV, bool bAssignInput, bool bSetActive, std::string DebugName)
{
	std::string FinalName = DebugName;
	if (DebugName == "" || GetObjectByName(DebugName))
	{
		FinalName = "Camera_" + std::to_string(WorldObjects.size()) + "_" + std::to_string(rand() % 6666);
	}

	PCamera* NewCamera = new PCamera(FinalName, FOV, bAssignInput);
	WorldObjects.push_back(NewCamera);

	if (bSetActive)
	{
		SetActiveCamera(NewCamera);
	}

	if (!NewCamera)
	{
		PrintToConsole("Could not create Camera: " + DebugName, 2);
	}

	if (NewCamera->GetDisplayName() != "RenderCamera")
	{
		SelectedObject = NewCamera;
	}

	return NewCamera;
}

// Create a primitive type shape such as a Plane, Cube, Sphere, or otherwise. This will be created as a Static Mesh.
PStaticMesh* PEnvironment::CreatePrimitive(EPrimitives Type, bool bVisible, std::string DebugName, PObject* Parent, float3 InScale)
{
	std::vector<Vertex> Verts;
	std::vector<int> Ind;

	if (Type == EPrimitives::PLANE)
	{
		// Tri 1
		Verts.push_back({ { -1.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } });
		Ind.push_back(0);

		Verts.push_back({ { -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
		Ind.push_back(1);

		Verts.push_back({ { 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } });
		Ind.push_back(2);

		// Tri 2
		Verts.push_back({ { 1.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } });
		Ind.push_back(3);
		
		Ind.push_back(0);
		Ind.push_back(2);
	}
	else if (Type == EPrimitives::CUBE)
	{
		// Bottom Face
		Verts.push_back({ { -1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } });
		Ind.push_back(0);

		Verts.push_back({ { -1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } });
		Ind.push_back(1);

		Verts.push_back({ { 1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } });
		Ind.push_back(2);

		Verts.push_back({ { 1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } });
		Ind.push_back(3);

		Ind.push_back(0);
		Ind.push_back(2);

		// Top Face
		Verts.push_back({ { -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } });
		Ind.push_back(4);

		Verts.push_back({ { -1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
		Ind.push_back(5);

		Verts.push_back({ { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } });
		Ind.push_back(6);

		Verts.push_back({ { 1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } });
		Ind.push_back(7);

		Ind.push_back(4);
		Ind.push_back(6);

		// Front Face
		Verts.push_back({ { -1.0f, -1.0f, 1.0f }, { 0.0, 0.0f, 1.0f }, { 0.0f, 1.0f } });
		Ind.push_back(8);

		Verts.push_back({ { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } });
		Ind.push_back(9);

		Verts.push_back({ { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } });
		Ind.push_back(10);

		Verts.push_back({ { 1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } });
		Ind.push_back(11);

		Ind.push_back(8);
		Ind.push_back(10);

		// Back Face
		Verts.push_back({ { -1.0f, -1.0f, -1.0f }, { 0.0, 0.0f, -1.0f }, { 0.0f, 1.0f } });
		Ind.push_back(12);

		Verts.push_back({ { -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } });
		Ind.push_back(13);

		Verts.push_back({ { 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } });
		Ind.push_back(14);

		Verts.push_back({ { 1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } });
		Ind.push_back(15);

		Ind.push_back(12);
		Ind.push_back(14);

		// Right Face
		Verts.push_back({ { 1.0f, -1.0f, -1.0f }, { 1.0, 0.0f, 0.0f }, { 0.0f, 1.0f } });
		Ind.push_back(16);

		Verts.push_back({ { 1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } });
		Ind.push_back(17);

		Verts.push_back({ { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } });
		Ind.push_back(18);

		Verts.push_back({ { 1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } });
		Ind.push_back(19);

		Ind.push_back(16);
		Ind.push_back(18);

		// Left Face
		Verts.push_back({ { -1.0f, -1.0f, -1.0f }, { -1.0, 0.0f, 0.0f }, { 0.0f, 1.0f } });
		Ind.push_back(20);

		Verts.push_back({ { -1.0f, 1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } });
		Ind.push_back(21);

		Verts.push_back({ { -1.0f, 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } });
		Ind.push_back(22);

		Verts.push_back({ { -1.0f, -1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } });
		Ind.push_back(23);

		Ind.push_back(20);
		Ind.push_back(22);
	}
	else if (Type == EPrimitives::SPHERE)
	{
		// Tri 1
		
		
	}
	else if (Type == EPrimitives::CONE)
	{
		// Tri 1

	}

	std::string FinalName = DebugName;
	if (DebugName == "" || GetObjectByName(DebugName))
	{
		FinalName = "StaticMesh_" + std::to_string(GetStaticMeshes().size()) + "_" + std::to_string(rand() % 6666);
	}

	PStaticMesh* NewStaticMesh = new PStaticMesh(FinalName, Verts, Ind, "Textures/Default/DefaultWorld.dds", Device, bVisible, Parent, InScale);
	WorldObjects.push_back(NewStaticMesh);

	if (!NewStaticMesh)
	{
		PrintToConsole("Could not create Primitive: " + DebugName, 2);
	}

	NewStaticMesh->PrimitiveType = Type;
	NewStaticMesh->ModelFile = "Primitive_" + PPrimNames[Type];

	SelectedObject = NewStaticMesh;

	return NewStaticMesh;
}

// Create a static mesh using some Model and Texture data. Optionally initialize this object with a set visibility, name, parent, and scale.
PStaticMesh* PEnvironment::CreateStaticMesh(const char* ModelFilePath, const char* DDSFilePath, bool bVisible, std::string DebugName, PObject* Parent, float3 InScale)
{
	std::string FinalName = DebugName;
	if (DebugName == "" || GetObjectByName(DebugName))
	{
		FinalName = "StaticMesh_" + std::to_string(GetStaticMeshes().size()) + "_" + std::to_string(rand() % 6666);
	}

	PStaticMesh* NewStaticMesh = new PStaticMesh(FinalName, ModelFilePath, DDSFilePath, Device, DeviceContext, bVisible, Parent, InScale);
	WorldObjects.push_back(NewStaticMesh);

	if (!NewStaticMesh)
	{
		PrintToConsole("Could not create Static Mesh: " + DebugName, 2);
	}

	SelectedObject = NewStaticMesh;

	NewStaticMesh->PrimitiveType = EPrimitives::NONE;

	return NewStaticMesh;
}

PSkeletalMesh* PEnvironment::CreateSkeletalMesh(const char* MeshFilePath, const char* DDSFilePath, const char* Spec_DDSFilePath, const char* Emissive_DDSFilePath, bool bVisible, std::string DebugName, PObject* Parent, float3 InScale)
{
	std::string FinalName = DebugName;
	if (DebugName == "" || GetObjectByName(DebugName))
	{
		FinalName = "SkeletalMesh_" + std::to_string(GetSkeletalMeshes().size()) + "_" + std::to_string(rand() % 6666);
	}

	PSkeletalMesh* NewSkeletalMesh = new PSkeletalMesh(FinalName, MeshFilePath, DDSFilePath, Device, DeviceContext, bVisible, Parent, InScale);
	WorldObjects.push_back(NewSkeletalMesh);

	if (NewSkeletalMesh)
	{
		if (Spec_DDSFilePath != "none")
		{
			NewSkeletalMesh->LoadTexture(Spec_DDSFilePath, Device, 2);
		}
		else
		{
			NewSkeletalMesh->Specular_DDSFile = "";
		}

		if (Emissive_DDSFilePath != "none")
		{
			NewSkeletalMesh->LoadTexture(Emissive_DDSFilePath, Device, 3);
		}
		else
		{
			NewSkeletalMesh->Emissive_DDSFile = "";
		}

		SelectedObject = NewSkeletalMesh;

		NewSkeletalMesh->PrimitiveType = EPrimitives::NONE;
	}
	else
	{
		PrintToConsole("Could not create Skeletal Mesh: " + DebugName, 2);
	}

	return NewSkeletalMesh;
}

PCharacter* PEnvironment::CreateCharacter(const char* ModelFilePath, const char* DDSFilePath, bool bVisible, std::string DebugName, PObject* Parent, float3 InScale)
{
	std::string FinalName = DebugName;
	if (DebugName == "" || GetObjectByName(DebugName))
	{
		FinalName = "Character_" + std::to_string(GetStaticMeshes().size()) + "_" + std::to_string(rand() % 6666);
	}

	PCharacter* NewCharacter = new PCharacter(FinalName, ModelFilePath, DDSFilePath, Device, DeviceContext, bVisible, Parent, InScale);
	WorldObjects.push_back(NewCharacter);

	if (!NewCharacter)
	{
		PrintToConsole("Could not create Character: " + DebugName, 2);
	}

	SelectedObject = NewCharacter;

	NewCharacter->PrimitiveType = EPrimitives::NONE;

	return NewCharacter;
}

PCharacter* PEnvironment::CreateCharacter(EPrimitives Type, bool bVisible, std::string DebugName, PObject* Parent, float3 InScale)
{
	std::vector<Vertex> Verts;
	std::vector<int> Ind;

	if (Type == EPrimitives::PLANE)
	{
		// Tri 1
		Verts.push_back({ { -1.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } });
		Ind.push_back(0);

		Verts.push_back({ { -1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
		Ind.push_back(1);

		Verts.push_back({ { 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } });
		Ind.push_back(2);

		// Tri 2
		Verts.push_back({ { 1.0f, 0.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } });
		Ind.push_back(3);

		Ind.push_back(0);
		Ind.push_back(2);
	}
	else if (Type == EPrimitives::CUBE)
	{
		// Bottom Face
		Verts.push_back({ { -1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } });
		Ind.push_back(0);

		Verts.push_back({ { -1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f } });
		Ind.push_back(1);

		Verts.push_back({ { 1.0f, -1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 0.0f } });
		Ind.push_back(2);

		Verts.push_back({ { 1.0f, -1.0f, -1.0f }, { 0.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } });
		Ind.push_back(3);

		Ind.push_back(0);
		Ind.push_back(2);

		// Top Face
		Verts.push_back({ { -1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f } });
		Ind.push_back(4);

		Verts.push_back({ { -1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } });
		Ind.push_back(5);

		Verts.push_back({ { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } });
		Ind.push_back(6);

		Verts.push_back({ { 1.0f, 1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 1.0f } });
		Ind.push_back(7);

		Ind.push_back(4);
		Ind.push_back(6);

		// Front Face
		Verts.push_back({ { -1.0f, -1.0f, 1.0f }, { 0.0, 0.0f, 1.0f }, { 0.0f, 1.0f } });
		Ind.push_back(8);

		Verts.push_back({ { -1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f } });
		Ind.push_back(9);

		Verts.push_back({ { 1.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f } });
		Ind.push_back(10);

		Verts.push_back({ { 1.0f, -1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } });
		Ind.push_back(11);

		Ind.push_back(8);
		Ind.push_back(10);

		// Back Face
		Verts.push_back({ { -1.0f, -1.0f, -1.0f }, { 0.0, 0.0f, -1.0f }, { 0.0f, 1.0f } });
		Ind.push_back(12);

		Verts.push_back({ { -1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 0.0f, 0.0f } });
		Ind.push_back(13);

		Verts.push_back({ { 1.0f, 1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 0.0f } });
		Ind.push_back(14);

		Verts.push_back({ { 1.0f, -1.0f, -1.0f }, { 0.0f, 0.0f, -1.0f }, { 1.0f, 1.0f } });
		Ind.push_back(15);

		Ind.push_back(12);
		Ind.push_back(14);

		// Right Face
		Verts.push_back({ { 1.0f, -1.0f, -1.0f }, { 1.0, 0.0f, 0.0f }, { 0.0f, 1.0f } });
		Ind.push_back(16);

		Verts.push_back({ { 1.0f, 1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } });
		Ind.push_back(17);

		Verts.push_back({ { 1.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } });
		Ind.push_back(18);

		Verts.push_back({ { 1.0f, -1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } });
		Ind.push_back(19);

		Ind.push_back(16);
		Ind.push_back(18);

		// Left Face
		Verts.push_back({ { -1.0f, -1.0f, -1.0f }, { -1.0, 0.0f, 0.0f }, { 0.0f, 1.0f } });
		Ind.push_back(20);

		Verts.push_back({ { -1.0f, 1.0f, -1.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } });
		Ind.push_back(21);

		Verts.push_back({ { -1.0f, 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f } });
		Ind.push_back(22);

		Verts.push_back({ { -1.0f, -1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } });
		Ind.push_back(23);

		Ind.push_back(20);
		Ind.push_back(22);
	}
	else if (Type == EPrimitives::SPHERE)
	{
		// Tri 1

	}
	else if (Type == EPrimitives::CONE)
	{
		// Tri 1

	}

	std::string FinalName = DebugName;
	if (DebugName == "" || GetObjectByName(DebugName))
	{
		FinalName = "Character_" + std::to_string(GetStaticMeshes().size()) + "_" + std::to_string(rand() % 6666);
	}

	PCharacter* NewCharacter = new PCharacter(FinalName, Verts, Ind, "Textures/Default/DefaultWorld.dds", Device, bVisible, Parent, InScale);
	WorldObjects.push_back(NewCharacter);

	if (!NewCharacter)
	{
		PrintToConsole("Could not create Primitive Character: " + DebugName, 2);
	}

	NewCharacter->PrimitiveType = Type;
	NewCharacter->ModelFile = "Primitive_" + PPrimNames[Type];

	SelectedObject = NewCharacter;

	return NewCharacter;
}

// Create a directional light to act as a sunlight.
PDirectionalLight* PEnvironment::CreateDirectionalLight(float Strength, float3 Dir, float4 Clr, std::string DebugName)
{
	std::string FinalName = DebugName;
	if (DebugName == "" || GetObjectByName(DebugName))
	{
		FinalName = "DirectionalLight_" + std::to_string(GetDirectionalLights().size()) + "_" + std::to_string(rand() % 6666);
	}

	PDirectionalLight* NewDirLight = new PDirectionalLight(FinalName, Strength, Dir, Clr);
	WorldObjects.push_back(NewDirLight);

	if (!NewDirLight)
	{
		PrintToConsole("Could not create Directional Light: " + DebugName, 2);
	}

	SelectedObject = NewDirLight;

	return NewDirLight;
}

// Create a point light to light a small area within a radius in the game world.
PPointLight* PEnvironment::CreatePointLight(float Strength, float Rad, float4 Clr, std::string DebugName)
{
	std::string FinalName = DebugName;
	if (DebugName == "" || GetObjectByName(DebugName))
	{
		FinalName = "PointLight_" + std::to_string(GetPointLights().size()) + "_" + std::to_string(rand() % 6666);
	}

	PPointLight* NewPointLight = new PPointLight(FinalName, Strength, Rad, Clr);
	WorldObjects.push_back(NewPointLight);

	if (!NewPointLight)
	{
		PrintToConsole("Could not create Point Light: " + DebugName, 2);
	}

	SelectedObject = NewPointLight;

	return NewPointLight;
}

// Destroy any object in the game world using a pointer to that object.
bool PEnvironment::DestroyObject(PObject* Obj)
{
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		if (WorldObjects[i] && (!GetActiveCamera() || (WorldObjects[i]->GetDisplayName() != GetActiveCamera()->GetDisplayName())) && (WorldObjects[i]->GetDisplayName() == Obj->GetDisplayName()))
		{
			if (SelectedObject->GetDisplayName() == Obj->GetDisplayName())
			{
				SelectedObject = nullptr;
			}

			WorldObjects[i]->Destroy();
			delete WorldObjects[i];
			WorldObjects.erase(WorldObjects.begin() + i);
			return true;
		}
	}

	return false;
}

// Get a stopwatch by the name you gave it when you created it.
PStopwatch* PEnvironment::GetStopwatchByName(std::string WatchName)
{
	for (unsigned int i = 0; i < WorldStopwatches.size(); ++i)
	{
		if (WorldStopwatches[i].Name == WatchName)
		{
			return WorldStopwatches[i].Watch;
		}
	}

	return nullptr;
}

// Get an object by the name you gave it when you created it. You cannot access the engine renderer objects via this method.
PObject* PEnvironment::GetObjectByName(std::string ObjectName)
{
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		if (WorldObjects[i] && (WorldObjects[i]->GetDisplayName() == ObjectName))
		{
			return WorldObjects[i];
		}
	}

	return nullptr;
}

// Get the currently active camera for the game viewport. Only one camera should be able to be active
// at any given time.
PCamera* PEnvironment::GetActiveCamera()
{
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		PCamera* TestCam = dynamic_cast<PCamera*>(WorldObjects[i]);
		if (TestCam && TestCam->GetIsActive())
		{
			return TestCam;
		}
	}

	return nullptr;
}

// Get the camera that will be used when the level is started.
PCamera* PEnvironment::GetLevelStartCamera()
{
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		PCamera* TestCam = dynamic_cast<PCamera*>(WorldObjects[i]);
		if (TestCam && TestCam->GetIsActiveOnStart())
		{
			return TestCam;
		}
	}

	return nullptr;
}

// Retreive a list of all cameras currently loaded into the game world.
std::vector<PCamera*> PEnvironment::GetCameras()
{
	std::vector<PCamera*> ReturnCameras;
	PCamera* TestCam = nullptr;
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		TestCam = dynamic_cast<PCamera*>(WorldObjects[i]);
		if (TestCam)
		{
			ReturnCameras.push_back(TestCam);
		}
	}

	return ReturnCameras;
}

// Retreive a list of all Static Meshes in the game world.
std::vector<PStaticMesh*> PEnvironment::GetStaticMeshes()
{
	std::vector<PStaticMesh*> ReturnCameras;
	PStaticMesh* TestCam = nullptr;
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		TestCam = dynamic_cast<PStaticMesh*>(WorldObjects[i]);
		if (TestCam && !(dynamic_cast<PSkeletalMesh*>(WorldObjects[i])))
		{
			ReturnCameras.push_back(TestCam);
		}
	}

	return ReturnCameras;
}

// Retreive a list of all Skeletal Meshes in the game world.
std::vector<PSkeletalMesh*> PEnvironment::GetSkeletalMeshes()
{
	std::vector<PSkeletalMesh*> ReturnMeshes;
	PSkeletalMesh* TestMesh = nullptr;
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		TestMesh = dynamic_cast<PSkeletalMesh*>(WorldObjects[i]);
		if (TestMesh)
		{
			ReturnMeshes.push_back(TestMesh);
		}
	}

	return ReturnMeshes;
}

// Retreive a list of all Characters in the game world.
std::vector<PCharacter*> PEnvironment::GetCharacters()
{
	std::vector<PCharacter*> ReturnCharacters;
	PCharacter* TestCam = nullptr;
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		TestCam = dynamic_cast<PCharacter*>(WorldObjects[i]);
		if (TestCam)
		{
			ReturnCharacters.push_back(TestCam);
		}
	}

	return ReturnCharacters;
}

// Retreive a list of all lights of all types currently loaded into the game world.
std::vector<PLight*> PEnvironment::GetLights()
{
	std::vector<PLight*> ReturnLights;
	PLight* TestLight = nullptr;
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		TestLight = dynamic_cast<PLight*>(WorldObjects[i]);
		if (TestLight)
		{
			ReturnLights.push_back(TestLight);
		}
	}

	return ReturnLights;
}

// Retreive a list of all point light objects currently loaded in the world.
std::vector<PPointLight*> PEnvironment::GetPointLights()
{
	std::vector<PPointLight*> ReturnLights;
	PPointLight* TestLight = nullptr;
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		TestLight = dynamic_cast<PPointLight*>(WorldObjects[i]);
		if (TestLight)
		{
			ReturnLights.push_back(TestLight);
		}
	}

	return ReturnLights;
}

// Retreive a list of all directional lights currently loaded into the environment.
std::vector<PDirectionalLight*> PEnvironment::GetDirectionalLights()
{
	std::vector<PDirectionalLight*> ReturnLights;
	PDirectionalLight* TestLight = nullptr;
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		TestLight = dynamic_cast<PDirectionalLight*>(WorldObjects[i]);
		if (TestLight)
		{
			ReturnLights.push_back(TestLight);
		}
	}

	return ReturnLights;
}

// Get the current sunlight source. If there are more than one sunlight, this will return
// the active light if one is hidden, or the first light found otherwise.
PDirectionalLight* PEnvironment::GetSunLight()
{
	PDirectionalLight* TestLight = nullptr;
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		TestLight = dynamic_cast<PDirectionalLight*>(WorldObjects[i]);
		if (TestLight && TestLight->GetVisibility())
		{
			return TestLight;
		}
	}
	return nullptr;
}

// Set the currently active camera in the game world, that is to set the camera that is viewing
// the game world.
void PEnvironment::SetActiveCamera(PCamera* InCam)
{
	InCam->SetActive(true);

	PCamera* TestCam = GetActiveCamera();
	if (TestCam && (TestCam != InCam))
	{
		TestCam->SetActive(false);
	}

	InCam->SetFieldOfView(InCam->GetFieldOfView());
}

// Set the currently active camera in the game world, that is to set the camera that is viewing
// the game world.
void PEnvironment::SetLevelStartCamera(PCamera* InCam)
{
	InCam->GetIsActiveOnStart() = true;

	std::vector<PCamera*> Cams = GetCameras();

	for (unsigned int i = 0; i < Cams.size(); ++i)
	{
		if (Cams[i] && Cams[i]->GetDisplayName() != InCam->GetDisplayName() && Cams[i]->GetIsActiveOnStart())
		{
			Cams[i]->GetIsActiveOnStart() = false;
		}
	}
}

// Clear Environment will clear all objects from the Environment. This is used for multiple things such as destroying the
// renderer or creating a new level.
void PEnvironment::ClearEnvironment(bool bShutdown, bool bNoLevel)
{
	SelectedObject = nullptr;

	// Delete controllers.
	//
	for (unsigned int i = 0; i < WorldControllers.size(); ++i)
	{
		if (WorldControllers[i] && (!GetActiveCamera() || !(GetActiveCamera()->Controller) || (WorldControllers[i] != GetActiveCamera()->Controller) || bShutdown))
		{
			delete WorldControllers[i];
			WorldControllers.erase(WorldControllers.begin() + i);
			--i;
		}
	}

	// Delete objects.
	//
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		if (WorldObjects[i] && (!GetActiveCamera() || ((WorldObjects[i]->GetDisplayName() != GetActiveCamera()->GetDisplayName())) || bShutdown))
		{
			WorldObjects[i]->Destroy();
			delete WorldObjects[i];
			WorldObjects.erase(WorldObjects.begin() + i);
			--i;
		}
	}

	if (bNoLevel)
	{
		CurrentLevel = "";
	}
}

// Save the state of the current level and all objects within the environment.
void PEnvironment::SaveLevel(std::string FilePath)
{
	// Flag of whether the file was created for this level when loading.
	bool bCreated = false;

	std::vector<std::string> LevelChunks = PGameplayStatics::SplitString(FilePath, '/');
	std::string LevelName = PGameplayStatics::SplitString(LevelChunks[LevelChunks.size() - 1], '.')[0];

	// Print that the level is saving and opening. Also setup the whole filepath.
	PrintToConsole("Saving level: \"" + LevelName + "\"", 4);
	std::string LevelFile = FilePath;
	PrintToConsole("Opening file: \"" + LevelFile + "\"");

	// Print if a new file had to be created for this level.
	if (!std::ifstream(LevelFile).good())
	{
		PrintToConsole("File does not exist, creating file: \"" + LevelFile + "\"", 3);
		bCreated = true;
	}

	std::fstream LevelStream(LevelFile, std::ios::out);

	LevelStream.close();

	LevelStream.open(LevelFile, std::fstream::out | std::fstream::trunc);

	if (LevelStream.is_open())
	{
		// Print if the file was created and is now open.
		if (bCreated)
		{
			PrintToConsole("File: \"" + LevelFile + "\"" + " has been created.", 1);
		}

		PrintToConsole("Opened file: \"" + LevelFile + "\"", 1);
		PrintToConsole("Saving level to file: \"" + LevelFile + "\"");

		LevelStream << "NAME " << LevelName << "\n";
		LevelStream << "AMBL " << AmbientLightIntensity << "\n";

		for (unsigned int i = 0; i < WorldObjects.size(); ++i)
		{
			PObject* CurrObject = WorldObjects[i];

			if (CurrObject)
			{
				// 0: Object, 1: Camera, 2: StaticMesh, 3: DirectionalLight, 4: PointLight
				unsigned int ObjType = 0;
				if (dynamic_cast<PCamera*>(CurrObject))
				{
					ObjType = 1;
				}
				else if (dynamic_cast<PStaticMesh*>(CurrObject))
				{
					PStaticMesh* TempMesh = dynamic_cast<PStaticMesh*>(CurrObject);
					PSkeletalMesh* TestSkMesh = dynamic_cast<PSkeletalMesh*>(CurrObject);
					if (TempMesh->PrimitiveType == 0)
					{
						if (TestSkMesh)
						{
							ObjType = 6;
						}
						else
						{
							ObjType = 2;
						}
					}
					else
					{
						ObjType = 5;
					}
				}
				else if (dynamic_cast<PDirectionalLight*>(CurrObject))
				{
					ObjType = 3;
				}
				else if (dynamic_cast<PPointLight*>(CurrObject))
				{
					ObjType = 4;
				}
				else
				{
					ObjType = 0;
				}

				if (ObjType != 1 || (dynamic_cast<PCamera*>(CurrObject)->GetDisplayName() != "RenderCamera"))
				{
					PrintToConsole("Saving object: \"" + CurrObject->GetDisplayName() + "\"");

					switch (ObjType)
					{
						case 0:
						{
							LevelStream << "OBJ";
							break;
						}
						case 1:
						{
							LevelStream << "CAM";
							break;
						}
						case 2:
						{
							LevelStream << "STM";
							break;
						}
						case 3:
						{
							LevelStream << "DIRL";
							break;
						}
						case 4:
						{
							LevelStream << "PNTL";
							break;
						}
						case 5:
						{
							// Primitive.
							LevelStream << "PRIMI";
							break;
						}
						case 6:
						{
							// Primitive.
							LevelStream << "SKM";
							break;
						}
					}

					// Output Display Name.
					LevelStream << " " << CurrObject->GetDisplayName();

					// Output the object visibility.
					LevelStream << " " << CurrObject->GetVisibility();

					// Output the World Matrix.
					LevelStream << " " << CurrObject->GetWorld().ViewMatrix[0][0] << " " << CurrObject->GetWorld().ViewMatrix[0][1] << " " << CurrObject->GetWorld().ViewMatrix[0][2] << " " << CurrObject->GetWorld().ViewMatrix[0][3] << " "
						<< CurrObject->GetWorld().ViewMatrix[1][0] << " " << CurrObject->GetWorld().ViewMatrix[1][1] << " " << CurrObject->GetWorld().ViewMatrix[1][2] << " " << CurrObject->GetWorld().ViewMatrix[1][3] << " "
						<< CurrObject->GetWorld().ViewMatrix[2][0] << " " << CurrObject->GetWorld().ViewMatrix[2][1] << " " << CurrObject->GetWorld().ViewMatrix[2][2] << " " << CurrObject->GetWorld().ViewMatrix[2][3] << " "
						<< CurrObject->GetWorld().ViewMatrix[3][0] << " " << CurrObject->GetWorld().ViewMatrix[3][1] << " " << CurrObject->GetWorld().ViewMatrix[3][2] << " " << CurrObject->GetWorld().ViewMatrix[3][3];

					// Output the Local Matrix.
					LevelStream << " " << CurrObject->GetLocal().r[0].m128_f32[0] << " " << CurrObject->GetLocal().r[0].m128_f32[1] << " " << CurrObject->GetLocal().r[0].m128_f32[2] << " " << CurrObject->GetLocal().r[0].m128_f32[3] << " "
						<< CurrObject->GetLocal().r[1].m128_f32[0] << " " << CurrObject->GetLocal().r[1].m128_f32[1] << " " << CurrObject->GetLocal().r[1].m128_f32[2] << " " << CurrObject->GetLocal().r[1].m128_f32[3] << " "
						<< CurrObject->GetLocal().r[2].m128_f32[0] << " " << CurrObject->GetLocal().r[2].m128_f32[1] << " " << CurrObject->GetLocal().r[2].m128_f32[2] << " " << CurrObject->GetLocal().r[2].m128_f32[3] << " "
						<< CurrObject->GetLocal().r[3].m128_f32[0] << " " << CurrObject->GetLocal().r[3].m128_f32[1] << " " << CurrObject->GetLocal().r[3].m128_f32[2] << " " << CurrObject->GetLocal().r[3].m128_f32[3];

					// Output the name of the parent object, if any.
					if (CurrObject->ParentObject)
					{
						LevelStream << " " << CurrObject->ParentObject->GetDisplayName();
					}
					else
					{
						LevelStream << " nullptr";
					}

					// Output whether this object has a controller.
					LevelStream << " " << (CurrObject->Controller != nullptr);

					// Output whether this object should begin the game with a controller.
					LevelStream << " " << CurrObject->Ctrl_bStartWithController;

					// Output whether this object has a controller.
					LevelStream << " " << (CurrObject->GetInputEnabled());

					// Ouput whether this object is hidden in game.
					LevelStream << " " << (CurrObject->GetHiddenInGame());

					// Below adds save data for Cameras.
					// Output field of view.
					if (ObjType == 1)
					{
						PCamera* PCam = dynamic_cast<PCamera*>(CurrObject);

						LevelStream << " " << PCam->GetFieldOfView();
						LevelStream << " " << PCam->GetIsActive();
						LevelStream << " " << PCam->GetIsActiveOnStart();
					}

					// Below adds save data for Static Meshes and Primitive Static Meshes.
					// Output Model and File filepaths.
					if (ObjType == 2 || ObjType == 5 || ObjType == 6)
					{
						PStaticMesh* SMesh = dynamic_cast<PStaticMesh*>(CurrObject);

						if (SMesh != nullptr)
						{
							std::string SpecFile = (SMesh->Specular_DDSFile != "") ? (SMesh->Specular_DDSFile) : "None";
							std::string EmiFile = (SMesh->Emissive_DDSFile != "") ? (SMesh->Emissive_DDSFile) : "None";

							LevelStream << " " << SMesh->ModelFile;
							LevelStream << " " << SMesh->DDSFile;
							LevelStream << " " << SpecFile;
							LevelStream << " " << EmiFile;
							LevelStream << " " << SMesh->Material.Specular[0];
							LevelStream << " " << SMesh->Material.Emissive[0];
							LevelStream << " " << SMesh->Material.Emissive[1];
							LevelStream << " " << SMesh->Material.Emissive[2];

							LevelStream << " " << SMesh->Col_bEnableCollision;
							LevelStream << " " << SMesh->GetBoundingBoxExtents().x << " " << SMesh->GetBoundingBoxExtents().y << " " << SMesh->GetBoundingBoxExtents().z;
							LevelStream << " " << SMesh->GetBoundingBoxOffset().x << " " << SMesh->GetBoundingBoxOffset().y << " " << SMesh->GetBoundingBoxOffset().z;

							if (ObjType == 5)
							{
								LevelStream << " " << std::to_string(SMesh->PrimitiveType);
							}
						}
						else
						{
							PrintToConsole("There was an issue saving the texture and model filenames.", 3);
						}
					}

					// Below adds save data for Generic and Point Lights.
					if (ObjType == 3 || ObjType == 4)
					{
						PLight* SMesh = dynamic_cast<PLight*>(CurrObject);

						LevelStream << " " << SMesh->Intensity << " " << SMesh->Color.x << " " << SMesh->Color.y << " " << SMesh->Color.z << " " << SMesh->Color.w;
					}

					if (ObjType == 4)
					{
						PPointLight* SLight = dynamic_cast<PPointLight*>(CurrObject);

						LevelStream << " " << SLight->Radius;
					}

					LevelStream << "\n";

					PrintToConsole(" - Object saved: \"" + CurrObject->GetDisplayName() + "\"");
				}
			}
			else
			{
				PrintToConsole(" - Could not save object: \"" + CurrObject->GetDisplayName() + "\". It seems to have been corrupt.", 2);
			}
		}

		LevelStream.close();

		PrintToConsole("Level: \"" + LevelName + "\" has been saved in file: \"" + LevelFile + "\"", 1);
	}
	else
	{
		PrintToConsole("Could not open file while attempting to save level: \"" + LevelFile + "\"", 2);
	}
}

// Load a level from a saved ".plevel" file and create all objects associated with its content.
void PEnvironment::LoadLevel(std::string FilePath)
{
	std::vector<std::string> LevelChunks = PGameplayStatics::SplitString(FilePath, '\\');

	if (LevelChunks.size() <= 1)
	{
		LevelChunks = PGameplayStatics::SplitString(FilePath, '/');
	}

	// Only continue if the file has more than one name attached, ensuring a directory is included.
	if (LevelChunks.size() > 1)
	{
		std::string LevelName = PGameplayStatics::SplitString(LevelChunks[LevelChunks.size() - 1], '.')[0];

		ClearEnvironment(false);

		// Print that the level is loading. Also setup the whole filepath.
		PrintToConsole("Loading level: \"" + LevelName + "\"", 4);
		std::string LevelFile = FilePath;
		PrintToConsole("Opening file: \"" + LevelFile + "\"");

		// Print if a new file had to be created for this level.
		if (std::ifstream(LevelFile).good())
		{
			std::ifstream LevelStream(LevelFile);

			if (LevelStream.is_open())
			{
				PrintToConsole("Opened file: \"" + LevelFile + "\"", 1);

				// Set the new current file for the level.
				CurrentLevel = FilePath;

				// Container for the current line of the file.
				std::string CurrLine;

				// Objects to attach to each other once the load is completed.
				std::vector<PObject*> Children;
				std::vector<std::string> Parents;

				// Read each line of the file. Get the next line in the list of objects.
				while (std::getline(LevelStream, CurrLine))
				{
					std::istringstream ISS(CurrLine);
					std::vector<std::string> Chunks((std::istream_iterator<std::string>(ISS)), (std::istream_iterator<std::string>()));

					if (Chunks[0] != "NAME" && Chunks[0] != "AMBL" && Chunks[0] != "" && Chunks[0] != "#")
					{
						// The object that was created during this line cycle.
						PObject* CurrObject = nullptr;

						// Objects.
						std::string ObjectName = Chunks[1];
						DirectX::XMMATRIX World;
						DirectX::XMMATRIX Local;
						bool bIsVisible = false;
						bool bIsHiddenInGame = false;
						bool bHasController = false;
						bool bStartWithController = false;
						bool bIsInputEnabled = false;
						std::string ParentName = "nullptr";

						// Static Meshes.
						std::string ModelFilePath;
						std::string  TextureFilePath;
						std::string  SpecularFilePath;
						std::string  EmissiveFilePath;
						float SpecAddative;
						PMath::float3 EmissiveAddative;
						bool bCollisionEnable;
						float3 BBExtents;
						float3 BBOffset;

						// Cameras.
						float Cam_FOV = 90.0f;
						bool bCam_IsActive = false;
						bool bCam_IsActiveOnStart = false;

						// Lighting.
						float LightIntensity = 0.0f;
						float LightRadius = 0.0f;
						float4 LightColor = { 0.0f, 0.0f, 0.0f, 0.0f };

						if (Chunks[0] == "OBJ" || Chunks[0] == "CAM" || Chunks[0] == "STM" || Chunks[0] == "SKM" || Chunks[0] == "PRIMI" || Chunks[0] == "DIRL" || Chunks[0] == "PNTL")
						{
							bIsVisible = (Chunks[2] == "1");
							World = { stof(Chunks[3]), stof(Chunks[4]), stof(Chunks[5]), stof(Chunks[6]),
									stof(Chunks[7]), stof(Chunks[8]), stof(Chunks[9]), stof(Chunks[10]),
									stof(Chunks[11]), stof(Chunks[12]), stof(Chunks[13]), stof(Chunks[14]),
									stof(Chunks[15]), stof(Chunks[16]), stof(Chunks[17]), stof(Chunks[18]) };
							Local = { stof(Chunks[19]), stof(Chunks[20]), stof(Chunks[21]), stof(Chunks[22]),
									stof(Chunks[23]), stof(Chunks[24]), stof(Chunks[25]), stof(Chunks[26]),
									stof(Chunks[27]), stof(Chunks[28]), stof(Chunks[29]), stof(Chunks[30]),
									stof(Chunks[31]), stof(Chunks[32]), stof(Chunks[33]), stof(Chunks[34]) };
							ParentName = Chunks[35];
							bHasController = (Chunks[36] == "1");
							bStartWithController = (Chunks[37] == "1");
							bIsInputEnabled = (Chunks[38] == "1");
							bIsHiddenInGame = (Chunks[39] == "1");

							if (Chunks[0] == "DIRL" || Chunks[0] == "PNTL")
							{
								LightIntensity = stof(Chunks[40]);
								LightColor = { stof(Chunks[41]), stof(Chunks[42]), stof(Chunks[43]), stof(Chunks[44]) };
							}
							if (Chunks[0] == "PNTL")
							{
								LightRadius = stof(Chunks[45]);
							}
							if (Chunks[0] == "STM" || Chunks[0] == "SKM" || Chunks[0] == "PRIMI")
							{
								ModelFilePath = Chunks[40];
								TextureFilePath = Chunks[41];
								SpecularFilePath = Chunks[42];
								EmissiveFilePath = Chunks[43];
								SpecAddative = stof(Chunks[44]);
								EmissiveAddative = { stof(Chunks[45]), stof(Chunks[46]), stof(Chunks[47]) };

								bCollisionEnable = (Chunks[48] == "1");
								BBExtents = { stof(Chunks[49]), stof(Chunks[50]), stof(Chunks[51]) };
								BBOffset = { stof(Chunks[52]), stof(Chunks[53]), stof(Chunks[54]) };

							}
							if (Chunks[0] == "CAM")
							{
								Cam_FOV = stof(Chunks[40]);
								bCam_IsActive = (Chunks[41] == "1");
								bCam_IsActiveOnStart = (Chunks[42] == "1");
							}
						}

						// Handle a generic Object.
						if (Chunks[0] == "OBJ")
						{
							PObject* NewPObject = CreateObject(bIsVisible, ObjectName);
							PrintToConsole("Object: " + NewPObject->GetDisplayName() + " was created.");

							CurrObject = NewPObject;
						}
						// Handle Cameras.
						else if (Chunks[0] == "CAM")
						{
							PCamera* NewCamera = CreateCamera(Cam_FOV, bIsInputEnabled, bCam_IsActive, ObjectName);
							NewCamera->GetIsActiveOnStart() = bCam_IsActiveOnStart;
							PrintToConsole("Camera: " + NewCamera->GetDisplayName() + " was created.");

							CurrObject = NewCamera;
						}
						// Handle StaticMeshes.
						else if (Chunks[0] == "STM")
						{
							PStaticMesh* NewStaticMesh = CreateStaticMesh(ModelFilePath.c_str(), TextureFilePath.c_str(), bIsVisible, ObjectName, nullptr);
							PMaterial Material(SpecAddative, EmissiveAddative);

							NewStaticMesh->Col_bEnableCollision = bCollisionEnable;
							NewStaticMesh->SetBoundingBoxExtents(BBExtents);
							NewStaticMesh->SetBoundingBoxOffset(BBOffset);
							NewStaticMesh->SetMaterial(Material);
							PrintToConsole("Static Mesh: " + NewStaticMesh->GetDisplayName() + " was created.");

							CurrObject = NewStaticMesh;
						}
						// Handle SkeletalMeshes.
						else if (Chunks[0] == "SKM")
						{
							PSkeletalMesh* NewSkeletalMesh = CreateSkeletalMesh(ModelFilePath.c_str(), TextureFilePath.c_str(), SpecularFilePath.c_str(), EmissiveFilePath.c_str(), bIsVisible, ObjectName, nullptr);
							NewSkeletalMesh->Col_bEnableCollision = bCollisionEnable;
							NewSkeletalMesh->SetBoundingBoxExtents(BBExtents);
							NewSkeletalMesh->SetBoundingBoxOffset(BBOffset);
							PrintToConsole("Static Mesh: " + NewSkeletalMesh->GetDisplayName() + " was created.");

							CurrObject = NewSkeletalMesh;
						}
						else if (Chunks[0] == "PRIMI")
						{
							EPrimitives PrimStruc;
							unsigned int PrimType = stoi(Chunks[55]);
							if (PrimType >= EPrimitives::NONE && PrimType <= EPrimitives::LAST)
							{
								PrimStruc = static_cast<EPrimitives>(PrimType);
							}

							PStaticMesh* NewPrimMesh = CreatePrimitive(PrimStruc, bIsVisible, ObjectName, nullptr);
							NewPrimMesh->LoadTexture(TextureFilePath.c_str(), Device, 0);

							if (SpecularFilePath != "None")
							{
								NewPrimMesh->LoadTexture(SpecularFilePath.c_str(), Device, 2);
							}

							if (EmissiveFilePath != "None")
							{
								NewPrimMesh->LoadTexture(EmissiveFilePath.c_str(), Device, 3);
							}

							NewPrimMesh->Col_bEnableCollision = bCollisionEnable;
							NewPrimMesh->SetBoundingBoxExtents(BBExtents);
							NewPrimMesh->SetBoundingBoxOffset(BBOffset);
							PrintToConsole("Primitive Mesh: " + NewPrimMesh->GetDisplayName() + " was created.");

							CurrObject = NewPrimMesh;
						}
						// Handle Directional Lights.
						else if (Chunks[0] == "DIRL")
						{
							PDirectionalLight* NewDirLight = CreateDirectionalLight(LightIntensity, float3{ 0, 0, 0 }, LightColor, ObjectName);
							PrintToConsole("Light: " + NewDirLight->GetDisplayName() + " was created.");

							CurrObject = NewDirLight;
						}
						// Handle Point Lights.
						else if (Chunks[0] == "PNTL")
						{
							PPointLight* NewPointLight = CreatePointLight(LightIntensity, LightRadius, LightColor, ObjectName);
							PrintToConsole("Light: " + NewPointLight->GetDisplayName() + " was created.");

							CurrObject = NewPointLight;
						}

						// Setup information that applies to all objects in the game world.
						if (Chunks[0] == "OBJ" || Chunks[0] == "CAM" || Chunks[0] == "STM" || Chunks[0] == "SKM" || Chunks[0] == "DIRL" || Chunks[0] == "PNTL" || Chunks[0] == "PRIMI")
						{
							CurrObject->Ctrl_bStartWithController = bStartWithController;

							// Create a controller for this object if it needs a controller.
							if (bHasController || bStartWithController)
							{
								PrintToConsole("Creating controller for: " + CurrObject->GetDisplayName(), 4);
								CurrObject->PossessController(CreateController(), bIsInputEnabled);
								PrintToConsole("Controller created and assigned to: " + CurrObject->GetDisplayName(), 1);
							}

							CurrObject->DefaultWorld.ViewMatrix = (float4x4_a&)World;
							CurrObject->LocalMatrix = Local;
							CurrObject->SetVisibility(bIsVisible);
							CurrObject->SetInputEnabled(bIsInputEnabled);
							CurrObject->SetMovementEnabled(bIsInputEnabled);

							// Check for parent to attach to, if it has a parent check to see if the parent was created yet. If it was, attach now, if not, add to the list to attach when all objects have been created.
							if (ParentName != "nullptr")
							{
								PObject* Parent = GetObjectByName(ParentName);
								if (Parent)
								{
									CurrObject->AttachToObject(Parent);
									PrintToConsole("Attached Object: " + CurrObject->GetDisplayName() + " to parent: " + Parent->GetDisplayName() + ".");
								}
								else
								{
									Children.push_back(CurrObject);
									Parents.push_back(ParentName);
								}
							}
						}

						CurrObject->SetHiddenInGame(bIsHiddenInGame);
					}
					else if (Chunks[0] == "AMBL")
					{
						AmbientLightIntensity = stof(Chunks[1]);
					}
				}

				for (unsigned int i = 0; i < Children.size(); ++i)
				{
					PObject* Child = Children[i];
					PObject* Parent = GetObjectByName(Parents[i]);

					if (Child && Parent)
					{
						Child->AttachToObject(Parent);
						PrintToConsole("Attached Object: " + Child->GetDisplayName() + " to parent: " + Parent->GetDisplayName() + ".");
					}
				}


				PrintToConsole("Level: \"" + LevelName + "\" has been loaded from file: \"" + LevelFile + "\"", 1);
			}
			else
			{
				PrintToConsole("Could not open file while attempting to load level: \"" + LevelFile + "\"", 2);
			}

			if (LevelStream.is_open())
			{
				LevelStream.close();
			}
		}
		else
		{
			PrintToConsole("File: \"" + LevelFile + "\" does not exist! The level cannot be loaded, please check the spelling of the level name, and be sure to use \"/\" to append any folders you create for levels (normal C++ file navigation syntax).", 2);
		}
	}
	else
	{
		PrintToConsole("Improperly formatted level name.", 2);
	}
}

// This will return the camera being used by the editor (not the game) to render the game viewport.
PCamera* PEnvironment::GetRenderCamera()
{
	for (unsigned int i = 0; i < WorldObjects.size(); ++i)
	{
		if (WorldObjects[i] && (WorldObjects[i]->GetDisplayName() == "RenderCamera"))
		{
			PCamera* RenderCam = dynamic_cast<PCamera*>(WorldObjects[i]);
			if (RenderCam)
			{
				return RenderCam;
			}
		}
	}

	return nullptr;
}

// Begin a test game for the loaded level.
void PEnvironment::BeginPlaytest()
{
	SaveLevel(CurrentLevel);

	PrintToConsole("Starting Testing mode.", 4);
	CurrentState = ERenderStates::TEST;
	LoadLevel(CurrentLevel);
	SetActiveCamera(GetLevelStartCamera());
}

// Finish the currently active playtest session.
void PEnvironment::EndPlaytest()
{
	PrintToConsole("Exiting Testing mode.", 4);
	CurrentState = ERenderStates::DEBUG;
	SetActiveCamera(GetRenderCamera());
	LoadLevel(CurrentLevel);
	GetRenderCamera()->SetInputEnabled(true);
	GetRenderCamera()->SetMovementEnabled(true);
}

// Test every PCharacter against every Mesh in the game world with collision enabled to see if the Character is colliding.
void PEnvironment::CheckObjectCollisions()
{
	std::vector<PCharacter*> Chars = GetCharacters();
	std::vector<PStaticMesh*> Meshes = GetStaticMeshes();

	if (Chars.size() > 0)
	{
		for (unsigned int i = 0; i < Chars.size(); ++i)
		{
			PCharacter* CurrChar = Chars[i];

			for (unsigned int j = 0; j < Meshes.size(); ++j)
			{
				PStaticMesh* CurrAntiMesh = Meshes[i];

				// Ensure the Mesh is valid and that you are not testing a mesh against itself.
				if (Meshes[j] != nullptr && CurrAntiMesh->GetDisplayName() != CurrChar->GetDisplayName())
				{
					
				}
			}
		}
	}
}

void PEnvironment::CreateCustomClass(std::string FilePath, EClasses Type)
{
	// Flag of whether the file was created for this level when loading.
	bool bCreated = false;

	std::vector<std::string> CustomChunks = PGameplayStatics::SplitString(FilePath, '\\');
	std::string CustomName = PGameplayStatics::SplitString(CustomChunks[CustomChunks.size() - 1], '.')[0];
	std::string SourcePath = PGameplayStatics::SplitString(FilePath, '.')[0];

	// Print that the level is saving and opening. Also setup the whole filepath.
	PrintToConsole("Creating class: \"" + CustomName + "\"", 4);
	std::string LevelFile = FilePath;
	PrintToConsole("Attempting to open a file: \"" + LevelFile + "\"");

	unsigned int TestInt = ((CustomChunks.size() - PGameplayStatics::SplitString(PGameplayStatics::GetMainDirectory(), '/').size()) - 1);

	//
	// The following is the header file creation.
	//
	{
		// Print if a new file had to be created for this level.
		if (!std::ifstream(LevelFile).good())
		{
			PrintToConsole("File does not exist, creating file: \"" + LevelFile + "\"", 3);
			bCreated = true;
		}

		std::fstream LevelStream(LevelFile, std::ios::out);

		LevelStream.close();

		LevelStream.open(LevelFile, std::fstream::out | std::fstream::trunc);

		if (LevelStream.is_open())
		{
			// Print if the file was created and is now open.
			if (bCreated)
			{
				PrintToConsole("File: \"" + LevelFile + "\"" + " has been created and opened.", 1);
				PrintToConsole("Saving Class to file: \"" + LevelFile + "\"");

				if (Type == EClasses::OBJECT)
				{
					LevelStream << "// Made with Polyngine.\n\n" << 
						"#pragma once\n\n" <<
						"#include \"";

					for (unsigned int i = 0; i < TestInt; ++i)
					{
						LevelStream << "../";
					}

					LevelStream <<
						"PolynGameEngine/PObjects/PObject/PObject.h\"\n" <<
						"#include \"";
					
					for (unsigned int i = 0; i < TestInt; ++i)
					{
						LevelStream << "../";
					}

					LevelStream << "PolynGameEngine/PMath/PMath.h\"\n\n" <<
						"/**\n" << 
						"* !!! REPLACE THIS COMMENT !!!\n" <<
						"* Type out a quick description of what this class is and what purposes it serves. Maybe even breakdown the parts of it in a \n" << 
						"* brief paragraph.\n" <<
						"*/\n" << 
						"class " << CustomName << " : public PObject\n{\n" <<
						"public:\n" <<
						"    /**\n	* Constructors for this object. These are called in the engine and game.\n	*/\n" <<
						"    " << CustomName << "();\n\n" <<
						"    /**\n	* This is called when the object is finished being loaded into the world and after the constructors have been\n	* called.\n	*/\n" <<
						"    void BeginPlay();\n\n" <<
						"    /**\n	* Update() is called once per frame. The DeltaTime is the time between the current and previous frame.\n	*/\n" <<
						"    void Update(float DeltaTime);\n\n" <<
						"    /**\n	* Manually call destroy to delete this object from the game world. Only destroy using this function.\n	*/\n" <<
						"    void Destroy();\n\n" <<
						"    /**\n	* EndPlay() is called before the object finished being Destroyed. This should cleanup all memory used.\n	*/\n" <<
						"    void EndPlay();\n" <<
						"};";
				}
				else if (Type == EClasses::CONTROLLER)
				{
					LevelStream << "// Made with Polyngine.\n\n" <<
						"#pragma once\n\n" <<
						"#include \"";

					for (unsigned int i = 0; i < TestInt; ++i)
					{
						LevelStream << "../";
					}

					LevelStream <<
						"PolynGameEngine/PSystem/PController/PController.h\"\n" <<
						"#include \"";

					for (unsigned int i = 0; i < TestInt; ++i)
					{
						LevelStream << "../";
					}

					LevelStream << "PolynGameEngine/PMath/PMath.h\"\n\n" <<
						"/**\n" << 
						"* !!! REPLACE THIS COMMENT !!!\n" <<
						"* Type out a quick description of what this class is and what purposes it serves. Maybe even breakdown the parts of it in a \n" << 
						"* brief paragraph.\n" <<
						"*/\n" << 
						"class " << CustomName << " : public PController\n{\n" <<
						"public:\n" <<
						"    /**\n	* Constructors for this object. These are called in the engine and game.\n	*/\n" <<
						"    " << CustomName << "();\n\n" <<
						"    /**\n	* Update() is called once per frame. The DeltaTime is the time between the current and previous frame.\n	*/\n" <<
						"    void Update(float DeltaTime);\n\n" <<
						"};";
				}

				PrintToConsole("Controller Class: \"" + CustomName + "\" has been saved in file: \"" + LevelFile + "\"", 1);
			}
			else
			{
				PrintToConsole("A controller or object aleady exists there, please pick a new name or path and try again.", 3);
			}

			LevelStream.close();

			ShellExecute(NULL, "open", FilePath.c_str(), NULL, NULL, SW_SHOW);
		}
		else
		{
			PrintToConsole("Could not open file while attempting to save Class: \"" + LevelFile + "\"", 2);
		}
	}

	//
	// The following is the source file creation.
	//
	{
		LevelFile = SourcePath + ".cpp";
		std::string LevelFileFilter = PGameplayStatics::ReplaceString(LevelFile, '\\', '/');

		// Print if a new file had to be created for this level.
		if (!std::ifstream(LevelFile).good())
		{
			PrintToConsole("File does not exist, creating file: \"" + LevelFile + "\"", 3);
			bCreated = true;
		}

		std::fstream LevelStream(LevelFile, std::ios::out);

		LevelStream.close();

		LevelStream.open(LevelFile, std::fstream::out | std::fstream::trunc);

		if (LevelStream.is_open())
		{
			// Print if the file was created and is now open.
			if (bCreated)
			{
				PrintToConsole("File: \"" + LevelFile + "\"" + " has been created.", 1);

				PrintToConsole("Opened file: \"" + LevelFile + "\"", 1);
				PrintToConsole("Saving Class to file: \"" + LevelFile + "\"");

				if (Type == EClasses::OBJECT)
				{
					LevelStream <<
						"#include \"" << CustomName << ".h\"\n\n" <<
						"/**\n* Constructors for this object. These are called in the engine and game.\n*/\n" <<
						CustomName << "::" + CustomName + "()\n" <<
						"{\n" <<
						"    // Initialize this objects default parameters here.\n\n" <<
						"    // BeginPlay() is the last thing that should be called in all Constructors.\n" <<
						"    BeginPlay();\n" <<
						"}\n\n" <<
						"/**\n* This is called when the object is finished being loaded into the world and after the constructors have been\n* called.\n*/\n" <<
						"void " + CustomName + "::BeginPlay()\n" <<
						"{\n" <<
						"    // Anything below is executed when this object has been loaded into the world and initialized.\n\n" <<
						"    // Call parent class base function after the rest of the function executes.\n" <<
						"    PObject::BeginPlay();\n" <<
						"}\n\n" <<
						"/**\n* Update() is called once per frame. The DeltaTime is the time between the current and previous frame.\n*/\n" <<
						"void " + CustomName + "::Update(float DeltaTime)\n" <<
						"{\n" <<
						"    // Anything below will execute very often, and you can use DeltaTime to make things lerp over time.\n\n" <<
						"    // Call parent class base function after the rest of the function executes.\n" <<
						"    PObject::Update(DeltaTime);\n" <<
						"}\n\n" <<
						"/**\n* Manually call destroy to delete this object from the game world. Only destroy using this function.\n*/\n" <<
						"void " + CustomName + "::Destroy()\n" <<
						"{\n" <<
						"    // This will be the code executed when this object is told to be deleted. This may not be called always, but EndPlay() will.\n\n" <<
						"    // Call parent class base function after the rest of the function executes.\n" <<
						"    PObject::Destroy();\n\n" <<
						"    // Fire off the EndPlay() function after destroying.\n" <<
						"    EndPlay();\n" <<
						"}\n\n" <<
						"/**\n* EndPlay() is called before the object finished being Destroyed. This should cleanup all memory used.\n*/\n" <<
						"void " + CustomName + "::EndPlay()\n" <<
						"{\n" <<
						"    // The following code will execute with positivity before this object is destroyed.\n\n" <<
						"    // Call parent class base function after the rest of the function executes.\n" <<
						"    PObject::EndPlay();\n" <<
						"}\n";
				}
				else if (Type == EClasses::CONTROLLER)
				{
					LevelStream <<
						"#include \"" << CustomName << ".h\"\n\n" <<
						"/**\n* Constructors for this object. These are called in the engine and game.\n*/\n" <<
						CustomName << "::" + CustomName + "()\n" <<
						"{\n" <<
						"    // Initialize this objects default parameters here.\n\n" <<
						"}\n\n" <<
						"/**\n* Update() is called once per frame. The DeltaTime is the time between the current and previous frame.\n*/\n" <<
						"void " + CustomName + "::Update(float DeltaTime)\n" <<
						"{\n" <<
						"    // Anything below will execute very often, and you can use DeltaTime to make things lerp over time.\n\n" <<
						"    // Call parent class base function after the rest of the function executes.\n" <<
						"    PController::Update(DeltaTime);\n" <<
						"}\n";
				}

				PrintToConsole("Controller class: \"" + CustomName + "\" has been saved in file: \"" + LevelFile + "\"", 1);
			}
			else
			{
				PrintToConsole("A controller or object aleady exists there, please pick a new name or path and try again.", 3);
			}

			LevelStream.close();

			ShellExecute(NULL, "open", LevelFile.c_str(), NULL, NULL, SW_SHOW);
		}
		else
		{
			PrintToConsole("Could not open file while attempting to save Class: \"" + LevelFile + "\"", 2);
		}
	}
}
