#pragma once

#include <directxmath.h>
#include <directxcolors.h>
#include <fbxsdk.h>
#include <vector>
#include <fstream>
#include <array>

#include "../../PAnimation/PAnim/PAnim.h"

// Funtime random normal!
#define RAND_NORMAL XMFLOAT3(rand()/float(RAND_MAX), rand()/float(RAND_MAX), rand()/float(RAND_MAX))
#define Epsilon 0.005f

class FBXExporter
{
	using file_path_t = std::array<char, 400>;

public:
	//--------------------------------------------------------------------------------------
	// Structures
	//--------------------------------------------------------------------------------------

	// This struct is used to hold information about a vertex.
	struct SimpleVertex
	{
		DirectX::XMFLOAT3	Pos;
		DirectX::XMFLOAT3	Normal;
		DirectX::XMFLOAT2	Tex;
		float4				Weights		= { 0.0f, 0.0f, 0.0f };
		int					Joints[4]	= { -1, -1, -1, -1 };

		SimpleVertex() {}

		SimpleVertex& operator=(SimpleVertex& Vert)
		{
			Pos = Vert.Pos;
			Normal = Vert.Normal;
			Tex = Vert.Tex;
			Weights = Vert.Weights;
			Joints[0] = Vert.Joints[0];
			Joints[1] = Vert.Joints[1];
			Joints[2] = Vert.Joints[2];
			Joints[3] = Vert.Joints[3];

			return *this;
		}
	};

	// This holds some basic information about an FBX mesh including vertices and indices.
	struct SimpleMesh
	{
		std::vector<SimpleVertex> VertexList;
		std::vector<int> IndicesList;
	};

	// A joint not containing the transform.
	struct FBX_Joint
	{
		FbxNode* Node;
		int ParentIndex;
	};

	// A bind pose holds the Joint information for a bind pose.
	struct PBindPose
	{
		std::vector<PAnim::Joint> Joints;
		std::vector<FbxNode*> Nodes;

		PBindPose() {}

		PBindPose(std::vector<PAnim::Joint> InJoints, std::vector<FbxNode*> InNodes)
		{
			Joints = InJoints;
			Nodes = InNodes;
		}
	};

	struct Influence
	{
		int		Joint;
		float	Weight;
	};

	// This struct is used to hold an FBX Material from a .FBX model file.
	struct Material_FBX
	{
		// The layer is the part of the material you are accessing.
		enum E_Layer
		{
			EMISSIVE = 0,		// How much light is emitted from the object.
			DIFFUSE = 1,		// The normal color of the object's texture.
			SPECULAR = 2,		// How much lights affect this object.
			ROUGHNESS = 3,		// How smooth or rough the surface of the object is.
			COUNT = 4			// This is to keep the count of how many layers there can be.
		};

		// This is the setup for a single layer.
		struct Layer
		{
			float	Value[3] = { 0.0f, 0.0f, 0.0f };
			float	Factor = 0.0f;
			int64_t Input = -1;
		};

		Layer& operator[](int i)
		{
			return Layers[i];
		}

		const Layer& operator[](int i) const
		{
			return Layers[i];
		}

	private:
		Layer Layers[E_Layer::COUNT];
	};

	enum CONSTYPE
	{
		MESH = 0,
		ANIM = 1
	};

	using InfluenceSet = std::array<Influence, 4>;

public:
	// This constructor is here to allow you to simply call it to use FBXtoMesh or FBXtoAnim instead of
	// making a new instance of this class to use it.
	FBXExporter(CONSTYPE Type, const char* FbxFilePath, const char* MeshFilePath);

	// This will take in a filepath for a .FBX file and convert it to a .MESH file before importing
	// it into the project directory.
	void FBXtoMesh(const char* FbxFileName, const char* MeshFileName);
	void FBXtoAnim(const char* FbxFileName, const char* AnimFileName);

	// Global SimpleMesh.
	SimpleMesh simpleMesh;
	std::vector<Material_FBX> Materials;
	std::vector<file_path_t> Paths;
	std::vector<InfluenceSet> InfluenceCtrlPnts;

	// For file manipulation.
	FILE* fbx_conout;
	FILE* fbx_conerr;

	// FBX Scene container.
	FbxScene* lScene;

	std::vector<FbxPose*> Poses;
	std::vector<PBindPose> BindPoses;
	std::vector<FBX_Joint> FbxJoints;
	std::vector<FbxSkeleton*> Skeletons;
	std::vector<FbxMesh*> FbxMeshes;
	std::vector<FbxSkin*> FbxSkins;
	std::vector<FbxCluster*> FbxClusters;
	std::vector<PAnim::AnimClip> AnimClips;

	// Helpers for FBX initialization.
	void FBX_CreateScene(const char* FileName);

	// Functions for exporting a .MESH file.
	void SaveMesh(const char* MeshFileName, SimpleMesh& Mesh);
	void ProcessFbxMesh(FbxNode* Node, bool bOnlyLoad = false);
	std::vector<DirectX::XMFLOAT2> LoadUVInformation(FbxMesh* pMesh);
	void Unindex(FbxMesh* Mesh, std::vector<DirectX::XMFLOAT2> UVInformation);
	void Compactify();

	// Functions for exporting a material from an FBX.
	void SaveMaterial(FbxScene* InScene);

	// Functions for exporting an animation.
	void ReadBindPoses();
	void RecursiveBindJointRead(PBindPose* Pose, FbxNode* Node, int Index, int ParentIndex);
	void ReadJointAnimations();
	void RecursiveJointRead(FbxNode* Node, int Index, int ParentIndex);
	void ReadWeightData();
	void SaveAnimation(const char* AnimName);
};
