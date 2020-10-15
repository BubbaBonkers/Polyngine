#include "PStaticMesh.h"
#include "../../PSystem/PObjLoader/OBJ_Loader.h"
#include "../../PSystem/DDSTextureLoader/DDSTextureLoader.h"

template<typename T>
void safe_release(T* t)
{
	if (t)
		t->Release();
}

PStaticMesh::PStaticMesh()
{
	
}

PStaticMesh::PStaticMesh(std::string DebugName, std::vector<Vertex> Verts, std::vector<int> Ind, std::string DDSFilePath, ID3D11Device* Dvc, bool bVisible, PObject* Parent, float3 InScale)
{
	DisplayName = DebugName;			// Choose a display name for debug printing.
	Ctrl_bIsVisible = bVisible;			// Set the initial visibility.
	ModelFile = "Primitive";
	DDSFile = DDSFilePath;

	// World and Local Matrices.
	DefaultWorld.ViewMatrix = (float4x4_a&)DirectX::XMMatrixIdentity();
	LocalMatrix = DirectX::XMMatrixIdentity();

	if (Parent)
	{
		AttachToObject(Parent);
	}

	ScaleObject(float3{ InScale });
	ScaleObjectLocally(float3{ InScale });

	// Load the texture.
	if (DDSFilePath != "")
	{
		LoadTexture(DDSFilePath.c_str(), Dvc);
	}

	// Load the primitive.
	Indices = Ind;
	Vertices = Verts;

	D3D11_BUFFER_DESC BufferDesc;
	D3D11_SUBRESOURCE_DATA SubData;
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));
	ZeroMemory(&SubData, sizeof(SubData));

	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.ByteWidth = sizeof(Vertex) * Vertices.size();
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = 0;
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	SubData.pSysMem = Vertices.data();

	// Vertex Buffer.
	HRESULT hr = Dvc->CreateBuffer(&BufferDesc, &SubData, &VertexBuffer);

	BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	BufferDesc.ByteWidth = sizeof(int) * Indices.size();
	SubData.pSysMem = Indices.data();

	hr = Dvc->CreateBuffer(&BufferDesc, &SubData, &IndexBuffer);

	// Call BeginPlay() to signal that setup is complete.
	BeginPlay();
}

PStaticMesh::PStaticMesh(std::string DebugName, std::string ModelFilePath, std::string DDSFilePath, ID3D11Device* Dvc, ID3D11DeviceContext* Cntxt, bool bVisible, PObject* Parent, float3 InScale)
{
	DisplayName = DebugName;			// Choose a display name for debug printing.
	Ctrl_bIsVisible = bVisible;			// Set the initial visibility.

	// World and Local Matrices.
	DefaultWorld.ViewMatrix = (float4x4_a&)DirectX::XMMatrixIdentity();
	LocalMatrix = DirectX::XMMatrixIdentity();

	if (Parent)
	{
		AttachToObject(Parent);
	}

	ScaleObject(float3{ InScale });
	ScaleObjectLocally(float3{ InScale });

	// Load the texture data into the object members.
	LoadTexture(DDSFilePath.c_str(), Dvc);

	// Load the mesh data into the object members.
	std::string MFP = ModelFilePath;
	LoadMesh(ModelFilePath.c_str(), Dvc, Cntxt);

	// Call BeginPlay() to signal that setup is complete.
	BeginPlay();
}

void PStaticMesh::BeginPlay()
{
	PObject::BeginPlay();
}

void PStaticMesh::Update(float DeltaTime)
{
	// Handle input.
	if (Ctrl_bEnableInput && (Controller))
	{
		if (Controller->IsInputDown(PInputManager::PInputMap::PIN_ARROW_UP))
		{
			AddMovementInput(float3{ 0, 0, (Ctrl_MovementSpeed * DeltaTime) });
		}
		else if (Controller->IsInputDown(PInputManager::PInputMap::PIN_ARROW_DOWN))
		{
			AddMovementInput(float3{ 0, 0, -(Ctrl_MovementSpeed * DeltaTime) });
		}

		if (Controller->IsInputDown(PInputManager::PInputMap::PIN_ARROW_L))
		{
			AddRotationInput(float3{ 0, (Ctrl_RotationRate * -DeltaTime), 0 });
		}
		else if (Controller->IsInputDown(PInputManager::PInputMap::PIN_ARROW_R))
		{
			AddRotationInput(float3{ 0, (Ctrl_RotationRate * DeltaTime), 0 });
		}
	}

	PObject::Update(DeltaTime);
}

void PStaticMesh::EndPlay()
{
	safe_release(VertexBuffer);
	safe_release(IndexBuffer);
	safe_release(D_ShaderResourceView);
	safe_release(N_ShaderResourceView);
	safe_release(S_ShaderResourceView);
	safe_release(E_ShaderResourceView);

	PObject::EndPlay();
}

// Destroy will call EndPlay() and ensure cleanup.
void PStaticMesh::Destroy()
{
	PObject::Destroy();

	EndPlay();
}

bool PStaticMesh::LoadMesh(const char* MeshFileName, ID3D11Device* Dvc, ID3D11DeviceContext* Cntxt, bool RefreshBuffers)
{
	// Container for the file type.
	std::string FileType = "";

	// Get parse through the FileType string.
	for (unsigned int i = 0; i < strlen(MeshFileName); ++i)
	{
		if (MeshFileName[i] == '.')
		{
			FileType = ".";
		}
		else if (FileType[0] == '.')
		{
			FileType = FileType + MeshFileName[i];
		}
	}

	// Load .obj file type. Ensure it is obj. If it is FBX it should be created as a SkeletalMesh instead of this StaticMesh.
	if (FileType == ".obj")
	{
		objl::Loader ObjLoader;

		if (ObjLoader.LoadFile(PGameplayStatics::GetGameDirectory() + "Assets/" + MeshFileName))
		{
			Indices = (std::vector<int>&)ObjLoader.LoadedIndices;

			// Resize the Vertices list.
			Vertices.resize(ObjLoader.LoadedVertices.size());

			// Add the vertices.
			for (unsigned int i = 0; i < ObjLoader.LoadedVertices.size(); ++i)
			{
				Vertices[i].Position = { ObjLoader.LoadedVertices[i].Position.X, ObjLoader.LoadedVertices[i].Position.Y, ObjLoader.LoadedVertices[i].Position.Z };
				Vertices[i].Normal = { ObjLoader.LoadedVertices[i].Normal.X, ObjLoader.LoadedVertices[i].Normal.Y, ObjLoader.LoadedVertices[i].Normal.Z };
				Vertices[i].Texture = { ObjLoader.LoadedVertices[i].TextureCoordinate.X, ObjLoader.LoadedVertices[i].TextureCoordinate.Y };
			}

			// Condition mesh to fix UV and Normal flip.
			for (auto& v : Vertices)
			{
				//v.Position.x = -v.Position.x;
				//v.Normal.x = -v.Normal.x;
				v.Texture.y = 1.0f - v.Texture.y;
			}

			ModelFile = MeshFileName;

			if (PrimitiveType != 0)
			{
				PrimitiveType = 0;
			}
		}
		else
		{
			// Object could not be loaded.
			return false;
		}
	}

	RefreshVertexIndexBuffers(Dvc, Cntxt);

	return true;
}

// Load a texture into this object.
//
// Texture Types:
//	0 - Diffuse
//	1 - Normal
//	2 - Specular
//	3 - Emissive
bool PStaticMesh::LoadTexture(const char* DDSFilePath, ID3D11Device* Dvc, int TextureType)
{
	// Load the texture.
	std::string DDSText = DDSFilePath;
	std::string CharPtr = PGameplayStatics::GetGameDirectory() + "Assets/" + DDSText;
	std::wstring FullPath(CharPtr.length(), L' ');

	// Copy string to wstring.
	std::copy(CharPtr.begin(), CharPtr.end(), FullPath.begin());
	const wchar_t* WChar = FullPath.c_str();

	// Release the shader resource view if there is already one.
	if ((TextureType == 0) && D_ShaderResourceView)
	{
		D_ShaderResourceView->Release();
		D_ShaderResourceView = nullptr;
	}
	else if ((TextureType == 1) && N_ShaderResourceView)
	{
		N_ShaderResourceView->Release();
		N_ShaderResourceView = nullptr;
	}
	else if ((TextureType == 2) && S_ShaderResourceView)
	{
		S_ShaderResourceView->Release();
		S_ShaderResourceView = nullptr;
	}
	else if ((TextureType == 3) && E_ShaderResourceView)
	{
		E_ShaderResourceView->Release();
		E_ShaderResourceView = nullptr;
	}

	// Create the texture and set it to the ShaderResourceView for the graphics card.
	HRESULT hr;
	if (TextureType == 0)
	{
		hr = CreateDDSTextureFromFile(Dvc, WChar, nullptr, &D_ShaderResourceView);
	}
	else if (TextureType == 1)
	{
		hr = CreateDDSTextureFromFile(Dvc, WChar, nullptr, &N_ShaderResourceView);
	}
	else if (TextureType == 2)
	{
		hr = CreateDDSTextureFromFile(Dvc, WChar, nullptr, &S_ShaderResourceView);
	}
	else if (TextureType == 3)
	{
		hr = CreateDDSTextureFromFile(Dvc, WChar, nullptr, &E_ShaderResourceView);
	}

	if (SUCCEEDED(hr))
	{
		if (TextureType == 0)
		{
			DDSFile = DDSFilePath;
		}
		else if (TextureType == 1)
		{
			Normal_DDSFile = DDSFilePath;
		}
		else if (TextureType == 2)
		{
			Specular_DDSFile = DDSFilePath;
		}
		else if (TextureType == 3)
		{
			Emissive_DDSFile = DDSFilePath;
		}
	}
	else
	{
		// Texture could not be created.
		return false;
	}

	return true;
}

// Set the current material being used.
void PStaticMesh::SetMaterial(PMaterial Mat)
{
	Material = Mat;
}

void PStaticMesh::SetBoundingBoxExtents(float3 Ext)
{
	Col_BoundingBox.Extents = Ext;
}

// Set the extents of the Bounding Box collider for this object.
void PStaticMesh::SetBoundingBoxOffset(float3 Off)
{
	Col_BBOffset = Off;
}

float3 PStaticMesh::GetBoundingBoxExtents()
{
	return Col_BoundingBox.Extents;
}

// Return the extents of the Bounding Box collider for this object.
float3 PStaticMesh::GetBoundingBoxOffset()
{
	return Col_BBOffset;
}

void PStaticMesh::RefreshVertexIndexBuffers(ID3D11Device* Dvc, ID3D11DeviceContext* Cntxt, bool Ver, bool Ind)
{
	if (VertexBuffer)
	{
		VertexBuffer->Release();
		VertexBuffer = nullptr;
	}
	if (IndexBuffer)
	{
		IndexBuffer->Release();
		IndexBuffer = nullptr;
	}

	D3D11_BUFFER_DESC BufferDesc;
	D3D11_SUBRESOURCE_DATA SubData;
	ZeroMemory(&BufferDesc, sizeof(BufferDesc));
	ZeroMemory(&SubData, sizeof(SubData));

	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.ByteWidth = sizeof(Vertex) * Vertices.size();
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.MiscFlags = 0;
	BufferDesc.StructureByteStride = 0;
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	SubData.pSysMem = Vertices.data();

	// Vertex Buffer.
	HRESULT hr = Dvc->CreateBuffer(&BufferDesc, &SubData, &VertexBuffer);

	BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	BufferDesc.ByteWidth = sizeof(int) * Indices.size();
	SubData.pSysMem = Indices.data();

	hr = Dvc->CreateBuffer(&BufferDesc, &SubData, &IndexBuffer);
}

