#include "Actor.h"

namespace Actor
{
	// Enemy Implementation
	Enemy::Enemy() {}

	Enemy::Enemy(std::string decalName, olc::PixelGameEngine* p, olc::Pixel tint) : ActorBase(decalName, p, tint)
	{ 
	}

	Enemy::~Enemy()
	{
	}

	bool Enemy::Update(Assets::Map* map, float timeElapsed)
	{
		return ActorBase::InternalUpdate(timeElapsed);
	}

	void Enemy::Render(Assets::Map* map)
	{
		ActorBase::InternalRender(map);
	}

}
