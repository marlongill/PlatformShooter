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

	void Enemy::Update(Assets::Map* map, float timeElapsed)
	{
		ActorBase::InternalUpdate(timeElapsed);
	}

	void Enemy::Render(Assets::Map* map)
	{
		ActorBase::InternalRender(map);
	}

}
