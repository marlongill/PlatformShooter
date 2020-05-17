#include "Actor_Animation.h"

namespace Actor
{
	AnimationControl::AnimationControl() {
		_active = false;
		_currentIndex = 0;
		_elapsedTime = 0.0;
		_frameTime = 0.0f;
	}

	void AnimationControl::SetAnimation(Actor::Animation anim, std::string animationName) {
		_animation = anim;
		_currentIndex = 0;
		_active = true;
		_frameTime = (1.0f / 60) * (60.0f / _animation.FPS);
		_currentAnimation = animationName;
	}

	// Getters
	int AnimationControl::GetCurrentSprite() { return _animation.Frames[_currentIndex]; };
	std::string AnimationControl::GetCurrentAnimation() { return _currentAnimation; }
	int AnimationControl::GetCurrentFrame() { return _currentIndex; }

	/*===================================================================
	  == Update the animation frame based on the global timer          ==
	  == Will return true if we have ended the current animation cycle ==
	  ===================================================================*/
	bool AnimationControl::Update(float timeElapsed) {
		if (_active) {
			_elapsedTime += timeElapsed;
			if (_elapsedTime >= _frameTime) {
				_elapsedTime -= _frameTime;
				_currentIndex++;
				if (_currentIndex >= _animation.Frames.size()) {
					if (_animation.Loop) {
						_currentIndex = 0;
						return false;
					}
					else {
						_currentIndex--;
						_active = false;
						return true;
					}
				}
			}
			return false;
		}
		return false;
	}
}