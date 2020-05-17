#pragma once

#include <vector>
#include <map>
#include <string>

namespace Actor
{
	class Animation
	{
	public:
		int FPS;
		bool Loop;
		std::vector<int> Frames;
	};

	class AnimationControl
	{
	private:
		int _currentIndex;
		bool _active;
		float _frameTime, _elapsedTime;
		Actor::Animation _animation;
		std::string _currentAnimation;

	public:
		AnimationControl();

		// Setters
		void SetAnimation(Actor::Animation anim, std::string animationName);

		// Getters
		int GetCurrentSprite();
		int GetCurrentFrame();
		std::string GetCurrentAnimation();

		// Methods
		bool Update(float timeElapsed);
	};
}