#include "FBXExporter.h"
#include <assert.h>
#include <stdio.h>

using namespace std;

// This constructor is here to allow you to simply call it to use FBXtoMesh instead of
// making a new instance of this class to use it.
FBXExporter::FBXExporter(CONSTYPE Type, const char* FbxFilePath, const char* DestFilePath)
{
	if ((FbxFilePath != "") && (DestFilePath != ""))
	{
		switch(Type)
		{
			case 0:
			{
				FBXtoMesh(FbxFilePath, DestFilePath);
				break;
			}
			case 1:
			{
				FBXtoAnim(FbxFilePath, DestFilePath);
				break;
			}
		}
	}
}


//--------------------------------------------------------------------------------------
// Create an FBX scene to use for the exporter. Generic FBX scene,
//--------------------------------------------------------------------------------------
void FBXExporter::FBX_CreateScene(const char* FileName)
{
	// Change the following filename to a suitable filename value.
	const char* lFilename = FileName;

	// Initialize the SDK manager. This object handles all our memory management.
	FbxManager* lSdkManager = FbxManager::Create();

	// Create the IO settings object.
	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	// Create an importer using the SDK manager.
	FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

	// Use the first argument as the filename for the importer.
	if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings())) {
		exit(-1);
	}

	// Create a new scene so that it can be populated by the imported file.
	lScene = FbxScene::Create(lSdkManager, "myScene");

	// Import the contents of the file into the scene.
	lImporter->Import(lScene);

	// The file is imported; so get rid of the importer.
	lImporter->Destroy();
}


//--------------------------------------------------------------------------------------
// Quick actions for importing different FBX information.
//--------------------------------------------------------------------------------------

// This will convert an FBX model into a .MESH equivilent using all of the helper functions.
void FBXExporter::FBXtoMesh(const char* FbxFileName, const char* MeshFileName)
{
	// Create the FBX scene.
	FBX_CreateScene(FbxFileName);

	FbxNode* ChildNode = lScene->GetRootNode()->GetChild(0);
	FbxMesh* Mesh = ChildNode->GetMesh();

	ProcessFbxMesh(lScene->GetRootNode());

	SaveMesh(MeshFileName, simpleMesh);
}

void FBXExporter::FBXtoAnim(const char* FbxFileName, const char* AnimFileName)
{
	// Create the FBX scene.
	FBX_CreateScene(FbxFileName);

	// ------------------------------------------------------------
	// Fill PAnim::AnimClip Vector with Animations.
	// ------------------------------------------------------------
	ReadJointAnimations();
	
	// ------------------------------------------------------------
	// Read in a Bind Pose from an FBX File.
	// ------------------------------------------------------------
	ReadBindPoses();

	// ------------------------------------------------------------
	// Write out the Animation Data to a new .ANIM File.
	// ------------------------------------------------------------
	SaveAnimation(AnimFileName);
}


//--------------------------------------------------------------------------------------
// Get mesh information.
//--------------------------------------------------------------------------------------
void FBXExporter::ProcessFbxMesh(FbxNode* Node, bool bOnlyLoad)
{
	// FBX Mesh Stuff.
	int ChildrenCount = Node->GetChildCount();

	for (int i = 0; i < ChildrenCount; ++i)
	{
		FbxNode* ChildNode = Node->GetChild(i);
		FbxMesh* Mesh = ChildNode->GetMesh();

		if (Mesh != NULL)
		{
			// Get index count from each.
			int NumVertices = Mesh->GetControlPointsCount();
			InfluenceCtrlPnts.resize(NumVertices);

			// Resize the vertex vector to the size of this mesh.
			simpleMesh.VertexList.resize(NumVertices);

			for (int j = 0; j < NumVertices; ++j)
			{
				FbxVector4 Vert = Mesh->GetControlPointAt(j);
				simpleMesh.VertexList[j].Pos.x = (float)Vert.mData[0];
				simpleMesh.VertexList[j].Pos.y = (float)Vert.mData[1];
				simpleMesh.VertexList[j].Pos.z = (float)Vert.mData[2];
			}

			int NumIndices = Mesh->GetPolygonVertexCount();

			// No need to allocate int array, FBX will do that for me.
			int* Indices = Mesh->GetPolygonVertices();

			// Fill IndiceList.
			simpleMesh.IndicesList.resize(NumIndices);
			memcpy(simpleMesh.IndicesList.data(), Indices, NumIndices * sizeof(int));

			ReadJointAnimations();
			ReadBindPoses();
			ReadWeightData();

			if (!bOnlyLoad)
			{
				Unindex(Mesh, LoadUVInformation(Mesh));
			}
		}
	}
}

void FBXExporter::SaveMesh(const char* MeshFileName, SimpleMesh& Mesh)
{
	// Open a file using binary truncation.
	ofstream file(MeshFileName, ios::trunc | ios::binary | ios::out);

	// Ensure the file was opened properly.
	assert(file.is_open());

	// Setup holders for the count of indices and vertices.
	uint32_t index_count = (uint32_t)Mesh.IndicesList.size();
	uint32_t vert_count = (uint32_t)Mesh.VertexList.size();

	// Write all appropriate data to the new file and overwrite the old data.
	file.write((const char*)&index_count, sizeof(uint32_t));
	file.write((const char*)Mesh.IndicesList.data(), sizeof(uint32_t) * index_count);
	file.write((const char*)&vert_count, sizeof(uint32_t));
	file.write((const char*)Mesh.VertexList.data(), sizeof(SimpleVertex) * vert_count);

	// Close the file.
	file.close();
}


//--------------------------------------------------------------------------------------
// Load in the UV information.
//--------------------------------------------------------------------------------------
std::vector<DirectX::XMFLOAT2> FBXExporter::LoadUVInformation(FbxMesh* pMesh)
{
	// simpleMesh.VertexList[0].Tex
	// Vector to hold all UV values as it loops through.
	vector<DirectX::XMFLOAT2> UVInformation;

	//get all UV set names
	FbxStringList lUVSetNameList;
	pMesh->GetUVSetNames(lUVSetNameList);

	//iterating over all uv sets
	for (int lUVSetIndex = 0; lUVSetIndex < lUVSetNameList.GetCount(); lUVSetIndex++)
	{
		//get lUVSetIndex-th uv set
		const char* lUVSetName = lUVSetNameList.GetStringAt(lUVSetIndex);
		const FbxGeometryElementUV* lUVElement = pMesh->GetElementUV(lUVSetName);

		if (!lUVElement)
			continue;

		// only support mapping mode eByPolygonVertex and eByControlPoint
		if (lUVElement->GetMappingMode() != FbxGeometryElement::eByPolygonVertex &&
			lUVElement->GetMappingMode() != FbxGeometryElement::eByControlPoint)
			return vector<DirectX::XMFLOAT2>{};

		//index array, where holds the index referenced to the uv data
		const bool lUseIndex = lUVElement->GetReferenceMode() != FbxGeometryElement::eDirect;
		const int lIndexCount = (lUseIndex) ? lUVElement->GetIndexArray().GetCount() : 0;

		//iterating through the data by polygon
		const int lPolyCount = pMesh->GetPolygonCount();

		if (lUVElement->GetMappingMode() == FbxGeometryElement::eByControlPoint)
		{
			for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
				for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
				{
					FbxVector2 lUVValue;

					//get the index of the current vertex in control points array
					int lPolyVertIndex = pMesh->GetPolygonVertex(lPolyIndex, lVertIndex);

					//the UV index depends on the reference mode
					int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyVertIndex) : lPolyVertIndex;

					lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

					// Add the UV to the UVInformation vector.
					UVInformation.push_back(DirectX::XMFLOAT2(lUVValue.mData[0], lUVValue.mData[1]));
				}
			}
		}
		else if (lUVElement->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
		{
			int lPolyIndexCounter = 0;
			for (int lPolyIndex = 0; lPolyIndex < lPolyCount; ++lPolyIndex)
			{
				// build the max index array that we need to pass into MakePoly
				const int lPolySize = pMesh->GetPolygonSize(lPolyIndex);
				for (int lVertIndex = 0; lVertIndex < lPolySize; ++lVertIndex)
				{
					if (lPolyIndexCounter < lIndexCount)
					{
						FbxVector2 lUVValue;

						//the UV index depends on the reference mode
						int lUVIndex = lUseIndex ? lUVElement->GetIndexArray().GetAt(lPolyIndexCounter) : lPolyIndexCounter;

						lUVValue = lUVElement->GetDirectArray().GetAt(lUVIndex);

						// Add the UV to the UVInformation vector.
						UVInformation.push_back(DirectX::XMFLOAT2(lUVValue.mData[0], lUVValue.mData[1]));

						lPolyIndexCounter++;
					}
				}
			}
		}
	}

	return UVInformation;
}


//--------------------------------------------------------------------------------------
// Unindex for normal and location data.
//--------------------------------------------------------------------------------------
void FBXExporter::Unindex(FbxMesh* Mesh, std::vector<DirectX::XMFLOAT2> UVInformation)
{
	// Get the number of indices.
	int NumIndices = Mesh->GetPolygonVertexCount();

	// Get the normals array from the FBX mesh.
	FbxArray<FbxVector4> NormalsVector;
	Mesh->GetPolygonVertexNormals(NormalsVector);

	// Declare a new array for the second vertex array. Working with NumIndices for size.
	vector<SimpleVertex> VertexListExpanded;
	VertexListExpanded.resize(NumIndices);

	// Align, or expand, vertext array and set the normals.
	for (int i = 0; i < NumIndices; ++i)
	{
		VertexListExpanded[i].Pos = simpleMesh.VertexList[simpleMesh.IndicesList[i]].Pos;

		VertexListExpanded[i].Normal.x = NormalsVector[i].mData[0];
		VertexListExpanded[i].Normal.y = NormalsVector[i].mData[1];
		VertexListExpanded[i].Normal.z = NormalsVector[i].mData[2];

		// Put UV setting here.
		VertexListExpanded[i].Tex.x = UVInformation[i].x;
		VertexListExpanded[i].Tex.y = UVInformation[i].y;

		VertexListExpanded[i].Weights = simpleMesh.VertexList[i].Weights;
		VertexListExpanded[i].Joints[0] = simpleMesh.VertexList[i].Joints[0];
		VertexListExpanded[i].Joints[1] = simpleMesh.VertexList[i].Joints[1];
		VertexListExpanded[i].Joints[2] = simpleMesh.VertexList[i].Joints[2];
		VertexListExpanded[i].Joints[3] = simpleMesh.VertexList[i].Joints[3];
	}

	// Make new indices to match the new vertex(2) array.
	vector<int> IndicesList;
	IndicesList.resize(NumIndices);
	for (int i = 0; i < NumIndices; ++i)
	{
		IndicesList[i] = i;
	}

	// Copy the working data into the global SimpleMesh.
	simpleMesh.IndicesList.assign(IndicesList.begin(), IndicesList.end());
	simpleMesh.VertexList.assign(VertexListExpanded.begin(), VertexListExpanded.end());

	Compactify();
}


//--------------------------------------------------------------------------------------
// Compactify for use in game.
//--------------------------------------------------------------------------------------
void FBXExporter::Compactify()
{
	vector<SimpleVertex> CompactVertices;
	vector<int> CompactIndices;

	for (int i = 0; i < simpleMesh.IndicesList.size(); ++i)
	{
		bool MatchFound = false;

		for (int j = 0; j < CompactVertices.size(); ++j)
		{
			if (abs(simpleMesh.VertexList[i].Pos.x - CompactVertices[j].Pos.x) <= Epsilon &&
				abs(simpleMesh.VertexList[i].Pos.y - CompactVertices[j].Pos.y) <= Epsilon &&
				abs(simpleMesh.VertexList[i].Pos.z - CompactVertices[j].Pos.z) <= Epsilon &&
				abs(simpleMesh.VertexList[i].Normal.x - CompactVertices[j].Normal.x) <= Epsilon &&
				abs(simpleMesh.VertexList[i].Normal.y - CompactVertices[j].Normal.y) <= Epsilon &&
				abs(simpleMesh.VertexList[i].Normal.z - CompactVertices[j].Normal.z) <= Epsilon &&
				abs(simpleMesh.VertexList[i].Tex.x - CompactVertices[j].Tex.x) <= Epsilon &&
				abs(simpleMesh.VertexList[i].Tex.y - CompactVertices[j].Tex.y) <= Epsilon)
			{
				MatchFound = true;
				CompactIndices.push_back(j);
				break;
			}
		}

		if (!MatchFound)
		{
			CompactVertices.push_back((SimpleVertex&)simpleMesh.VertexList[i]);
			CompactIndices.push_back(CompactVertices.size() - 1);
		}
	}

	// Copy the working data into the global SimpleMesh.
	simpleMesh.IndicesList.assign(CompactIndices.begin(), CompactIndices.end());
	simpleMesh.VertexList.assign(CompactVertices.begin(), CompactVertices.end());
}


//--------------------------------------------------------------------------------------
// Material information extraction.
//--------------------------------------------------------------------------------------
void FBXExporter::SaveMaterial(FbxScene* InScene)
{
	FbxScene* Scene = InScene;

	int NumMats = Scene->GetMaterialCount();

	for (unsigned int i = 0; i < NumMats; ++i)
	{
		Material_FBX Mat;
		FbxSurfaceMaterial* SurfaceMat = Scene->GetMaterial(i);

		if (!SurfaceMat->Is<FbxSurfaceLambert>())
		{
			continue;
		}

		// ------------------------------------------------------------
		// Diffuse Color
		// ------------------------------------------------------------

		FbxSurfaceLambert* Lam = (FbxSurfaceLambert*)SurfaceMat;
		FbxDouble3 Diffuse = Lam->Diffuse.Get();
		FbxDouble DiffuseFactor = Lam->DiffuseFactor.Get();

		Mat[Material_FBX::DIFFUSE].Value[0] = Diffuse[0];
		Mat[Material_FBX::DIFFUSE].Value[1] = Diffuse[1];
		Mat[Material_FBX::DIFFUSE].Value[2] = Diffuse[2];
		Mat[Material_FBX::DIFFUSE].Factor = DiffuseFactor;

		FbxFileTexture* FileTex = Lam->Diffuse.GetSrcObject<FbxFileTexture>();

		if (FileTex)
		{
			const char* FileName = FileTex->GetRelativeFileName();
			file_path_t FilePath;
			strcpy(FilePath.data(), FileName);
			Mat[Material_FBX::DIFFUSE].Input = Paths.size();
			Paths.push_back(FilePath);
		}

		// ------------------------------------------------------------
		// Emissive Color
		// ------------------------------------------------------------

		// Get emissive property the same way as above.
		FbxDouble3 Emissive = Lam->Emissive.Get();
		FbxDouble EmissiveFactor = Lam->EmissiveFactor.Get();

		Mat[Material_FBX::EMISSIVE].Value[0] = Emissive[0];
		Mat[Material_FBX::EMISSIVE].Value[1] = Emissive[1];
		Mat[Material_FBX::EMISSIVE].Value[2] = Emissive[2];
		Mat[Material_FBX::EMISSIVE].Factor = EmissiveFactor;

		FileTex = Lam->Emissive.GetSrcObject<FbxFileTexture>();

		if (FileTex)
		{
			const char* FileName = FileTex->GetRelativeFileName();
			file_path_t FilePath;
			strcpy(FilePath.data(), FileName);
			Mat[Material_FBX::EMISSIVE].Input = Paths.size();
			Paths.push_back(FilePath);
		}

		// ------------------------------------------------------------
		// Specular Color
		// ------------------------------------------------------------

		FbxSurfacePhong* Spec = (FbxSurfacePhong*)SurfaceMat;

		// Get the Specular of the material.
		if (SurfaceMat->Is<FbxSurfacePhong>())
		{
			FbxDouble3 Specular = Spec->Specular.Get();
			FbxDouble SpecularFactor = Spec->SpecularFactor.Get();

			Mat[Material_FBX::SPECULAR].Value[0] = Specular[0];
			Mat[Material_FBX::SPECULAR].Value[1] = Specular[1];
			Mat[Material_FBX::SPECULAR].Value[2] = Specular[2];
			Mat[Material_FBX::SPECULAR].Factor = SpecularFactor;

			FileTex = Spec->Emissive.GetSrcObject<FbxFileTexture>();

			if (FileTex)
			{
				const char* FileName = FileTex->GetRelativeFileName();
				file_path_t FilePath;
				strcpy(FilePath.data(), FileName);
				Mat[Material_FBX::SPECULAR].Input = Paths.size();
				Paths.push_back(FilePath);
			}
		}

		// ------------------------------------------------------------
		// Finish and Clean Up
		// ------------------------------------------------------------

		Materials.push_back(Mat);
	}

	// ------------------------------------------------------------
	// Write Material Count, Materials, and Path Count to File.
	// ------------------------------------------------------------

}


//--------------------------------------------------------------------------------------
// Animation information extraction.
//--------------------------------------------------------------------------------------

// Save animation and bind post information for a .FBX mesh.
void FBXExporter::SaveAnimation(const char* AnimName)
{
	// Open a file using binary truncation.
	ofstream file(AnimName, ios::trunc | ios::binary | ios::out);

	// Ensure the file was opened properly.
	assert(file.is_open());

	// Make holder for number of joints in the bind pose.
	int NumBindJoints = BindPoses[0].Joints.size();

	// Write out the bind pose for the animation.
	file.write((const char*)&NumBindJoints, sizeof(uint32_t));
	file.write((const char*)BindPoses[0].Joints.data(), sizeof(PAnim::Joint) * NumBindJoints);

	// Setup holders for the count of animation storage variables.
	int NumFrames = AnimClips[0].Frames.size();

	// Write all appropriate data to the new file and overwrite the old data.
	file.write((const char*)&AnimClips[0].Duration, sizeof(double));
	file.write((const char*)&NumFrames, sizeof(uint32_t));

	for (unsigned int i = 1; i < NumFrames; ++i)
	{
		int NumJoints = AnimClips[0].Frames[i].Joints.size();

		file.write((const char*)&AnimClips[0].Frames[i].Time, sizeof(double));
		file.write((const char*)&NumJoints, sizeof(uint32_t));
		file.write((const char*)AnimClips[0].Frames[i].Joints.data(), sizeof(PAnim::Joint) * NumJoints);// *NumJoints);
	}

	// Close the file.
	file.close();
}

// Read the animation data for the joint animation.
void FBXExporter::ReadJointAnimations()
{
	FbxNode* RootNode = lScene->GetRootNode();
	int RootChildCount = RootNode->GetChildCount();

	for (unsigned int a = 0; a < RootChildCount; ++a)
	{
		RecursiveJointRead(RootNode, 0, -1);
	}

	FbxAnimStack* AnimStack = lScene->GetCurrentAnimationStack();

	FbxTimeSpan TimeSpan		= AnimStack->GetLocalTimeSpan();
	FbxTime		TimeDuration	= TimeSpan.GetDuration();
	float		Duration		= TimeDuration.GetSecondDouble();
	int			FrameCount		= (int)TimeDuration.GetFrameCount(FbxTime::eFrames24);

	//for (unsigned int j = 0; j < BindPoses.size(); ++j)
	{
		PAnim::AnimClip NewClip;
		NewClip.Duration	= Duration;
	
		for (unsigned int i = 0; i < FrameCount; ++i)
		{
			TimeDuration.SetFrame(i, FbxTime::eFrames24);

			PAnim::Keyframe NewKeyframe;
			NewKeyframe.Time = (float)TimeDuration.GetSecondDouble();

			for (unsigned int k = 0; k < FbxJoints.size(); ++k)
			{
				FbxNode* CurrNode = FbxJoints[k].Node;

				if (CurrNode)
				{
					PAnim::Joint NewJoint;
					NewJoint.ParentIndex = FbxJoints[k].ParentIndex;

					FbxAMatrix Trans = CurrNode->EvaluateGlobalTransform(TimeDuration);
					NewJoint.Transform[0] = (float)Trans.Get(0, 0);
					NewJoint.Transform[1] = (float)Trans.Get(0, 1);
					NewJoint.Transform[2] = (float)Trans.Get(0, 2);
					NewJoint.Transform[3] = (float)Trans.Get(0, 3);
					NewJoint.Transform[4] = (float)Trans.Get(1, 0);
					NewJoint.Transform[5] = (float)Trans.Get(1, 1);
					NewJoint.Transform[6] = (float)Trans.Get(1, 2);
					NewJoint.Transform[7] = (float)Trans.Get(1, 3);
					NewJoint.Transform[8] = (float)Trans.Get(2, 0);
					NewJoint.Transform[9] = (float)Trans.Get(2, 1);
					NewJoint.Transform[10] = (float)Trans.Get(2, 2);
					NewJoint.Transform[11] = (float)Trans.Get(2, 3);
					NewJoint.Transform[12] = (float)Trans.Get(3, 0);
					NewJoint.Transform[13] = (float)Trans.Get(3, 1);
					NewJoint.Transform[14] = (float)Trans.Get(3, 2);
					NewJoint.Transform[15] = (float)Trans.Get(3, 3);

					NewKeyframe.Joints.push_back(NewJoint);
				}
			}

			NewClip.Frames.push_back(NewKeyframe);
		}

		AnimClips.push_back(NewClip);
	}
}

// Recurse through each joint of a skeleton given a node to use and build a list of joints.
void FBXExporter::RecursiveJointRead(FbxNode* Node, int Index, int ParentIndex)
{
	if (Node->GetNodeAttribute() &&
		Node->GetNodeAttribute()->GetAttributeType() &&
		Node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		FBX_Joint NewJoint;

		NewJoint.Node = Node;
		NewJoint.ParentIndex = ParentIndex;

		FbxJoints.push_back(NewJoint);
	}

	for (unsigned int i = 0; i < Node->GetChildCount(); ++i)
	{
		RecursiveJointRead(Node->GetChild(i), FbxJoints.size(), Index);
	}
}


//--------------------------------------------------------------------------------------
// Bind Pose information extraction.
//--------------------------------------------------------------------------------------

// Read in information to save for animation data.
void FBXExporter::ReadBindPoses()
{
	// ------------------------------------------------------------
	// Read in Bind Pose Information.
	// ------------------------------------------------------------

	int PoseCount = lScene->GetPoseCount();

	for (unsigned int i = 0; i < PoseCount; ++i)
	{
		FbxPose* BindPose = lScene->GetPose(i);

		if (BindPose->IsBindPose())
		{
			Poses.push_back(BindPose);

			int BindPoseCount = BindPose->GetCount();

			PBindPose NewBindPose;

			for (unsigned int j = 0; j < BindPoseCount; ++j)
			{
				FbxNode* SkeletonNode = BindPose->GetNode(j);
				FbxSkeleton* Skeleton = SkeletonNode->GetSkeleton();

				if (Skeleton && Skeleton->IsSkeletonRoot())
				{
					Skeletons.push_back(Skeleton);

					int NodesCount = SkeletonNode->GetChildCount();

					for (int k = -1; k < NodesCount - 1; k++)
					{
						FbxNode* CurrNode = SkeletonNode->GetChild(k + 1);

						RecursiveBindJointRead(&NewBindPose, SkeletonNode, 0, -1);
					}
				}
			}

			//PBindPose NewBindPose(Joints, Nodes);
			BindPoses.push_back(NewBindPose);
		}
	}
}

// Recurse through each joint of a bind pose given a node to use and build a list of joints.
void FBXExporter::RecursiveBindJointRead(PBindPose* Pose, FbxNode* Node, int Index, int ParentIndex)
{
	if 	(Node->GetNodeAttribute() &&
		Node->GetNodeAttribute()->GetAttributeType() &&
		Node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	{
		PAnim::Joint NewJoint;

		FbxAMatrix Trans = Node->EvaluateGlobalTransform().Inverse();

		NewJoint.ParentIndex = ParentIndex;

		NewJoint.Transform[0] = (float)Trans.Get(0, 0);
		NewJoint.Transform[1] = (float)Trans.Get(0, 1);
		NewJoint.Transform[2] = (float)Trans.Get(0, 2);
		NewJoint.Transform[3] = (float)Trans.Get(0, 3);
		NewJoint.Transform[4] = (float)Trans.Get(1, 0);
		NewJoint.Transform[5] = (float)Trans.Get(1, 1);
		NewJoint.Transform[6] = (float)Trans.Get(1, 2);
		NewJoint.Transform[7] = (float)Trans.Get(1, 3);
		NewJoint.Transform[8] = (float)Trans.Get(2, 0);
		NewJoint.Transform[9] = (float)Trans.Get(2, 1);
		NewJoint.Transform[10] = (float)Trans.Get(2, 2);
		NewJoint.Transform[11] = (float)Trans.Get(2, 3);
		NewJoint.Transform[12] = (float)Trans.Get(3, 0);
		NewJoint.Transform[13] = (float)Trans.Get(3, 1);
		NewJoint.Transform[14] = (float)Trans.Get(3, 2);
		NewJoint.Transform[15] = (float)Trans.Get(3, 3);

		Pose->Joints.push_back(NewJoint);
		Pose->Nodes.push_back(Node);
	}

	for (unsigned int i = 0; i < Node->GetChildCount(); ++i)
	{
		RecursiveBindJointRead(Pose, Node->GetChild(i), Pose->Joints.size(), Index);
	}
}


//--------------------------------------------------------------------------------------
// Get weigt information for mesh skin.
//--------------------------------------------------------------------------------------

// Read in the weight information of this model.
void FBXExporter::ReadWeightData()
{
	std::vector<Influence>* InfluenceVector = new std::vector<Influence>[simpleMesh.VertexList.size()];

	int MyInt = InfluenceVector->capacity();
	int MyIntA = InfluenceVector->size();

	for (unsigned int i = 0; i < Poses.size(); ++i)
	{
		for (unsigned int j = 0; j < Poses[i]->GetCount(); ++j)
		{
			FbxNode* Node = Poses[i]->GetNode(j);
			FbxMesh* Mesh = Node->GetMesh();

			if (Mesh)
			{
				FbxMeshes.push_back(Mesh);

				for (unsigned int k = 0; k < Mesh->GetDeformerCount(); ++k)
				{
					FbxDeformer* Deformer = Mesh->GetDeformer(k);

					if (Deformer->Is<FbxSkin>())
					{
						FbxSkin* ThisSkin = (FbxSkin*)Deformer;
						FbxSkins.push_back(ThisSkin);

						for (unsigned int l = 0; l < ThisSkin->GetClusterCount(); ++l)
						{
							FbxCluster* Cluster = ThisSkin->GetCluster(l);

							FbxClusters.push_back(Cluster);

							double* WeightList = Cluster->GetControlPointWeights();
							int* WeightIndices = Cluster->GetControlPointIndices();
							int VertexCount = Cluster->GetControlPointIndicesCount();

							FbxNode* ClusterNode = Cluster->GetLink();
							int JointIndex = 0;

							for (unsigned int m = 0; m < FbxJoints.size(); ++m)
							{
								if (ClusterNode == FbxJoints[m].Node)
								{
									JointIndex = j;
									break;
								}
							}

							for (unsigned int m = 0; m < VertexCount; ++m)
							{
								if (ClusterNode == FbxJoints[m].Node)
								{
									Influence Inf;
									Inf.Joint =  JointIndex;
									Inf.Weight = (float)WeightList[m];
									InfluenceVector[WeightIndices[m]].push_back(Inf);
								}
							}

							for (unsigned int m = 0; m < simpleMesh.VertexList.size(); ++m)
							{
								float4 TempWeights;
								TempWeights.x = simpleMesh.VertexList[m].Weights.x;
								TempWeights.y = simpleMesh.VertexList[m].Weights.y;
								TempWeights.z = simpleMesh.VertexList[m].Weights.z;
								TempWeights.w = simpleMesh.VertexList[m].Weights.w;

								for (unsigned int n = 0; n < InfluenceVector[m].size(); ++n)
								{
									int Low = 0;

									for (unsigned int o = 1; o < 4; ++o)
									{
										if (TempWeights[Low] > TempWeights[o])
										{
											Low = o;
										}
									}

									if (TempWeights[Low] < (InfluenceVector[m])[n].Weight)
									{
										simpleMesh.VertexList[m].Weights[Low] = (InfluenceVector[m])[n].Weight;
										simpleMesh.VertexList[m].Joints[Low] = (InfluenceVector[m])[n].Joint;
									}
								}

								float TotalWeight = 0;

								for (unsigned int n = 0; n < 4; ++n)
								{
									TotalWeight += simpleMesh.VertexList[m].Weights[n];
								}

								for (unsigned int n = 0; n < 4; ++n)
								{
									simpleMesh.VertexList[m].Weights[n] /= TotalWeight;
								}

								/*for (unsigned int n = 0; n < simpleMesh.VertexList.size(); ++n)
								{
									float TotalWeight = 0;

									for (unsigned int o = 0; o < 4; ++o)
									{
										TotalWeight += simpleMesh.VertexList[n].Weights[o];

									}

									for (unsigned int o = 0; o < 4; ++o)
									{
										simpleMesh.VertexList[n].Weights[o] /= TotalWeight;
									}
								}*/
							}
						}
					}
				}
			}
		}
	}

	delete[] InfluenceVector;
}