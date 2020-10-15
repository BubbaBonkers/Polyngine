#include "PAnim.h"
#include "../../../PMath/PMath.h"
#include "../../../PStatics/PGameplayStatics/PGameplayStatics.h"
#include <fstream>
#include <string>

// Called once per frame change.
//
// No return value.
void PAnim::Update(float DeltaTime)
{
	if (!Anim.GetPaused())
	{
		Anim.Time += DeltaTime;

		if (Anim.Time > Anim.GetDuration())
		{
			Anim.Time = 0.0f;
		}
	}
}

// Play an animation if it exists at the input file path. If no path is supplied, it will attempt to play any loaded animation.
//
// Returns true if the animation was loaded and played, false otherwise.
bool PAnim::Play(const char* AnimFilePath)
{
	if ((Anim.GetFile() == AnimFilePath) || ((AnimFilePath == "") && (Anim.GetFile() != "")))
	{
		Anim.Play();

		if (!Anim.GetPaused())
		{
			return true;
		}
	}
	else
	{
		if (AnimFilePath != "")
		{
			if (LoadAnimation(AnimFilePath))
			{
				Anim.Play();

				if (!Anim.GetPaused())
				{
					return true;
				}
			}
		}
	}

	return false;
}

// Pause an animation if it exists at the input file path.
//
// Returns true if the animation was able to pause, false otherwise.
bool PAnim::Pause()
{
	if (Anim.GetFile() != "")
	{
		Anim.Pause();

		if (Anim.GetPaused())
		{
			return true;
		}
	}

	return false;
}

// Get whether the current animation is being played or not.
//
// Returns true if the animation clip loaded is playing.
bool PAnim::GetPaused()
{
	return Anim.GetPaused();
}

// Get the bind pose that is currently loaded in.
//
// Returns the current bind pose.
PAnim::BindPose PAnim::GetBindPose()
{
	return Anim.GetBindPose();
}

// Get the total duration of the currently loaded animation, if any.
	//
	// Returns the duration in the form of a double.
double PAnim::GetDuration()
{
	if (GetReady())
	{
		return Anim.GetDuration();
	}
	else
	{
		return 0.0;
	}
}

// Return the current Keyframe that the animation is on.
//
// Returns a keyframe struct.
PAnim::Keyframe PAnim::GetKeyframe()
{
	return Anim.GetClip().Frames[Anim.GetFrameAtTime()];
}

// Return the current Keyframe that the animation is on.
//
// Returns a keyframe struct.
PAnim::Keyframe PAnim::GetKeyframe(float InTime)
{
	return Anim.GetClip().Frames[Anim.GetFrameAtTime(InTime)];
}

// Return the keyframe from the current animation at index Frame.
//
// Returns a keyframe struct.
PAnim::Keyframe PAnim::GetKeyframeAt(int FrameNum)
{
	return Anim.GetClip().Frames[FrameNum];
}

// Return the next Keyframe that the animation is will move to.
//
// Returns a keyframe struct.
PAnim::Keyframe PAnim::GetNextKeyframe()
{
	return GetKeyframeAt(Anim.GetFrameAtTime() + 1);
}

// Return the next Keyframe that the animation is will move to from StartFrame.
//
// Returns a keyframe struct.
PAnim::Keyframe PAnim::GetNextKeyframe(PAnim::Keyframe StartFrame)
{
	// This is broken.
	int NextFrame = Anim.GetFrameAtTime(StartFrame.Time) + 1;
	int MaxSize = Anim.GetClip().Frames.size();

	if ((NextFrame < MaxSize) && (NextFrame >= 0))
	{
		return GetKeyframeAt(NextFrame);
	}
	else
	{
		return GetKeyframeAt(MaxSize - 1);
	}
}

// Returns a keyframe containing animation information for a blended frame between 
// two other frames. Essentially blends two keyframes that exist into a new keyframe 
// containing a new frame that would exist between frame A and B. CheckTime is the 
// time to get the blended frame for.
PAnim::Keyframe PAnim::GetLerpKeyframe(float CheckTime)
{
	Keyframe CurrFrame = (CheckTime < 0.0f) ? GetKeyframe(Anim.GetTime()) : GetKeyframe(CheckTime);
	Keyframe NextFrame = GetNextKeyframe(CurrFrame);

	Keyframe Blended;
	Blended.Time = (CheckTime < 0.0f) ? Anim.Time : CheckTime;
	Blended.Joints.resize(CurrFrame.Joints.size());

	float CurrTime = abs(Blended.Time - CurrFrame.Time);
	float MaxTime = (NextFrame.Time >= CurrFrame.Time) ? abs(NextFrame.Time - CurrFrame.Time) : (abs(GetDuration() - CurrFrame.Time));

	float LerpTime = fclamp(1 - (CurrTime/MaxTime), 0.f, 1.f);

	for (unsigned int i = 0; i < CurrFrame.Joints.size(); ++i)
	{
		Blended.Joints[i].ParentIndex = CurrFrame.Joints[i].ParentIndex;

		for (unsigned int x = 0; x < 16; ++x)
		{
			Blended.Joints[i].Transform[x] = std::lerp(CurrFrame.Joints[i].Transform[x], NextFrame.Joints[i].Transform[x], LerpTime);
		}
	}

	return Blended;
}

// Jump the current frame ahead or behind by Num number of frames, if possible.
//
// No return value.
void PAnim::FrameJump(int Num)
{
	Anim.FrameJump(Num);
}

// Set the current time to this keyframe, if an animation exists.
//
// No return value.
void PAnim::FrameSet(int Frame)
{
	Anim.FrameSet(Frame);
}

// Set the current time to this keyframe, if an animation exists.
//
// No return value.
void PAnim::TimeSet(float pTime)
{
	Anim.TimeSet(pTime);
}

// Get the filepath for the currently loaded animation.
//
// Returns the filepath.
std::string PAnim::GetFile()
{
	return Anim.GetFile();
}

// Return whether this animator is ready and has joints and animations loaded into it.
//
// Returns true if the animation is ready to be played.
bool PAnim::GetReady()
{
	return Anim.GetReady();
}

// Load an animation into the current Animation variable.
//
// Returns true if the animation was loaded successfully, otherwise false.
bool PAnim::LoadAnimation(const char* AnimFilePath)
{
	// Open the file (mesh) in binary input mode.
	std::fstream file{ (PGameplayStatics::GetGameDirectory() + "Assets/" + AnimFilePath), std::ios_base::in | std::ios_base::binary };

	// Ensure the file opened correctly.
	if (file.is_open())
	{
		// Setup index count for the mesh.
		AnimClip	NewClip;
		BindPose	NewBind;

		int			NumFrames;
		int			NumBindJoints;

		file.read((char*)&NumBindJoints, sizeof(uint32_t));
		//NewBind.Joints.resize(NumBindJoints);

		for (unsigned int i = 0; i < NumBindJoints; ++i)
		{
			Joint NewJoint;

			file.read((char*)&NewJoint, sizeof(PAnim::Joint));

			NewBind.Joints.push_back(NewJoint);
		}

		Anim.GetBindPose() = NewBind;

		file.read((char*)&NewClip.Duration, sizeof(double));
		file.read((char*)&NumFrames, sizeof(uint32_t));

		for (unsigned int i = 0; i < NumFrames; ++i)
		{
			PAnim::Keyframe NewFrame;

			file.read((char*)&NewFrame.Time, sizeof(double));

			int NumJoints;
			file.read((char*)&NumJoints, sizeof(uint32_t));

			for (unsigned int j = 0; j < NumJoints; ++j)
			{
				PAnim::Joint NewJoint;

				file.read((char*)&NewJoint, sizeof(PAnim::Joint));

				NewFrame.Joints.push_back(NewJoint);
			}

			NewClip.Frames.push_back(NewFrame);
		}

		// Close the file.
		file.close();

		Anim.Set(AnimFilePath, NewClip);
	}
	else
	{
		return false;
	}

	return true;
}
