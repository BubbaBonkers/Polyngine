#pragma once

#include <vector>
#include <string>

#include "../../../PStatics/PGameplayStatics/PGameplayStatics.h"

using namespace PMath;

class PAnim
{
public:
	// A joint describing this joint's attachment in relation to the rest of the skeleton.
	struct Joint
	{
		float Transform[16];
		int ParentIndex;

		bool operator==(const Joint& Input) const
		{
			return ((Transform == Input.Transform) && (ParentIndex == Input.ParentIndex));
		}
	};

	// A keyframe is a single frame within an animation.
	struct Keyframe
	{
		double Time;
		std::vector<PAnim::Joint> Joints;

		bool operator==(const Keyframe& Input) const
		{
			return (PMath::IsNearlyEqual(Time, Input.Time, 0.01f) && (Joints == Input.Joints));
		}
	};

	// An AnimClip is a set of keyframes that make up an animation.
	struct AnimClip
	{
		double Duration;
		std::vector<PAnim::Keyframe> Frames;

		AnimClip()
		{

		}

		AnimClip(const AnimClip& Input)
		{
			Duration = Input.Duration;
			Frames.resize(Input.Frames.size());
			Frames.assign(&Input.Frames.front(), &Input.Frames.back() + 1);//Frames = Input.Frames;
		}

		AnimClip& operator=(const AnimClip& Input)
		{
			Duration = Input.Duration;

			Frames.resize(Input.Frames.size());
			Frames.assign(&Input.Frames.front(), &Input.Frames.back());

			return *this;
		}
	};

	// A bind pose holds the Joint information for a bind pose.
	struct BindPose
	{
		std::vector<PAnim::Joint> Joints;

		BindPose() {}

		BindPose(std::vector<PAnim::Joint> InJoints)
		{
			Joints = InJoints;
		}

		BindPose& operator=(const BindPose& Input)
		{
			Joints.resize(Input.Joints.size());
			Joints.assign(&Input.Joints.front(), &Input.Joints.back() + 1);

			return *this;
		}
	};

	// An animation holds an AnimClip, and extra information about it.
	struct Animation
	{
	private:
		// Information about the current animation.
		//

		std::string			FilePath = "";				// The location of the animation that is playing.
		PAnim::AnimClip		CurrentAnim;				// The current AnimClip that is loaded, holding the duration and frames.
		PAnim::BindPose		Bind;						// The current bind pose for the animation.

		// Information about the current animation clip.
		//

		bool				bReady = false;				// Whether this animation is ready for playback controls.
		bool				bPaused = true;				// Whether the animation should be paused or resume playback.

	public:
		// Information about the current animation clip.
		//

		double				Time = 0.0;					// The current time in the animation playback.

		// Setup a new animation to play through this Animation container.
		//
		// Filepath and AnimClip inputs are required.
		void Set(std::string File, PAnim::AnimClip Clip, double CurrTime = 0.0f, bool bShouldPause = true)
		{
			FilePath = File;
			CurrentAnim = Clip;
			Time = CurrTime;
			bPaused = bShouldPause;

			if (FilePath != "")
			{
				bReady = true;
			}
		}


		// ------------------------------------------------------------------
		//		Getters for the Animation Information.
		// ------------------------------------------------------------------

		// Returns whether this component is ready to play animations.
		bool GetReady()
		{
			return (FilePath != "" && bReady);
		}

		// Returns the filepath of the loaded animation.
		std::string GetFile()
		{
			return FilePath;
		}

		// Returns the current clip of this animation.
		AnimClip GetClip()
		{
			return CurrentAnim;
		}

		// Returns the frame number for a given Keyframe object. Returns -1 if not found.
		// This is broken.
		int GetFrame(Keyframe InFrame)
		{
			for (unsigned int i = 0; i < CurrentAnim.Frames.size(); ++i)
			{
				if (CurrentAnim.Frames[i] == InFrame)
				{
					return i;
				}
			}

			return -1;
		}

		// Returns the current frame number of this animation based on the current playtime.
		unsigned int GetFrameAtTime()
		{
			return (int)(fclamp(((float)Time / (float)CurrentAnim.Duration) * (float)CurrentAnim.Frames.size(), 0.0f, (float)CurrentAnim.Frames.size() - 1));
		}

		// Returns the current frame number of this animation based on the input pTime.
		unsigned int GetFrameAtTime(float pTime)
		{
			return (int)(fclamp(floorf((pTime / (float)CurrentAnim.Duration) * (float)CurrentAnim.Frames.size()), 0.0f, (float)CurrentAnim.Frames.size() - 1));
		}

		// Returns the current timestamp of this animation based on the input Frame.
		float GetTimeAtFrame(int Frame)
		{
			return (float)(fclamp(((float)Frame / (float)(GetClip().Frames.size() - 1)) * (float)GetDuration(), 0.0f, (float)GetDuration()));
		}

		// Jump the current frame ahead or behind by Num number of frames, if possible.
		void FrameJump(int Num)
		{
			if (GetReady())
			{
				unsigned int CurrFrame = GetFrameAtTime();
				unsigned int NextFrame = CurrFrame + Num;

				if (((NextFrame) <= CurrentAnim.Frames.size()) && ((NextFrame) >= 0))
				{
					Time = CurrentAnim.Frames[NextFrame].Time;
				}
			}
		}

		// Set the current frame of animation to clip number Frame.
		void FrameSet(int Frame)
		{
			if (GetReady() && (Frame <= CurrentAnim.Frames.size()) && (Frame >= 0))
			{
				Time = ((Frame / CurrentAnim.Frames.size()) * GetDuration());
			}
		}

		// Set the current time to this keyframe, if an animation exists.
		void TimeSet(float pTime)
		{
			if (GetReady() && (pTime <= CurrentAnim.Frames[CurrentAnim.Frames.size() - 1].Time) && (pTime >= 0))
			{
				Time = (fclamp(pTime, 0.0f, GetDuration()));
			}
		}

		// Returns the duration of the loaded animation.
		double GetDuration()
		{
			if (FilePath != "")
			{
				return CurrentAnim.Duration;
			}
			else
			{
				return 0.0;
			}
		}

		// Returns the current timestamp of the animation.
		double GetTime()
		{
			if (FilePath != "")
			{
				return Time;
			}
			else
			{
				return 0.0;
			}
		}

		// Returns whether the animation clip is paused or not.
		double GetPaused()
		{
			return bPaused;
		}

		// Returns the currently loaded Bind Pose.
		PAnim::BindPose GetBindPose() const
		{
			return Bind;
		}

		// Returns the currently loaded Bind Pose.
		PAnim::BindPose& GetBindPose()
		{
			return Bind;
		}


		// ------------------------------------------------------------------
		//		Animation Playback Controls.
		// ------------------------------------------------------------------

		// Play or resume the loaded animation, if there is one.
		void Play()
		{
			if (bReady)
			{
				bPaused = false;
			}
		}

		// Pause playback of the loaded animation, if there is one.
		void Pause()
		{
			if (bReady)
			{
				bPaused = true;
			}
		}
	};

	// This is the current animation's information. It holds not only the loaded frames for animation, but also information about the animation that is currently loaded, if one is.
	Animation Anim;


	// ------------------------------------------------------------------
	//		Animation Playback Controls.
	// ------------------------------------------------------------------

	// Update should be called once per frame change.
	void Update(float DeltaTime);

	// Play an animation if it exists at the input file path. If no path is supplied, it will attempt to play any loaded animation.
	//
	// Returns true if the animation was loaded and played, false otherwise.
	bool Play(const char* AnimFilePath = "");

	// Pause an animation if it exists at the input file path.
	//
	// Returns true if the animation was able to pause, false otherwise.
	bool Pause();

	// Get whether the current animation is being played or not.
	//
	// Returns true if the animation clip loaded is playing.
	bool GetPaused();

	// Get the bind pose that is currently loaded in.
	//
	// Returns the current bind pose.
	PAnim::BindPose GetBindPose();

	// Get the total duration of the currently loaded animation, if any.
	//
	// Returns the duration in the form of a double.
	double GetDuration();

	// Return the current Keyframe that the animation is on.
	//
	// Returns a keyframe struct.
	Keyframe GetKeyframe();

	// Return the Keyframe that is at InTime time.
	//
	// Returns a keyframe struct.
	Keyframe GetKeyframe(float InTime);

	// Return the keyframe from the current animation at index Frame.
	//
	// Returns a keyframe struct.
	Keyframe GetKeyframeAt(int FrameNum);

	// Return the next Keyframe that the animation is will move to.
	//
	// Returns a keyframe struct.
	Keyframe GetNextKeyframe();

	// Return the next Keyframe that the animation is will move to from StartFrame.
	//
	// Returns a keyframe struct.
	PAnim::Keyframe GetNextKeyframe(PAnim::Keyframe StartFrame);

	// Returns a keyframe containing animation information for a blended frame between 
	// two other frames. Essentially blends two keyframes that exist into a new keyframe 
	// containing a new frame that would exist between frame A and B. CheckTime is the 
	// time to get the blended frame for. Leave CheckTime at -1 to use the current time.
	Keyframe GetLerpKeyframe(float CheckTime = -1.0f);

	// Jump the current frame ahead or behind by Num number of frames, if possible.
	//
	// No return value.
	void FrameJump(int Num);

	// Set the current time to this keyframe, if an animation exists.
	//
	// No return value.
	void FrameSet(int Frame);

	// Set the current time to this keyframe, if an animation exists.
	//
	// No return value.
	void TimeSet(float pTime);

	// Get the filepath for the currently loaded animation.
	//
	// Returns the filepath.
	std::string GetFile();

	// Return whether this animator is ready and has joints and animations loaded into it.
	//
	// Returns true if the animation is ready to be played.
	bool GetReady();

private:
	// Load an animation into the current Animation variable.
	//
	// Returns true if the animation was loaded successfully, otherwise false.
	bool LoadAnimation(const char* AnimFilePath);
};
