#include "Actor.h"

namespace Actor
{
	Projectile::Projectile() {}

	Projectile::Projectile(std::string decalName, olc::PixelGameEngine* p, olc::Pixel tint) : ActorBase(decalName, p, tint)
	{ 
	}

	Projectile::~Projectile()
	{
	}

	bool Projectile::Update(Assets::Map* map, float timeElapsed)
	{
		return ActorBase::InternalUpdate(timeElapsed);
	}
	
	void Projectile::Render(Assets::Map* map)
	{
		ActorBase::InternalRender(map);
	}
}
