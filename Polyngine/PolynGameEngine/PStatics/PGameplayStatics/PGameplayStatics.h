#pragma once

#include <string>
#include <vector>

#include "../../PMath/PMath.h"

class PEnvironment;
class PController;
class PObject;
class PCamera;
class PPointLight;
class PStaticMesh;
class PSkeletalMesh;

class PGameplayStatics
{
private:
	static PEnvironment* ActiveEnvironment;

public:
	static int Number;

	// ----------------------------------------------------------------------------------------------->
	//		Below functions are for getting and setting gameplay globals, such as the current
	//		environment.
	// ----------------------------------------------------------------------------------------------->


	// Set the active gameplay environment.
	static void SetEnvironment(PEnvironment* Env);

	// Returns the current gameplay environment.
	static PEnvironment* GetEnvironment();

	// Returns the camera being used to render with. This can be the render camera.
	PCamera* GetActiveCamera();


	// ----------------------------------------------------------------------------------------------->
	//		Below functions are for creating and initializing objects in the game world.
	// ----------------------------------------------------------------------------------------------->


	// Creates a controller for use in game. The created PController object is returned.
	static PController* CreateController();

	// Create a camera that can visualize the world. This will return the created PCamera object.
	static PCamera* CreateCamera(float FOV = 90.0f, bool bAssignInput = false, bool bSetActive = false, std::string DebugName = "");

	// Creates a point light for use in game. The created PPointLight object is returned.
	static PPointLight* CreatePointLight(float Strength, float Radius, PMath::float4 Clr = {1.f, 1.f, 1.f, 1.f}, std::string DebugName = "");

	// Creates a Static Mesh for use in game. The created PStaticMesh object is returned.
	static PStaticMesh* CreateStaticMesh(const char* ModelFilePath, const char* DDSFilePath, std::string DebugName = "", PMath::float3 InScale = { 1.f, 1.f, 1.f });

	// Creates a Skeletal Mesh for use in game. The created PSkeletalMesh object is returned.
	static PSkeletalMesh* CreateSkeletalMesh(const char* ModelFilePath, const char* DDSFilePath, std::string DebugName = "", PMath::float3 InScale = { 1.f, 1.f, 1.f }, const char* SpecularFilePath = "", const char* EmissiveFilePath = "");

	// Destroys an object from the game world and cleans up its memory used. Returns true if the object was destroyed.
	static bool DestroyObject(PObject* Obj);


	// ----------------------------------------------------------------------------------------------->
	//		Below functions will interact with levels via saving and loading.
	// ----------------------------------------------------------------------------------------------->


	// Load a level into the gameplay environment. This will clear all loaded objects.
	static void LoadLevel(std::string FilePath);

	// Save all objects loaded into this environment to a file. If the file exists, it will 
	// be overwritten.
	static void SaveLevel(std::string FilePath);


	// ----------------------------------------------------------------------------------------------->
	//		Below functions will manipulate strings and their information.
	// ----------------------------------------------------------------------------------------------->


	// Split a string using a delimeter, then return all strings that were split in chunks.
	static std::vector<std::string> SplitString(const std::string& InString, const char& Delim);

	// Split a string using a delimeter, then return the last element from the post-split string.
	static std::string SplitStringGetLast(const std::string& InString, const char& Delim);

	// Replace an InString with Replace at Delim character.
	static std::string ReplaceString(const std::string& InString, const char& Delim, const char& Replace);

	// Split a filepath in two chunks at a specific folder/directory.
	static std::string SliceFilepathByDir(std::string Dir, std::string SplitAt);


	// ----------------------------------------------------------------------------------------------->
	//		Below functions will retrieve directory paths and information, of multiple types.
	// ----------------------------------------------------------------------------------------------->


	// Return the main Directory that leads to Polyn folder. Example output: "D:/User/Documents/Polyn/"
	static std::string GetMainDirectory();

	// Return the main Directory that leads to PolynGameEngine folder. Example output: "D:/User/Documents/Polyn/PolynGameEngine/"
	static std::string GetDevDirectory();

	// Return the main Directory that leads to PolynGame folder. Example output: "D:/User/Documents/Polyn/PolynGame/"
	static std::string GetGameDirectory();


	// ----------------------------------------------------------------------------------------------->
	//		Below functions will interact with the debug printing on the screen.
	// ----------------------------------------------------------------------------------------------->


	// Print some text to the output log. OutString is the string that will be printed and Status will decide the
	// color and type of the string to print.
	//
	// Status:
	//	0. Text
	//	1. Success
	//	2. Failure
	//	3. Warning
	//	4. System Command
	static void PrintToConsole(std::string OutString, int Status = 0, std::string Source = "GameplayStatics");
};

