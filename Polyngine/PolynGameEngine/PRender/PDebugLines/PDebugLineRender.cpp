#include "PDebugLineRender.h"
#include "../../PObjects/PObject/PObject.h"
#include "../../PObjects/PSkeletalMesh/PSkeletalMesh.h"
#include "../../PSystem/PAnimation/PAnim/PAnim.h"
#include <array>

namespace
{
	constexpr size_t MAX_LINE_VERTS = 4096;

	size_t LineVertCount = 0;
	std::array<PMath::colored_vertex, MAX_LINE_VERTS> LineVerts;
}

namespace DebugLines
{
	void AddLine(PMath::float3 A, PMath::float3 B, PMath::float4 ColorA, PMath::float4 ColorB)
	{
		LineVerts[LineVertCount] = { A, ColorA };
		LineVertCount++;
		LineVerts[LineVertCount] = { B, ColorB };
		LineVertCount++;
	}

	void AddObject(PObject* Object, DrawType Type)
	{
		if (Object->GetVisibility())
		{
			// For each object, draw the matrix with red, blue, and green lines.
			float3 ObjectScale = Object->GetScale();

			// Get position and directional vectors.
			float3 Object_Location = Object->GetLocation();
			float3 RightVector = Object->GetRightVector();
			float3 UpVector = Object->GetUpVector();
			float3 ForwardVector = Object->GetForwardVector();

			// Find point in front of the current local position.
			float3 XYZ_x = Object_Location + (ForwardVector * 1.0f);
			float3 XYZ_y = Object_Location + (UpVector * 1.0f);
			float3 XYZ_z = Object_Location + (RightVector * 1.0f);

			// Draw the three lines.
			if (Type == OBJECT)
			{
				AddLine(Object_Location, XYZ_x, { 1, 0, 0, 1 });
				AddLine(Object_Location, XYZ_y, { 0, 1, 0, 1 });
				AddLine(Object_Location, XYZ_z, { 0, 0, 1, 1 });
			}
			else if (Type == CAMERA)
			{
				AddLine(Object_Location, XYZ_x, { 0.96f, 0.26f, 0.78f, 1 });
				AddLine(Object_Location, XYZ_y, { 0.96f, 0.26f, 0.78f, 1 });
				AddLine(Object_Location, XYZ_z, { 0.96f, 0.26f, 0.78f, 1 });
			}
			else if (Type == DIRECTIONAL)
			{
				AddLine(Object_Location, XYZ_z, { 0.96f, 0.91f, 0.26f, 1 });
			}
			else if (Type == POINT)
			{
				AddLine(Object_Location, XYZ_x, { 1.0f, 1.0f, 1.0f, 1.0f });
				AddLine(Object_Location, XYZ_y, { 1.0f, 1.0f, 1.0f, 1.0f });
				AddLine(Object_Location, XYZ_z, { 1.0f, 1.0f, 1.0f, 1.0f });
			}
		}
	}

	void AddJointTransform(float Trans[16])
	{
		// Get position and directional vectors.
		float3 Object_Location = { Trans[12], Trans[13], Trans[14] };
		float3 RightVector = { Trans[0], Trans[1], Trans[2] };
		float3 UpVector = { Trans[4], Trans[5], Trans[6] };
		float3 ForwardVector = { Trans[8], Trans[9], Trans[10] };

		// Find point in front of the current local position.
		float3 XYZ_x = Object_Location + (ForwardVector * 0.1f);
		float3 XYZ_y = Object_Location + (UpVector * 0.1f);
		float3 XYZ_z = Object_Location + (RightVector * 0.1f);

		// Draw the three lines.
		AddLine(Object_Location, XYZ_x, { 1, 0, 0, 1 });
		AddLine(Object_Location, XYZ_y, { 0, 1, 0, 1 });
		AddLine(Object_Location, XYZ_z, { 0, 0, 1, 1 });
	}

	void AddSkeleton(PSkeletalMesh* Object)
	{
		if (Object && Object->Animator.GetReady())
		{
			float4 BoneColor = { 1.0f, 1.0f, 1.0f, 1.0f };

			std::vector<PAnim::Joint> Joints = Object->Animator.GetLerpKeyframe().Joints;

			for (unsigned int i = 0; i < Joints.size(); ++i)
			{
				if (Joints[i].ParentIndex >= 0 && Joints[i].ParentIndex < Joints.size())
				{
					float3 StartPos = { Joints[i].Transform[12], Joints[i].Transform[13], Joints[i].Transform[14] };
					float3 EndPos = { Joints[Joints[i].ParentIndex].Transform[12], Joints[Joints[i].ParentIndex].Transform[13], Joints[Joints[i].ParentIndex].Transform[14] };
					AddLine(StartPos, EndPos, BoneColor);
					AddJointTransform(Joints[i].Transform);
				}
			}
		}
	}

	void AddBindPoseSkeleton(PSkeletalMesh* Object)
	{
		if (Object && Object->Animator.GetReady())
		{
			float4 BoneColor = { 1.0f, 1.0f, 1.0f, 1.0f };

			std::vector<PAnim::Joint> Joints = Object->Animator.GetBindPose().Joints;

			for (unsigned int i = 0; i < Joints.size(); ++i)
			{
				if (Joints[i].ParentIndex >= 0 && Joints[i].ParentIndex < Joints.size())
				{
					float3 StartPos = { Joints[i].Transform[12], Joints[i].Transform[13], Joints[i].Transform[14] };
					float3 EndPos = { Joints[Joints[i].ParentIndex].Transform[12], Joints[Joints[i].ParentIndex].Transform[13], Joints[Joints[i].ParentIndex].Transform[14] };
					AddLine(StartPos, EndPos, BoneColor);
					AddJointTransform(Joints[i].Transform);
				}
			}
		}
	}

	void AddBoxCollider(PMath::PAABB BoundsBox)
	{
		float4 ColliderLineColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		float3 Origin = BoundsBox.Center;
		float3 Ext = BoundsBox.Extents;

		// Top Face.
		AddLine({ Origin.x - Ext.x, Origin.y + Ext.y, Origin.z - Ext.z }, { Origin.x + Ext.x, Origin.y + Ext.y, Origin.z - Ext.z }, ColliderLineColor);
		AddLine({ Origin.x - Ext.x, Origin.y + Ext.y, Origin.z + Ext.z }, { Origin.x + Ext.x, Origin.y + Ext.y, Origin.z + Ext.z }, ColliderLineColor);
		AddLine({ Origin.x - Ext.x, Origin.y + Ext.y, Origin.z - Ext.z }, { Origin.x - Ext.x, Origin.y + Ext.y, Origin.z + Ext.z }, ColliderLineColor);
		AddLine({ Origin.x + Ext.x, Origin.y + Ext.y, Origin.z - Ext.z }, { Origin.x + Ext.x, Origin.y + Ext.y, Origin.z + Ext.z }, ColliderLineColor);

		// Bottom Face.
		AddLine({ Origin.x - Ext.x, Origin.y - Ext.y, Origin.z - Ext.z }, { Origin.x + Ext.x, Origin.y - Ext.y, Origin.z - Ext.z }, ColliderLineColor);
		AddLine({ Origin.x - Ext.x, Origin.y - Ext.y, Origin.z + Ext.z }, { Origin.x + Ext.x, Origin.y - Ext.y, Origin.z + Ext.z }, ColliderLineColor);
		AddLine({ Origin.x - Ext.x, Origin.y - Ext.y, Origin.z - Ext.z }, { Origin.x - Ext.x, Origin.y - Ext.y, Origin.z + Ext.z }, ColliderLineColor);
		AddLine({ Origin.x + Ext.x, Origin.y - Ext.y, Origin.z - Ext.z }, { Origin.x + Ext.x, Origin.y - Ext.y, Origin.z + Ext.z }, ColliderLineColor);

		// Front Face.
		AddLine({ Origin.x - Ext.x, Origin.y + Ext.y, Origin.z - Ext.z }, { Origin.x - Ext.x, Origin.y - Ext.y, Origin.z - Ext.z }, ColliderLineColor);
		AddLine({ Origin.x + Ext.x, Origin.y + Ext.y, Origin.z - Ext.z }, { Origin.x + Ext.x, Origin.y - Ext.y, Origin.z - Ext.z }, ColliderLineColor);

		// Back Face.
		AddLine({ Origin.x - Ext.x, Origin.y + Ext.y, Origin.z + Ext.z }, { Origin.x - Ext.x, Origin.y - Ext.y, Origin.z + Ext.z }, ColliderLineColor);
		AddLine({ Origin.x + Ext.x, Origin.y + Ext.y, Origin.z + Ext.z }, { Origin.x + Ext.x, Origin.y - Ext.y, Origin.z + Ext.z }, ColliderLineColor);
	}

	void ClearLines()
	{
		LineVertCount = 0;
	}

	const PMath::colored_vertex* GetLineVerts()
	{
		return LineVerts.data();
	}

	size_t GetLineVertCount()
	{
		return LineVertCount;
	}

	size_t GetLineVertCapacity()
	{
		return MAX_LINE_VERTS;
	}

	void DrawGrid(int GridSize, PMath::float4 GridColor)
	{
		// Draw the grid for the ground.
		int InitialOffset = (int)round((float)GridSize / 2.0f);

		for (int x = 0; x < (GridSize + 1); x+=2)
		{
			AddLine({ (float)x - InitialOffset, 0, 0.0f - (float)InitialOffset }, { (float)x - InitialOffset, 0, (float)GridSize - InitialOffset }, GridColor);
			AddLine({ 0.0f - (float)InitialOffset, 0, (float)x - InitialOffset }, { (float)GridSize - InitialOffset, 0, (float)x - InitialOffset }, GridColor);
		}
	}
}
