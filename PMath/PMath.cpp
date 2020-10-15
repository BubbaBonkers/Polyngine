#include "PMath.h"

namespace PMath
{
	// Calculates the plane of a triangle from three points.
	PPlane PCalculatePlane(float3 a, float3 b, float3 c)
	{
		DirectX::XMVECTOR PlaneNormal = XMVector3Normalize(XMVector3Cross(PFloat3_Vector(b - a), PFloat3_Vector(c - b)));
		DirectX::XMVECTOR PlaneOffset = XMVector3Dot(PlaneNormal, PFloat3_Vector(a));

		PPlane Plane = PPlane{};
		Plane.Normal = PVector_Float3(PlaneNormal);
		Plane.Offset = XMVectorGetY(PlaneOffset);

		return Plane;
	}

	// Calculates a frustum (6 planes) from the input view parameter.
	PFrustum PCalculateFrustum(const view_t& View, int ScreenX, int ScreenY)
	{
		XMMATRIX Ident = XMMatrixIdentity();

		// Get points for each plane to use in calculate_plane function.
		DirectX::XMVECTOR FTL = XMVector3Unproject(XMVectorSet(0, (ScreenY), 1, 1), 0, 0, ScreenX, ScreenY, 0, 1, (XMMATRIX&)View.ProjectionMatrix, (XMMATRIX&)View.ViewMatrix, Ident);
		DirectX::XMVECTOR FTR = XMVector3Unproject(XMVectorSet((ScreenX), (ScreenY), 1, 1), 0, 0, ScreenX, ScreenY, 0, 1, (XMMATRIX&)View.ProjectionMatrix, (XMMATRIX&)View.ViewMatrix, Ident);
		DirectX::XMVECTOR FBL = XMVector3Unproject(XMVectorSet(0, 0, 1, 1), 0, 0, ScreenX, ScreenY, 0, 1, (XMMATRIX&)View.ProjectionMatrix, (XMMATRIX&)View.ViewMatrix, Ident);
		DirectX::XMVECTOR FBR = XMVector3Unproject(XMVectorSet((ScreenX), 0, 1, 1), 0, 0, ScreenX, ScreenY, 0, 1, (XMMATRIX&)View.ProjectionMatrix, (XMMATRIX&)View.ViewMatrix, Ident);
		DirectX::XMVECTOR NTL = XMVector3Unproject(XMVectorSet(0, (ScreenY), 0, 1), 0, 0, ScreenX, ScreenY, 0, 1, (XMMATRIX&)View.ProjectionMatrix, (XMMATRIX&)View.ViewMatrix, Ident);
		DirectX::XMVECTOR NTR = XMVector3Unproject(XMVectorSet((ScreenX), (ScreenY), 0, 1), 0, 0, ScreenX, ScreenY, 0, 1, (XMMATRIX&)View.ProjectionMatrix, (XMMATRIX&)View.ViewMatrix, Ident);
		DirectX::XMVECTOR NBL = XMVector3Unproject(XMVectorSet(0, 0, 0, 1), 0, 0, ScreenX, ScreenY, 0, 1, (XMMATRIX&)View.ProjectionMatrix, (XMMATRIX&)View.ViewMatrix, Ident);
		DirectX::XMVECTOR NBR = XMVector3Unproject(XMVectorSet((ScreenX), 0, 0, 1), 0, 0, ScreenX, ScreenY, 0, 1, (XMMATRIX&)View.ProjectionMatrix, (XMMATRIX&)View.ViewMatrix, Ident);

		// Get each plane for frustum culling.
		PPlane TopPlane = PCalculatePlane(PVector_Float3(NTL), PVector_Float3(FTL), PVector_Float3(FTR));
		PPlane BottomPlane = PCalculatePlane(PVector_Float3(NBR), PVector_Float3(FBR), PVector_Float3(FBL));
		PPlane FrontPlane = PCalculatePlane(PVector_Float3(NBR), PVector_Float3(NBL), PVector_Float3(NTL));
		PPlane BackPlane = PCalculatePlane(PVector_Float3(FBL), PVector_Float3(FBR), PVector_Float3(FTR));
		PPlane RightPlane = PCalculatePlane(PVector_Float3(FBR), PVector_Float3(NBR), PVector_Float3(NTR));
		PPlane LeftPlane = PCalculatePlane(PVector_Float3(NBL), PVector_Float3(FBL), PVector_Float3(FTL));

		// Return array of all planes.
		return { TopPlane, BottomPlane, FrontPlane, BackPlane, RightPlane, LeftPlane };
	}

	// Calculates which side of a plane the sphere is on.
	// Returns -1 if the sphere is completely behind the plane.
	// Returns 1 if the sphere is completely in front of the plane.
	// Returns 0 is the Sphere overlaps the plane.
	int PClassifySphereToPlane(const PSphere& Sphere, const PPlane& Plane)
	{
		float SphereRadius = Sphere.Radius;
		float SignedDistance = (dot(Sphere.Center, Plane.Normal) - Plane.Offset);

		if (SignedDistance > SphereRadius)
		{
			return 1;
		}
		else if (SignedDistance < -SphereRadius)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}

	// Calculates which side of a plane the aabb is on.
	// Returns -1 if the aabb is completely behind the plane.
	// Returns 1 if the aabb is completely in front of the plane.
	// Returns 0 if aabb overlaps the plane.
	int PClassifyAABBToPlane(const PAABB& aabb, const PPlane& Plane)
	{
		float3 SphereCenter = aabb.Center;
		float SphereRadius = ((aabb.Extents.x * abs(Plane.Normal.x)) + (aabb.Extents.y * abs(Plane.Normal.y)) + (aabb.Extents.z * abs(Plane.Normal.z)));
		float SignedDistance = (dot(SphereCenter, Plane.Normal) - Plane.Offset);

		if (SignedDistance > SphereRadius)
		{
			return 1;
		}
		else if (SignedDistance < -SphereRadius)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}

	// Determines if the aabb is inside the frustum.
	// Returns false if the aabb is completely behind any plane.
	// Otherwise returns true.
	bool PAABBToFrustum(const PAABB& aabb, const PFrustum& Frustum)
	{
		const float3& AABBExtents = aabb.Extents;

		for (unsigned int i = 0; i < (sizeof(Frustum) / sizeof(Frustum.Planes[0])); ++i)
		{
			if (PClassifyAABBToPlane(aabb, Frustum.Planes[i]) == -1)
			{
				return false;
			}
		}

		return true;
	}

	// Convert a Vector into a float3.
	float3 PVector_Float3(XMVECTOR In)
	{
		return { XMVectorGetX(In), XMVectorGetY(In), XMVectorGetZ(In) };
	}

	/**
	* This will return whether or not two things are nearly equal.
	* You can change the delta to alter how much difference is allowed.
	* The type used must have == , + , and - defined.
	*/
	bool IsNearlyEqual(float Lhs, float Rhs, float Delta)
	{
		return (abs(Lhs - Rhs) < Delta);
	}

	/**
	* This will return whether or not two things are nearly equal.
	* You can change the delta to alter how much difference is allowed.
	* The type used must have == , + , and - defined.
	*/
	bool IsNearlyEqual(int Lhs, int Rhs, int Delta)
	{
		return (abs(Lhs - Rhs) < Delta);
	}

	// Convert a float3 into a vector.
	XMVECTOR PFloat3_Vector(float3 In)
	{
		return { In.x, In.y, In.z, 0 };
	}

	// Convert a number from degrees to radians.
	float PDegrees_Radians(float Degrees)
	{
		return (Degrees * (PI / 180.0f));
	}

	// Convert a number from radians to degrees.
	float PRadians_Degrees(float Radians)
	{
		return ((Radians * 180.0f) / PI);
	}

	// Clamp a float Val between minimum Min and maximum Max.
	float fclamp(float Val, float Min, float Max)
	{
		return ((Val < Min) ? Min : (Val > Max) ? Max : Val);
	}
}

