#include "PGameplayStatics.h"
#include "../../PRender/PEnvironment/PEnvironment.h"

PEnvironment* PGameplayStatics::ActiveEnvironment;

// Set the active gameplay environment.
void PGameplayStatics::SetEnvironment(PEnvironment* Env)
{
	ActiveEnvironment = Env;
}

// Returns the current gameplay environment.
PEnvironment* PGameplayStatics::GetEnvironment()
{
	if (ActiveEnvironment)
	{
		return ActiveEnvironment;
	}
	else
	{
		return nullptr;
	}
}

// Returns the camera being used to render with. This can be the render camera.
PCamera* PGameplayStatics::GetActiveCamera()
{
	if (ActiveEnvironment)
	{
		return ActiveEnvironment->GetActiveCamera();
	}
}

PController* PGameplayStatics::CreateController()
{
	if (ActiveEnvironment)
	{
		return ActiveEnvironment->CreateController();
	}
	else
	{
		return nullptr;
	}
}

PCamera* PGameplayStatics::CreateCamera(float FOV, bool bAssignInput, bool bSetActive, std::string DebugName)
{
	if (ActiveEnvironment)
	{
		return ActiveEnvironment->CreateCamera(FOV, bAssignInput, bSetActive, DebugName);
	}
	else
	{
		return nullptr;
	}
}

PPointLight* PGameplayStatics::CreatePointLight(float Strength, float Radius, PMath::float4 Clr, std::string DebugName)
{
	if (ActiveEnvironment)
	{
		return ActiveEnvironment->CreatePointLight(Strength, Radius, Clr, DebugName);
	}
	else
	{
		return nullptr;
	}
}

PStaticMesh* PGameplayStatics::CreateStaticMesh(const char* ModelFilePath, const char* DDSFilePath, std::string DebugName, PMath::float3 InScale)
{
	if (ActiveEnvironment)
	{
		return ActiveEnvironment->CreateStaticMesh(ModelFilePath, DDSFilePath, true, DebugName, nullptr, InScale);
	}
	else
	{
		return nullptr;
	}
}

PSkeletalMesh* PGameplayStatics::CreateSkeletalMesh(const char* ModelFilePath, const char* DDSFilePath, std::string DebugName, PMath::float3 InScale, const char* SpecularFilePath, const char* EmissiveFilePath)
{
	if (ActiveEnvironment)
	{
		return ActiveEnvironment->CreateSkeletalMesh(ModelFilePath, DDSFilePath, SpecularFilePath, EmissiveFilePath, true, DebugName, nullptr, InScale);
	}
	else
	{
		return nullptr;
	}
}

bool PGameplayStatics::DestroyObject(PObject* Obj)
{
	if (ActiveEnvironment)
	{
		return ActiveEnvironment->DestroyObject(Obj);
	}
	else
	{
		return false;
	}
}

// Load a level into the gameplay environment. This will clear all loaded objects.
void PGameplayStatics::LoadLevel(std::string FilePath)
{
	ActiveEnvironment->LoadLevel(FilePath);
}

// Save all objects loaded into this environment to a file. If the file exists, it will 
// be overwritten.
void PGameplayStatics::SaveLevel(std::string FilePath)
{
	ActiveEnvironment->SaveLevel(FilePath);
}

// Split a string using a delimeter, then return all strings that were split in chunks.
std::vector<std::string> PGameplayStatics::SplitString(const std::string& InString, const char& Delim)
{
	std::vector<std::string> SplitS;
	std::string SBuffer = "";

	for (auto n : InString)
	{
		if (n != Delim)
		{
			SBuffer += n;
		}
		else if ((n == Delim) && (SBuffer != ""))
		{
			SplitS.push_back(SBuffer);
			SBuffer = "";
		}
	}

	if (SBuffer != "")
	{
		SplitS.push_back(SBuffer);
	}

	return SplitS;
}

// Split a string using a delimeter, then return the last element from the post-split string.
std::string PGameplayStatics::SplitStringGetLast(const std::string& InString, const char& Delim)
{
	std::vector<std::string> SplitS;
	std::string SBuffer = "";

	for (auto n : InString)
	{
		if (n != Delim)
		{
			SBuffer += n;
		}
		else if ((n == Delim) && (SBuffer != ""))
		{
			SplitS.push_back(SBuffer);
			SBuffer = "";
		}
	}

	if (SBuffer != "")
	{
		SplitS.push_back(SBuffer);
	}

	return SplitS.size() > 0 ? SplitS[SplitS.size() - 1] : "";
}

// Replace an InString with Replace at Delim character.
std::string PGameplayStatics::ReplaceString(const std::string& InString, const char& Delim, const char& Replace)
{
	std::string SBuffer = "";

	for (auto n : InString)
	{
		if (n != Delim)
		{
			SBuffer += n;
		}
		else if (n == Delim)
		{
			SBuffer += Replace;
		}
	}

	return SBuffer;
}

// Return everything after a folder by directory as a string.
std::string PGameplayStatics::SliceFilepathByDir(std::string Dir, std::string SplitAt)
{
	std::string MainDir = Dir;
	std::vector<std::string> Chunks = PGameplayStatics::SplitString(MainDir, '/');

	if (Chunks.size() == 1)
	{
		Chunks = PGameplayStatics::SplitString(MainDir, '\\');
	}

	std::string TempDir;
	bool bTrigger = false;
	for (unsigned int i = 0; i < Chunks.size(); i++)
	{
		if (Chunks[i] == SplitAt)
		{
			bTrigger = true;
		}
		else if (bTrigger && i != (Chunks.size() - 1))
		{
			TempDir += (Chunks[i] + "/");
		}
		else if (i == (Chunks.size() - 1))
		{
			TempDir += Chunks[i];
		}
	}

	return TempDir;
}

// Return the main Directory that leads to Polyn folder. Example output: "D:/User/Documents/Polyngine/"
std::string PGameplayStatics::GetMainDirectory()
{
	std::string MainDir = __FILE__;
	std::vector<std::string> Chunks = SplitString(MainDir, '\\');

	std::string TempDir;
	for (unsigned int i = 0; i < MainDir.size(); ++i)
	{
		if (Chunks[i] == "Polyngine")
		{
			TempDir += "Polyngine/";
			break;
		}
		else
		{
			TempDir += (Chunks[i] + "/");
		}
	}

	return TempDir;
}

// Return the main Directory that leads to PolynGameEngine folder. Example output: "D:/User/Documents/Polyn/PolynGameEngine/"
std::string PGameplayStatics::GetDevDirectory()
{
	return (GetMainDirectory() + "PolynGameEngine/");
}

// Return the main Directory that leads to PolynGame folder. Example output: "D:/User/Documents/Polyn/PolynGame/"
std::string PGameplayStatics::GetGameDirectory()
{
	return (GetMainDirectory() + "PolynGame/");
}

// Print some text to the output log. OutString is the string that will be printed and Status will decide the
// color and type of the string to print.
//
// Status:
//	0. Text
//	1. Success
//	2. Failure
//	3. Warning
//	4. System Command
void PGameplayStatics::PrintToConsole(std::string OutString, int Status, std::string Source)
{
	ActiveEnvironment->PrintToConsole(OutString, Status, Source);
}