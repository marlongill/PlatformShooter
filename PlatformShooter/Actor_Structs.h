#pragma once
#include "Assets.h"
#include "Polygon.h"

namespace Actor
{
	struct CheckInfo
	{
		Assets::MapCell Cell;
		Poly::Polygon Poly;
	};

	struct Properties
	{
		bool OnGround = false;
		bool OnClimbable = false;
		bool CanJump = false;
		bool ApplyGravity = false;
		bool SolidVsEnvironment = true;
		bool SolidVsActors = true;
		bool SlidingDownSlope = false;
		float MaxSpeed = 0.0f;
	};
}