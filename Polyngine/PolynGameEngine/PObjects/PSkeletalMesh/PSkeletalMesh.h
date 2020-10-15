#pragma once

#include "../../PObjects/PStaticMesh/PStaticMesh.h"

// A Skeletal Mesh is an object that has a 3D model attached to it. On creation, a model and texture must be supplied. The 3D model supplied should be a .mesh generated from a .FBX file.
class PSkeletalMesh :	public PStaticMesh
{
public:
	// ------------------------------------------------------------------
	//		Constructors.
	// ------------------------------------------------------------------

	// Constructor that sets no information. You will need to setup this model and manually call the begin functions.
	PSkeletalMesh();

	// Construct a PStaticMesh using a supplied list of Vertices and Indices.
	PSkeletalMesh(std::string DebugName, std::vector<Vertex> Verts, std::vector<int> Ind, std::string DDSFilePath, ID3D11Device* Dvc, bool bVisible = true, PObject* Parent = nullptr, float3 InScale = { 1.0f, 1.0f, 1.0f });

	// Construct a PStaticMesh using a supplied Mesh and Texture filepath.
	PSkeletalMesh(std::string DebugName, std::string ModelFilePath, std::string DDSFilePath, ID3D11Device* Dvc, ID3D11DeviceContext* DvcContext, bool bVisible = true, PObject* Parent = nullptr, float3 InScale = { 1.0f, 1.0f, 1.0f });

	// ------------------------------------------------------------------
	//		Begin, Update, and EndPlay for game state notification.
	// ------------------------------------------------------------------

	// Called when this object has been successfully loaded into the game world.
	void BeginPlay();
	
	// Called once per frame.
	void virtual Update(float DeltaTime);
	
	// Called just before this object is completely removed from the game world.
	void EndPlay();

	// Call this to manually remove this object from the game world.
	// NOTE: This is not guarenteed to be called before an object is removed unless you explicitly call it yourself.
	void Destroy();


	// ------------------------------------------------------------------
	//		Load Models, Texture, and Other Assets.
	// ------------------------------------------------------------------

	// Load a mesh into this object using Skeletal setups. Must be a .mesh file.
	bool LoadMesh(const char* MeshFileName, ID3D11Device* Dvc, ID3D11DeviceContext* Cntxt, bool RefreshBuffers = false);

	// Load a texture into this object.
	//
	// Texture Types:
	//	0 - Diffuse
	//	1 - Normal
	//	2 - Specular
	//	3 - Emissive
	bool LoadTexture(const char* DDSFilePath, ID3D11Device* Dvc, int TextureType = 0);

	// ------------------------------------------------------------------
	//		Interact with the Animation System.
	// ------------------------------------------------------------------

	// Play the supplied animation. If no filepath is supplied, it will attempt to play any loaded animation.
	bool PlayAnimation(const char* AnimFilePath = "");

	// Pause the currently loaded animation.
	bool PauseAnimation();
};

