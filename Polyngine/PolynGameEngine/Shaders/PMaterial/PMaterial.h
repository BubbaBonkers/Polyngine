#pragma once

#include "../../PMath/PMath.h"

// A PMaterial stores base information that will manipulate the shader used for rendering this model. From here you can animate, change, and even manipulate the output of the shader to change looks in the game.
class PMaterial
{
public:
	PMaterial() {};
	
	PMaterial(float Specular, PMath::float3 Emissive)
	{
		SetSpecular(Specular);
		SetEmissive(Emissive);
	}

	float Specular[3] = { 0.0f, 0.0f, 0.0f };
	float Emissive[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	void SetSpecular(float Input)
	{
		Specular[0] = Input;
		Specular[1] = Input;
		Specular[2] = Input;
	}

	void SetEmissive(PMath::float3 Input)
	{
		Emissive[0] = Input.x;
		Emissive[1] = Input.y;
		Emissive[2] = Input.z;
		Emissive[3] = 1.0f;
	}

	void SetEmissive(float Input[4])
	{
		memcpy(Emissive, Input, sizeof(Emissive));
	}

	PMaterial& operator=(PMaterial& Input)
	{
		SetSpecular(*Input.Specular);
		SetEmissive(Input.Emissive);

		return *this;
	}

	bool operator==(const PMaterial& Input) const
	{
		return ((Emissive == Input.Emissive) && (Specular == Input.Specular));
	}
};
