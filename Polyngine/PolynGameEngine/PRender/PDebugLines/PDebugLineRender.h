#pragma once

#include "../../PMath/PMath.h"

class PObject;
class PSkeletalMesh;

namespace DebugLines
{
	enum DrawType
	{
		OBJECT,
		CAMERA,
		DIRECTIONAL,
		POINT
	};

	void AddLine(PMath::float3 A, PMath::float3 B, PMath::float4 ColorA, PMath::float4 ColorB);
	inline void AddLine(PMath::float3 A, PMath::float3 B, PMath::float4 Color) { AddLine(A, B, Color, Color); };
	void AddObject(PObject* Object, DrawType Type = OBJECT);
	void AddSkeleton(PSkeletalMesh* Object);
	void AddBindPoseSkeleton(PSkeletalMesh* Object);
	void AddBoxCollider(PMath::PAABB BoundsBox);
	void ClearLines();
	const PMath::colored_vertex* GetLineVerts();
	size_t GetLineVertCount();
	size_t GetLineVertCapacity();

	void DrawGrid(int GridSize = 50, PMath::float4 GridColor = PMath::float4{ 0.3f, 0.3f, 0.3f, 1 });
};

