#pragma once
#define SHOW_DEBUG_INFO

#include "olcPixelGameEngine.h"
#include "Assets.h"
#include "Actor_Player.h"
#include "Text.h"

namespace PlatformShooter
{
	class PlatformShooterDemo : public olc::PixelGameEngine
	{
	private:
		const float FPS_LIMIT = 60.0f;
		float ElapsedTime, RedrawTime, Gravity;
		int Cycle;

		enum Layers : uint8_t { LYR_TEXT = 0, LYR_PROJECTILE, LYR_ACTOR, LYR_ENVIRONMENT, LYR_BACKGROUND };
		Assets::Map* CurrentMap;
		Text::TextRenderer Text;

		Actor::Player* Player;

	public:
		PlatformShooterDemo();
		~PlatformShooterDemo();

		bool OnUserCreate() override;
		bool OnUserUpdate(float fElapsedTime) override;
		bool OnUserDestroy() override;

		void RenderDebugInfo();
		bool LoadResources();
		void UpdateWorld(float elapsedTime);
		void Render();
	};
}