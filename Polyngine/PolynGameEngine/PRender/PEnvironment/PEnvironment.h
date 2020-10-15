#pragma once

#include "../../PObjects/PObject/PObject.h"
#include "../../PStatics/PGameplayStatics/PGameplayStatics.h"
#include "../../PObjects/PStaticMesh/PStaticMesh.h"
#include "../../PObjects/PSkeletalMesh/PSkeletalMesh.h"
#include "../../PObjects/PCharacter/PCharacter.h"
#include "../../PObjects/PCamera/PCamera.h"
#include "../../PObjects/PLights/PDirectionalLight/PDirectionalLight.h"
#include "../../PObjects/PLights/PPointLight/PPointLight.h"
#include "../../PSystem/PInputManager/PInputManager.h"
#include "../../PSystem/PController/PController.h"
#include "../../PSystem/Timer/PStopwatch/PStopwatch.h"

enum ERenderStates
{
	DEBUG = 0,
	TEST = 1,
	SHIP = 2
};

// Used to define the custom class creation types.
enum EClasses
{
	OBJECT,
	CONTROLLER
};

// Used to define primitives.
enum EPrimitives
{
	NONE = 0,
	PLANE = 1,
	CUBE = 2,
	SPHERE = 3,
	CONE = 4,
	LAST = 5
};

class PEnvironment
{
public:
	PEnvironment();

	std::string PPrimNames[6] = { "None", "Plane", "Cube", "Sphere", "Cone", "Last" };

	// This holds an output log item.
	struct POutput
	{
		std::string Data;
		int Status;
		std::string Source;
	};

	struct StopwatchCont
	{
		std::string Name;
		PStopwatch* Watch;
	};

	// The RenderState is the state the renderer is currently in.
	ERenderStates CurrentState;

	// The level that is currently loaded in.
	std::string CurrentLevel = "";
	std::string CurrentLevelName = "No Level Loaded";

	ID3D11Device* Device = nullptr;
	ID3D11DeviceContext* DeviceContext = nullptr;
	PInputManager* InputManager = nullptr;
	HINSTANCE hInst;
	bool bFlag_ShowDebugMatrices = true;
	bool bFlag_ShowDebugGrid = true;
	bool bFlag_ShowCollisionBoxes = true;

	PObject* SelectedObject = nullptr;
	std::vector<POutput> OutputLogData;
	float FPSRef = 0.0f;

	// Basic scene information and containers.
	//
	float AmbientLightIntensity = 0.05f;			// Ambient light intensity controls the base lighting for all objects in a scene.
	std::vector<PController*> WorldControllers;		// All PControllers in the game world.
	std::vector<PObject*> WorldObjects;				// All PObjects in the game world. This includes Meshes, Lights, and any other PObject derrived class.
	std::vector<StopwatchCont> WorldStopwatches;	// All PStopwatch objects in the game world.
	//
	//

	// React to the game starting, updating, and ending.
	//
	// Called once the Environment has finished setup. The first time it is safe to assume all members have been initialized.
	void BeginPlay();

	// Update is called once per frame update, and can alter values that change over time, using the DeltaTime (time in ms since the last frame change).
	void Update(float DeltaTime);

	// Called by Destroy. The first time it is safe to assume that all dynamic memory and references have been cleaned up.
	void EndPlay();

	// Called manually, and will call EndPlay on completion of execution. Destroy is responisble for cleaning up all memory before calling EndPlay.
	void Destroy();
	//
	//

	// Print some text to the output log (the console window behind the renderer). Status: 0) Text, 1) Success, 2) Error, 3) Warning.
	void PrintToConsole(std::string OutString, int Status = 0, std::string Source = "Environment");

	// Refresh all cameras Aspect Ratios.
	void RefreshCameraAspectRatios(float Aspect);

	// ----------------------------------------------------------------------------------------------->
	//		Below are the Creation helpers. Always use these to create objects in the world!
	//		When creating anything, leave the DebugName as "" to have one automatically created.
	// ----------------------------------------------------------------------------------------------->


	// Creates a Stopwatch and sets itself up to be handled by the environment.
	//
	// RETURN: The newly created Stopwatch.
	PStopwatch* CreateStopwatch(std::string Name, float Time, bool bShouldRecycle);

	// Helper functions to assist in creating all objects to be rendered or interacted with.
	//
	// Create a Controller that can be attached to an object to allow input processing.
	PController* CreateController();
	
	// Create a basic object. This object simply has interaction, no mesh or visuals. This can be used to attach other objects and create interactive scenarios.
	//
	// RETURN: The created PObject class.
	PObject* CreateObject(bool bVisible = true, std::string DebugName = "");

	// Create a camera that can visualize the world. You can change active cameras during rendering, but you must create them before you can render with them.
	//
	// RETURN: The created PCamera class.
	PCamera* CreateCamera(float FOV = 90.0f, bool bAssignInput = false, bool bSetActive = false, std::string DebugName = "");

	// Creates a primitive object with all vertex and index data.
	//
	// RETURN: The created PStaticMesh class.
	PStaticMesh* CreatePrimitive(EPrimitives Type, bool bVisible = true, std::string DebugName = "", PObject* Parent = nullptr, float3 InScale = { 1.0f, 1.0f, 1.0f });

	// Create a Static Mesh (OBJ file) that will be rendered (unless changed) by the renderer. This has all of the functionality of a basic object, plus visuals and texture information. You must specify a file name for the model (OBJ) and texture (DDS) files. An InputManager is optional, and can be supplied and updated later on after creation if this object at any point needs input. Scale input only adjusts world scale, local scale must be adjusted manually.
	//
	// RETURN: The created PStaticMesh class.
	PStaticMesh* CreateStaticMesh(const char* ModelFilePath, const char* DDSFilePath, bool bVisible = true, std::string DebugName = "", PObject* Parent = nullptr, float3 InScale = { 1.0f, 1.0f, 1.0f });

	// Create a Skeletal Mesh (MESH file) that will be rendered (unless changed) by the renderer. This has all of the functionality of a basic object and a PStaticMesh, plus bones and animation information. You must specify a file name for the model (MESH) and texture (DDS) files, and can optionally specify the Emissive and Specular textures. An InputManager is optional, and can be supplied and updated later on after creation if this object at any point needs input. Scale input only adjusts world scale, local scale must be adjusted manually.
	//
	// RETURN: The created PSkeletalMesh class.
	PSkeletalMesh* CreateSkeletalMesh(const char* MeshFilePath, const char* DDSFilePath, const char* Spec_DDSFilePath = std::string("none").c_str(), const char* Emissive_DDSFilePath = std::string("none").c_str(), bool bVisible = true, std::string DebugName = "", PObject* Parent = nullptr, float3 InScale = { 1.0f, 1.0f, 1.0f });

	// Create a PCharacter object to use for input, collision testing, and gameplay. You have the option of either creating a character with a normal mesh, or with a primitive.
	//
	// RETURN: The created PCharacter class.
	PCharacter* CreateCharacter(const char* ModelFilePath, const char* DDSFilePath, bool bVisible = true, std::string DebugName = "", PObject* Parent = nullptr, float3 InScale = { 1.0f, 1.0f, 1.0f });
	PCharacter* CreateCharacter(EPrimitives Type, bool bVisible = true, std::string DebugName = "", PObject* Parent = nullptr, float3 InScale = { 1.0f, 1.0f, 1.0f });

	// Create a Directional Light. Directional Lights are a PLight that has the ability to light all objects in a target direction, as if the "sun" had lighted it. Only one can ever be used at one time, but you may place many and change which is set as the Sunlight.
	//
	// RETURN: The created PDirectionalLight class.
	PDirectionalLight* CreateDirectionalLight(float Strength, float3 Dir, float4 Clr = { 1, 1, 1, 1 }, std::string DebugName = "");

	// Create a Point Light. Point Lights light an area locally from a source location, having the light bounds extend to a specified radius. The light will falloff as it travels, just as a normal streetlight or house-light.
	//
	// RETURN: The created PPointLight class.
	PPointLight* CreatePointLight(float Strength, float Rad, float4 Clr = { 1.0f, 1.0f, 1.0f, 1.0f }, std::string DebugName = "");

	// Destroy an object.
	//
	// RETURN: True if the object was found and destroyed, false otherwise.
	bool DestroyObject(PObject* Obj);


	// ----------------------------------------------------------------------------------------------->
	//		Below are the Sample helpers. Always use these if you need access to something in the world!
	// ----------------------------------------------------------------------------------------------->


	// Go through the world stopwatches and find one based on its name.
	// 
	// RETURN: The Stopwatch whose name matches the input, or nullptr if none were found.
	PStopwatch* GetStopwatchByName(std::string WatchName);

	// Go through the WorldObjects vector and find an object that has a debug name matching the passed in string name.
	// 
	// RETURN: The PObject with the queried name, or nullptr if no object was found.
	PObject* GetObjectByName(std::string ObjectName);

	// Go through the WorldObjects vector and find the camera that is currently being used to render the game world. You can consider this the eyes of the player, and the returned camera is the same that the renderer uses to project the world.
	// 
	// RETURN: The currently activated camera. Returns nullptr if no such Camera exists.
	PCamera* GetActiveCamera();

	// Get the camera that is set to be used when this level starts up.
	//
	// RETURN: The camera to be used on level start.
	PCamera* GetLevelStartCamera();

	// Go through the WorldObjects vector and find all PCamera objects, and compile a list of all of them.
	// 
	// RETURN: A vector<PCamera*> containing all PCameras in the world.
	std::vector<PCamera*> GetCameras();

	// Go through the WorldObjects vector and find all PStaticMesh objects, and compile a list of all of them.
	// 
	// RETURN: A vector<PStaticMesh*> containing all PStaticMesh in the world.
	std::vector<PStaticMesh*> GetStaticMeshes();

	// Go through the WorldObjects vector and find all PSkeletalMesh objects, and compile a list of all of them.
	// 
	// RETURN: A vector<PSkeletalMesh*> containing all PStaticMesh in the world.
	std::vector<PSkeletalMesh*> GetSkeletalMeshes();

	// Go through the WorldObjects vector and find all PCharacter objects, and compile a list of all of them.
	// 
	// RETURN: A vector<PStaticMesh*> containing all PStaticMesh in the world.
	std::vector<PCharacter*> GetCharacters();

	// Go through the WorldObjects vector and find the directional light object that is currently visible/being used as the sunlight.
	// 
	// RETURN: A PDirectionalLight* containing the current environment sunlight.
	PDirectionalLight* GetSunLight();

	// Go through the WorldObjects vector and find all lighting objects typed: PLight, PPointLight, and PDirectionalLight. The data will be stored in a vector of PLights.
	// 
	// RETURN: A vector<PLight*> containing all lights in the environment.
	std::vector<PLight*> GetLights();

	// Go through the WorldObjects vector and find all point light objects typed: PPointLight. The data will be stored in a vector of PPointLights.
	// 
	// RETURN: A vector<PPointLight*> containing all point lights in the environment.
	std::vector<PPointLight*> GetPointLights();

	// Go through the WorldObjects vector and find all directional light objects typed: PDirectionalLight. The data will be stored in a vector of PDirectionalLights.
	// 
	// RETURN: A vector<PDirectionalLight*> containing all directional lights in the environment.
	std::vector<PDirectionalLight*> GetDirectionalLights();


	// ----------------------------------------------------------------------------------------------->
	//		Below are the Setter Helpers for changing the state of the Environment.
	// ----------------------------------------------------------------------------------------------->


	// Set the active camera to an input camera. This will diable any other active cameras in the world and make the input camera the one to use for rendering.
	//
	// RETURN: This function does not return any data.
	void SetActiveCamera(PCamera* InCam);

	// Sets the camera that functions as the starting view for the level that is currently loaded. Unsets this option for all other cameras in the world.
	//
	// RETURN: This function does not return any data.
	void SetLevelStartCamera(PCamera* InCam);

	// ----------------------------------------------------------------------------------------------->
	//		Below are the functions for loading and saving game levels from the Environment.
	// ----------------------------------------------------------------------------------------------->


	// Save a level using the current Environment settings to a .plevel file. The filepath will automatically append the directory for /Levels/, you only need to add any subdirectories or filenames.
	void SaveLevel(std::string FilePath);

	// Load a level from a .plevel file into this Environment. The filepath will automatically append the directory for /Levels/, you only need to add any subdirectories or filenames.
	void LoadLevel(std::string FilePath);

	// Remove all objects and items from the environment.
	void ClearEnvironment(bool bShutdown = true, bool bNoLevel = false);


	// ----------------------------------------------------------------------------------------------->
	//		Below are the functions for retrieving file directories for the project.
	// ----------------------------------------------------------------------------------------------->


	// Returns the camera used for the viewport in the editor.
	PCamera* GetRenderCamera();


	// ----------------------------------------------------------------------------------------------->
	//		Below are the functions for starting and stopping a test game.
	// ----------------------------------------------------------------------------------------------->


	// Begin a test game for the loaded level.
	void BeginPlaytest();

	// Finish the currently active playtest session.
	void EndPlaytest();


	// ----------------------------------------------------------------------------------------------->
	//		Below are the Collision detection helpers.
	// ----------------------------------------------------------------------------------------------->


	// Cycles through all PCharacters in the environment and checks their collision against the world.
	//
	// RETURN: No return value.
	void CheckObjectCollisions();

	// Create a document for a new custom class.
	//
	// RETURN: No return value.
	void CreateCustomClass(std::string FilePath, EClasses Type);
};

