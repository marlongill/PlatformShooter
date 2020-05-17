#pragma once
#include "olcPixelGameEngine.h"
#include "Assets.h"
#include "Actor_Animation.h"
#include "Actor_Structs.h"

namespace Actor
{
	// Base 
	class ActorBase
	{
	/*=========================================================
	  === Base Class for all Objects in the Game             ==
	  =========================================================*/
	protected:
		Assets::SpriteData* _spriteData = nullptr;
		olc::Decal* _spriteSheet = nullptr;
		AnimationControl* _animation = nullptr;
		Properties* _props = nullptr;

		olc::vf2d _velocity = { 0.0f, 0.0f };
		olc::vf2d _fineCoords = { 0.0f, 0.0f };
		float _rotation;
		olc::vf2d _rotationAxis = { 0.0f, 0.0f };

		bool _active = false; // Do not render if false!

		int _spriteWidth = 32;
		int _spriteHeight = 32;

		olc::PixelGameEngine* _pge;
		olc::Pixel _tint;

		Assets::TileAttributes _cellBelow;

	// Getters
	public:
		olc::vi2d GetWorldCoords();
		olc::vf2d GetVelocity();
		AnimationControl* GetAnimation();
		Properties* GetProperties();
		olc::vi2d GetSpriteSize();

	// Setters
	public:
		void SetWorldCoords(olc::vi2d coords);
		void SetVelocity(olc::vf2d velocity);
		void SetAnimation(std::string animationName);
		void SetProperties(Properties* properties);
		void SetRotation(float rotation);
		void SetRotation(float rotation, olc::vf2d axis);

	// Member Related Methods
	public:
		void Activate();
		void Deactivate();
		void UpdateCoords(olc::vf2d delta);

	// Internal Methods
	protected:
		void InternalUpdate(float timeElapsed);
		void InternalRender(Assets::Map* map);

	// Public Methods
	public:
		ActorBase();
		ActorBase(std::string decalName, olc::PixelGameEngine* p, olc::Pixel tint = olc::WHITE);
		~ActorBase();

		olc::vf2d ApplyGravity(Assets::Map* map, float elapsedTime);

		void Animate(float timeElapsed);

		virtual void Update(Assets::Map* map, float timeElapsed) = 0;
		virtual void Render(Assets::Map* map) = 0;

		int CheckFaceCollisions(Assets::Map* map, Poly::Polygon* playerPoly, olc::vf2d worldCoords, Poly::FACE face, float elapsedTime);
		int CheckBackgroundCollision(Assets::Map* map, float elapsedTime);
	};

	// Enemy Definition
	class Enemy : public ActorBase
	{
	public:
		Enemy();
		Enemy(std::string decalName, olc::PixelGameEngine* p, olc::Pixel tint = olc::WHITE);
		~Enemy();
		void Update(Assets::Map* map, float timeElapsed) override;
		void Render(Assets::Map* map) override;
	};
	
	class Projectile : public ActorBase
	{
	public:
		Projectile();
		Projectile(std::string decalName, olc::PixelGameEngine* p, olc::Pixel tint = olc::WHITE);
		~Projectile();
		void Update(Assets::Map* map, float timeElapsed) override;
		void Render(Assets::Map* map) override;
	};
}
