#pragma once
#include "Actor.h"

namespace Actor
{
	enum class FacingDirection { LEFT, RIGHT };
	enum class RotationPoint { BOTTOMLEFT, BOTTOMCENTRE, BOTTOMRIGHT };

	struct CellUnderPlayer
	{
		Assets::MapCell cell;
		Poly::line2d topEdge;
	};

	struct CellBesidePlayer
	{
		Assets::MapCell cell;
		Poly::line2d verticalEdge;
	};

	// Player Definition
	class Player : public ActorBase
	{
	private:
		Poly::Polygon _polyBelow;
		Poly::line2d _edgeBelow;
		FacingDirection _direction = FacingDirection::RIGHT;
		RotationPoint rotationPoint;

	private:
		bool TooSteep(bool left, Poly::line2d line);

	public:
		// MOVE BELOW SHIT TO PRIVATE AFTER TEST
		bool _acceptXInput = true; // Flag to determine whether x input is read. Is set when colliding with vertical face in mid air

		CellUnderPlayer _cellBelow;

		Player();
		Player(std::string decalName, olc::PixelGameEngine* p, olc::Pixel tint = olc::WHITE);
		~Player();

		void SetMaxSpeed(float s);

		void Update(Assets::Map* map, float timeElapsed) override;
		void Render(Assets::Map* map) override;

		void Jump();
		void Land();
		void Float();

		void HandleInput(float timeElapsed);

		void CheckAboveCharacter(Assets::Map* map);
		void CheckBelowCharacter(Assets::Map* map);
		void CheckHorizontalCollisions(Assets::Map* map);
		void HandleCollisions(Assets::Map* map, float timeElapsed);

	};
}