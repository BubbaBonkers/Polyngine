#pragma once

#include "../PObject/PObject.h"
#include "../../Shaders/PMaterial/PMaterial.h"

// A static mesh is an object that has a 3D model attached to it. On creation, a model and texture must be supplied.
class PStaticMesh :	public PObject
{
public:
	// ------------------------------------------------------------------
	//		Rendering information.
	// ------------------------------------------------------------------
	std::vector<Vertex> Vertices;								// The Position and Color data for each Vertex stored as a list containing every Vertex.
	std::vector<int> Indices;									// A vector containing the Index information based on the Vertices list.
	ID3D11Buffer* VertexBuffer = nullptr;						// The DirectX vertex buffer for this object.
	ID3D11Buffer* IndexBuffer = nullptr;						// The DirectX index buffer for this object.
	ID3D11ShaderResourceView* D_ShaderResourceView = nullptr;	// The diffuse texture for this object to be used in the DirectX rendering pipeline.
	ID3D11ShaderResourceView* N_ShaderResourceView = nullptr;	// The normal texture for this object to be used in the DirectX rendering pipeline.
	ID3D11ShaderResourceView* S_ShaderResourceView = nullptr;	// The specular texture for this object to be used in the DirectX rendering pipeline.
	ID3D11ShaderResourceView* E_ShaderResourceView = nullptr;	// The emissive texture for this object to be used in the DirectX rendering pipeline.


	// ------------------------------------------------------------------
	//		Model & Texture Filepaths.
	// ------------------------------------------------------------------
	std::string ModelFile;										// The filepath for the model this object should load.
	std::string DDSFile;										// The filepath for the diffuse texture this object should use on the Model.
	std::string Normal_DDSFile;									// The filepath for the normal texture this object should use on the Model.
	std::string Specular_DDSFile;								// The filepath for the specular texture this object should use on the Model.
	std::string Emissive_DDSFile;								// The filepath for the emissive texture this object should use on the Model.


	// ------------------------------------------------------------------
	//		Material Information.
	// ------------------------------------------------------------------
	PMaterial Material;


	// ------------------------------------------------------------------
	//		General Object Information
	// ------------------------------------------------------------------
	unsigned int PrimitiveType = 0;								// The type of primitive of this object (ex. Cube, Plane, etc). 0 means not a primitive.


	// ------------------------------------------------------------------
	//		Collision Information.
	// ------------------------------------------------------------------

	// Should collision be enabled?
	bool Col_bEnableCollision = true;											// Should collision be enabled?

	float3 Col_BBOffset = { 0.0f, 2.0f, 0.0f };									// Offset of the bounding box from the model origin.
	PAABB Col_BoundingBox = { { 0.0f, 0.0f, 0.0f }, { 2.0f, 2.0f, 2.0f } };		// Size of the bounding box in the form float2().


	// ------------------------------------------------------------------
	//		Constructors.
	// ------------------------------------------------------------------

	// Constructor that sets no information. You will need to setup this model and manually call the begin functions.
	PStaticMesh();

	// Construct a PStaticMesh using a supplied list of Vertices and Indices.
	PStaticMesh(std::string DebugName, std::vector<Vertex> Verts, std::vector<int> Ind, std::string DDSFilePath, ID3D11Device* Dvc, bool bVisible = true, PObject* Parent = nullptr, float3 InScale = { 1.0f, 1.0f, 1.0f });

	// Construct a PStaticMesh using a supplied Model and Texture filepath.
	PStaticMesh(std::string DebugName, std::string ModelFilePath, std::string DDSFilePath, ID3D11Device* Dvc, ID3D11DeviceContext* DvcContext, bool bVisible = true, PObject* Parent = nullptr, float3 InScale = { 1.0f, 1.0f, 1.0f });


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

	// Load a mesh into this object.
	bool LoadMesh(const char* MeshFileName, ID3D11Device* Dvc, ID3D11DeviceContext* Cntxt, bool RefreshBuffers = false);

	// Load a texture into this object.
	//
	// Texture Types:
	//	0 - Diffuse
	//	1 - Normal
	//	2 - Specular
	//	3 - Emissive
	bool LoadTexture(const char* DDSFilePath, ID3D11Device* Dvc, int TextureType = 0);

	// Set the current material being used.
	void SetMaterial(PMaterial Mat);


	// ------------------------------------------------------------------
	//		Handle Collision Data
	// ------------------------------------------------------------------

	// Set the extents of the Bounding Box collider for this object.
	void SetBoundingBoxExtents(float3 Ext);

	// Set the extents of the Bounding Box collider for this object.
	void SetBoundingBoxOffset(float3 Off);

	// Return the extents of the Bounding Box collider for this object.
	float3 GetBoundingBoxExtents();

	// Return the extents of the Bounding Box collider for this object.
	float3 GetBoundingBoxOffset();


	// ------------------------------------------------------------------
	//		Update Vertex/Index Buffer Information.
	// ------------------------------------------------------------------

	// Refresh buffer data for the model and texture.
	void RefreshVertexIndexBuffers(ID3D11Device* Dvc, ID3D11DeviceContext* Cntxt, bool Ver = true, bool Ind = true);
};

