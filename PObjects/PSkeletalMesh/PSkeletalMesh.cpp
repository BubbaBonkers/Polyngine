#include "PSkeletalMesh.h"
#include "../../PSystem/DDSTextureLoader/DDSTextureLoader.h"
#include <fstream>

template<typename T>
void safe_release(T* t)
{
	if (t)
		t->Release();
}

PSkeletalMesh::PSkeletalMesh()
{
	
}

PSkeletalMesh::PSkeletalMesh(std::string DebugName, std::vector<Vertex> Verts, std::vector<int> Ind, std::string DDSFilePath, ID3D11Device* Dvc, bool bVisible, PObject* Parent, float3 InScale)
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

PSkeletalMesh::PSkeletalMesh(std::string DebugName, std::string MeshFilePath, std::string DDSFilePath, ID3D11Device* Dvc, ID3D11DeviceContext* Cntxt, bool bVisible, PObject* Parent, float3 InScale)
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
	std::string MFP = MeshFilePath;
	LoadMesh(MeshFilePath.c_str(), Dvc, Cntxt);

	// Call BeginPlay() to signal that setup is complete.
	BeginPlay();
}

void PSkeletalMesh::BeginPlay()
{
	PStaticMesh::BeginPlay();
}

void PSkeletalMesh::Update(float DeltaTime)
{
	PStaticMesh::Update(DeltaTime);
}

void PSkeletalMesh::EndPlay()
{
	PStaticMesh::EndPlay();
}

// Destroy will call EndPlay() and ensure cleanup.
void PSkeletalMesh::Destroy()
{
	PStaticMesh::Destroy();

	EndPlay();
}

bool PSkeletalMesh::LoadMesh(const char* MeshFileName, ID3D11Device* Dvc, ID3D11DeviceContext* Cntxt, bool RefreshBuffers)
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
	if (FileType == ".mesh")
	{
		if (true)
		{
			// Open the file (mesh) in binary input mode.
			std::fstream file{ (PGameplayStatics::GetGameDirectory() + "Assets/" + MeshFileName), std::ios_base::in | std::ios_base::binary };

			// Ensure the file opened correctly.
			assert(file.is_open());

			// Setup index count for the mesh.
			uint32_t Item_Index_Count;
			// Read in the nuber of Indices for the mesh.
			file.read((char*)&Item_Index_Count, sizeof(uint32_t));

			// Resize the index list to hold the exact number of indices that the mesh has for easy copying.
			Indices.resize(Item_Index_Count);

			// Read in the indices to the list we made.
			file.read((char*)Indices.data(), sizeof(uint32_t) * Item_Index_Count);

			// Setup the count of vertices in the mesh.
			int Item_Vertex_Count;
			// Read up to the values of the vertices.
			file.read((char*)&Item_Vertex_Count, sizeof(uint32_t));

			// Resize the Vertex holding list to have easy copying later.
			Vertices.resize(Item_Vertex_Count);

			// Read in the vertex data to our list.
			file.read((char*)Vertices.data(), sizeof(Vertex) * Item_Vertex_Count);

			// Example mesh conditioning if needed, to fix up the flip.
			for (auto& v : Vertices)
			{
				v.Position.x = -v.Position.x;
				v.Normal.x = -v.Normal.x;
				v.Texture.y = 1.0f - v.Texture.y;
			}

			ModelFile = MeshFileName;

			// Setup a count for triangles.
			int Tri_Count = (int)(Indices.size() / 3);

			// Flip information to be the proper hand system.
			for (int i = 0; i < Tri_Count; ++i)
			{
				int* Tri = Indices.data() + i * 3;

				int Temp = Tri[0];
				Tri[0] = Tri[2];
				Tri[2] = Temp;
			}

			// Close the file.
			file.close();
		}
		else
		{
			// Object could not be loaded.
			return false;
		}
	}
	else
	{
		return false;
	}

	RefreshVertexIndexBuffers(Dvc, Cntxt);

	return true;
}

// Play an animation on this skeletal mesh. If this animation is not loaded, it will load the animation before playing it.
bool PSkeletalMesh::PlayAnimation(const char* AnimFilePath)
{
	if (!Animator.Play(AnimFilePath))
	{
		return false;
	}

	return true;
}

bool PSkeletalMesh::PauseAnimation()
{
	if (!Animator.Pause())
	{
		return false;
	}

	return true;
}

// Load a texture into this object.
//
// Texture Types:
//	0 - Diffuse
//	1 - Normal
//	2 - Specular
//	3 - Emissive
bool PSkeletalMesh::LoadTexture(const char* DDSFilePath, ID3D11Device* Dvc, int TextureType)
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