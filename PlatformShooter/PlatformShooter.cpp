#define DEBUG

#include "PlatformShooter.h"

#include <iostream>
#include <sstream>
#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

namespace PlatformShooter
{
	PlatformShooterDemo::PlatformShooterDemo() {}
	PlatformShooterDemo::~PlatformShooterDemo() {}

	float slowDownTimer = 1.0f;

	bool PlatformShooterDemo::OnUserCreate()
	{
		sAppName = "Platform Shooter Demo";
		// FPS Limiter
		Gravity = 4.0f; // One cell is 1m

		// Default Layer is Text
		SetDrawTarget(LYR_TEXT);
		Clear(olc::BLANK);

		// Projectiles and Explosions Layer
		CreateLayer(); EnableLayer(LYR_PROJECTILE, true);
		SetDrawTarget(LYR_PROJECTILE); Clear(olc::BLANK);

		// Actors Layer
		CreateLayer(); EnableLayer(LYR_ACTOR, true);
		SetDrawTarget(LYR_ACTOR); Clear(olc::BLANK);

		// Environment Layer
		CreateLayer(); EnableLayer(LYR_ENVIRONMENT, true);
		SetDrawTarget(LYR_ENVIRONMENT); Clear(olc::BLANK);

		// Background Layer
		CreateLayer(); EnableLayer(LYR_BACKGROUND, true); 
		SetDrawTarget(LYR_BACKGROUND); Clear(olc::BLANK);

		LoadResources();

		// Text Renderer 
		Text = Text::TextRenderer(32, 111, "Typeface", this);

		// Variable Initialisation
		CurrentMap = Assets::Assets::get().GetMap("TestMap");
		CurrentMap->SetGravity(Gravity);

		Player = new Actor::Player("Player2", this, olc::WHITE);

		Player->SetAnimation("Idle Right");
		Player->Activate();
		Player->SetWorldCoords(CurrentMap->MapRefToWorldCoord(olc::vi2d(0, 30)));
		Player->SetMaxSpeed(0.75f);
		Actor::Properties* props = Player->GetProperties();
		props->ApplyGravity = true;
		Player->SetProperties(props);

		return true;
	}

	bool PlatformShooterDemo::OnUserUpdate(float fElapsedTime)
	{
		//slowDownTimer -= fElapsedTime;
		//if (slowDownTimer < 0)
		//{
		//	slowDownTimer += 0.1f;
		//	UpdateWorld(fElapsedTime);
		//}
		//else
		//{
		//	Player->HandleInput(fElapsedTime);
		//}
		UpdateWorld(fElapsedTime);
		Render();
		return true;
	}

	bool PlatformShooterDemo::OnUserDestroy()
	{
		delete CurrentMap;
		delete Player;
		return true;
	}

	void PlatformShooterDemo::RenderDebugInfo()
	{
		olc::vi2d WorldCoords = Player->GetWorldCoords();
		olc::vf2d Velocity = Player->GetVelocity();
		Actor::Properties* Props = Player->GetProperties();

		char textString[128];
		snprintf(textString, 128, "PLAYER:    %d, %d\nVELOCITY:  %.2f, %.2f\nWORLD:     %d, %d\nFPS:       %d"
			, WorldCoords.x, WorldCoords.y, Velocity.x, Velocity.y, CurrentMap->Origin.x, CurrentMap->Origin.y, GetFPS());
		Text.RenderString(0, 0, textString, 0.25f);

		Assets::MapCell cell = CurrentMap->GetTileInfoWorldCoord(WorldCoords);
		snprintf(textString, 128, "CELL:      T=%d, S=%d, D=%d, F=%.2f", cell.TileID, cell.IsSolid, cell.Damage, cell.Friction);
		Text.RenderString(0, 32, textString, 0.25f);

		snprintf(textString, 128, "FLAGS:     OG=%d, CJ=%d, TB=%d, SDS=%d, XIN=%d", 
			Props->OnGround, Props->CanJump, Player->_cellBelow.cell.TileID, Props->SlidingDownSlope, Player->_acceptXInput);
		Text.RenderString(0, 40, textString, 0.25f);

		Actor::AnimationControl* anim = Player->GetAnimation();

		std::string name = anim->GetCurrentAnimation();

		snprintf(textString, 128, "ANIMATION: F=%d, S: %d, N: %s"
			, anim->GetCurrentFrame(), anim->GetCurrentSprite(), name.c_str());
		Text.RenderString(0, 48, textString, 0.25f);

	}

	void PlatformShooterDemo::Render()
	{
		CurrentMap->Render(LYR_ENVIRONMENT, true);

		SetDrawTarget(LYR_ACTOR);
		Player->Render(CurrentMap);

#ifdef SHOW_DEBUG_INFO
		SetDrawTarget(LYR_TEXT);
		RenderDebugInfo();
#endif
	}

	void PlatformShooterDemo::UpdateWorld(float timeElapsed)
	{
		SetDrawTarget(LYR_TEXT);
		memset(GetDrawTarget()->GetData(), 0, GetDrawTargetWidth() * GetDrawTargetHeight() * sizeof(olc::Pixel));

		Player->HandleInput(timeElapsed);
		
#ifdef DEBUG
		if (GetMouse(0).bPressed) {
			int x = floor((float)(GetMouseX() + CurrentMap->Origin.x) / CurrentMap->TileWidth);
			int y = floor((float)(GetMouseY() + CurrentMap->Origin.y) / CurrentMap->TileHeight);

			Player->SetWorldCoords(CurrentMap->MapRefToWorldCoord(olc::vi2d(x, y)));
			Player->SetRotation(0);
			Player->Float();
		}
#endif

		Player->ApplyGravity(CurrentMap, timeElapsed);
		Player->HandleCollisions(CurrentMap, timeElapsed);

		// Update the player in the world
		Player->Update(CurrentMap, timeElapsed);
		
		// Update Viewport
		CurrentMap->CenterOnPoint(Player->GetWorldCoords());
	}
}