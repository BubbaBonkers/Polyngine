#pragma once

#include <DirectXMath.h>
#include <array>
#include <vector>
#include <string>
#include <cstdint>

#define PI 3.14159265359f

using namespace DirectX;

namespace PMath
{
	struct float2
	{
		float x;
		float y;

		inline float& operator[](int i) { return (&x)[i]; }
		inline float operator[](int i)const { return (&x)[i]; }

		inline float* data() { return &x; }
		inline const float* data()const { return &x; }
		inline static constexpr size_t size() { return 2; }
	};

	struct float3
	{
		union
		{
			struct
			{
				float x;
				float y;
				float z;
			};

			float2 xy;
		};


		inline float& operator[](int i) { return (&x)[i]; }
		inline float operator[](int i)const { return (&x)[i]; }

		inline float* data() { return &x; }
		inline const float* data()const { return &x; }
		inline static constexpr size_t size() { return 3; }

		inline friend float3 operator+(float3 lhs, float3 rhs)
		{
			return { lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z };
		}

		inline friend float3 operator-(float3 lhs, float3 rhs)
		{
			return { lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z };
		}

		inline friend float3 operator*(float3 lhs, float3 rhs)
		{
			return { lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z };
		}

		inline friend float3 operator*(float3 lhs, float rhs)
		{
			return { lhs.x * rhs, lhs.y * rhs, lhs.z * rhs };
		}

		inline friend float3 operator/(float3 lhs, float3 rhs)
		{
			return { lhs.x / rhs.x, lhs.y / rhs.y, lhs.z / rhs.z };
		}

		inline friend float dot(float3 lhs, float3 rhs)
		{
			return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
		}

		inline friend float3 cross(float3 lhs, float3 rhs)
		{
			return { lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x };
		}

		inline float3& operator+=(float3 rhs)
		{
			x += rhs.x;
			y += rhs.y;
			z += rhs.z;

			return *this;
		}

		inline float3& operator-=(float3 rhs)
		{
			x -= rhs.x;
			y -= rhs.y;
			z -= rhs.z;

			return *this;
		}

		inline float3& operator*=(float3 rhs)
		{
			x *= rhs.x;
			y *= rhs.y;
			z *= rhs.z;

			return *this;
		}

		inline float3& operator/=(float3 rhs)
		{
			x /= rhs.x;
			y /= rhs.y;
			z /= rhs.z;

			return *this;
		}

		inline float3& operator*=(float rhs)
		{
			x *= rhs;
			y *= rhs;
			z *= rhs;

			return *this;
		}

		inline float3& operator/=(float rhs)
		{
			x /= rhs;
			y /= rhs;
			z /= rhs;

			return *this;
		}
	};

	struct float4
	{
		union
		{
			struct
			{
				float x;
				float y;
				float z;
				float w;
			};

			float3 xyz;

			struct
			{
				float2 xy;
				float2 zw;
			};
		};

		inline float& operator[](int i) { return (&x)[i]; }
		inline float operator[](int i)const { return (&x)[i]; }

		inline float* data() { return &x; }
		inline const float* data()const { return &x; }
		inline static constexpr size_t size() { return 4; }
	};

	struct alignas(8) float2_a : float2 {};

	struct alignas(16) float3_a : float3 {};

	struct alignas(16) float4_a : float4 {};

	using float4x4 = std::array< float4, 4 >;
	using float4x4_a = std::array< float4_a, 4 >;

	/**
	* This will return whether or not two things are nearly equal.
	* You can change the delta to alter how much difference is allowed.
	* The type used must have == , + , and - defined.
	*/
	bool IsNearlyEqual(float Lhs, float Rhs, float Delta);

	/**
	* This will return whether or not two things are nearly equal.
	* You can change the delta to alter how much difference is allowed.
	* The type used must have == , + , and - defined.
	*/
	bool IsNearlyEqual(int Lhs, int Rhs, int Delta);

	// Convert a Vector into a float3.
	float3 PVector_Float3(XMVECTOR In);

	// Convert a float3 into a vector.
	XMVECTOR PFloat3_Vector(float3 In);

	// Convert a number from degrees to radians.
	float PDegrees_Radians(float Degrees);
	float PRadians_Degrees(float Radians);

	struct colored_vertex
	{
		float3 pos = { 0.0f, 0.0f, 0.0f };
		float4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

		colored_vertex() = default;
		colored_vertex(const colored_vertex&) = default;

		inline colored_vertex(const float3& p, const float3& c) : pos{ p }, color{ c.x, c.y, c.z } {}
		inline colored_vertex(const float3& p, const float4& c) : pos{ p }, color{ c.x, c.y, c.z, c.w } {}
		inline colored_vertex(const float3& p, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) : pos{ p }, color{ r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f } {}
	};

	// A structure containing Position, Normal, and Texture information for a single point.
	struct Vertex
	{
	public:
		float3		Position;									// The position of this vertex in 3D space.
		float3		Normal;										// The normal data of this vertex.
		float2		Texture;									// Texture data for this vertex.
		float4		Weights = { 0.0f, 0.0f, 0.0f, 0.0f };		// Weights on joint effects.
		int			JointIndices[4] = { -1, -1, -1, -1 };		// Indices of joints to affects.
	};

	class view_t
	{
	public:
		float4x4_a ViewMatrix;
		float4x4_a ProjectionMatrix;

		view_t() {}
	};

	struct PSphere { float3 Center; float Radius; };

	struct PAABB { float3 Center; float3 Extents; };

	struct PPlane { float3 Normal; float Offset; };

	struct PFrustum { PPlane Planes[6]; };

	// Calculates the plane of a triangle from three points.
	PPlane PCalculatePlane(float3 a, float3 b, float3 c);

	// Calculates a frustum (6 planes) from the input view parameter.
	PFrustum PCalculateFrustum(const view_t& View, int ScreenX, int ScreenY);

	// Calculates which side of a plane the sphere is on.
	// Returns -1 if the sphere is completely behind the plane.
	// Returns 1 if the sphere is completely in front of the plane.
	// Returns 0 is the Sphere overlaps the plane.
	int PClassifySphereToPlane(const PSphere& Sphere, const PPlane& Plane);

	// Calculates which side of a plane the aabb is on.
	// Returns -1 if the aabb is completely behind the plane.
	// Returns 1 if the aabb is completely in front of the plane.
	// Returns 0 if aabb overlaps the plane.
	int PClassifyAABBToPlane(const PAABB& aabb, const PPlane& Plane);

	// Determines if the aabb is inside the frustum.
	// Returns false if the aabb is completely behind any plane.
	// Otherwise returns true.
	bool PAABBToFrustum(const PAABB& aabb, const PFrustum& Frustum);

	// Clamp a float Val between minimum Min and maximum Max.
	float fclamp(float Val, float Min, float Max);
}