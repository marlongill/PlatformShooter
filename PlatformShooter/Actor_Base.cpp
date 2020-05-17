#define SHOW_WIRE_FRAME

#include "Actor.h"

#include <cmath>

namespace Actor
{
	ActorBase::ActorBase() {
		_pge = nullptr;
		_spriteSheet = nullptr;
		_animation = new AnimationControl();
		_props = new Properties();
		_rotation = 0;
		_rotationAxis = { 0, 0 };
	}

	ActorBase::ActorBase(std::string decalName, olc::PixelGameEngine* p, olc::Pixel tint) {
		_tint = tint; 
		_pge = p;
		_props = new Properties();
		_spriteData = Assets::Assets::get().GetSpriteData(decalName);
		_animation = new AnimationControl();
		_rotation = 0;
		_rotationAxis = { 0, 0 };

		_spriteWidth = _spriteData->TileWidth; 
		_spriteHeight = _spriteData->TileHeight;
		_spriteSheet = _spriteData->Decal;
	}

	ActorBase::~ActorBase() {
		delete _animation;
		delete _props;
	}

	// Getters 
	olc::vi2d ActorBase::GetWorldCoords() { return olc::vi2d((int)floor(_fineCoords.x), (int)floor(_fineCoords.y)); }
	olc::vf2d ActorBase::GetVelocity() { return _velocity; }
	AnimationControl* ActorBase::GetAnimation() { return _animation; }
	Properties* ActorBase::GetProperties() { return _props;	}
	olc::vi2d ActorBase::GetSpriteSize() { return { _spriteWidth, _spriteHeight }; }

	// Setters
	void ActorBase::SetWorldCoords(olc::vi2d coords) { _fineCoords = coords; }
	void ActorBase::SetVelocity(olc::vf2d velocity) { _velocity = velocity; }
	void ActorBase::SetAnimation(std::string animationName) {
		if (_animation->GetCurrentAnimation() != animationName)
			_animation->SetAnimation(_spriteData->Animations[animationName], animationName);
	}
	void ActorBase::SetProperties(Properties* properties) { _props = properties; }
	void ActorBase::SetRotation(float rotation) { _rotation = rotation; }
	void ActorBase::SetRotation(float rotation, olc::vf2d axis)
	{
		_rotation = rotation; _rotationAxis = axis;
	}

	// Member Related Methods
	void ActorBase::Activate() { _active = true; }
	void ActorBase::Deactivate() { _active = false; }
	void ActorBase::UpdateCoords(olc::vf2d delta) { _fineCoords += delta; }

	// Internal Methods 
	void ActorBase::InternalUpdate(float timeElapsed)
	{
		_animation->Update(timeElapsed);
		if (_velocity.x != 0.0 || _velocity.y != 0.0) {
			_fineCoords += _velocity;
		}
	}

	void ActorBase::InternalRender(Assets::Map* map)
	{
		if (_active)
		{
			// Render decal to screen
			olc::vi2d screenCoords = GetWorldCoords() - map->Origin;
			if (screenCoords.x > -_spriteWidth || screenCoords.x < _pge->ScreenWidth() + _spriteWidth
				|| screenCoords.y > -_spriteHeight || screenCoords.y < _pge->ScreenHeight() + _spriteHeight)
			{
				int currentSprite = _animation->GetCurrentSprite();
				int dx = (currentSprite % _spriteData->SpritesPerRow) * _spriteWidth;
				int dy = (currentSprite / _spriteData->SpritesPerRow) * _spriteHeight;
				if (_rotation == 0)
				{
					_pge->DrawPartialDecal(
						olc::vi2d(screenCoords.x, screenCoords.y),
						_spriteSheet,
						olc::vi2d(dx, dy),
						olc::vi2d(_spriteWidth, _spriteHeight),
						{ 1.0f, 1.0f },
						_tint
					);
				}
				else
				{
					Poly::Polygon actorPoly = _spriteData->PolyMasks[currentSprite];
					
					_pge->DrawPartialRotatedDecal(
						{ screenCoords.x + _rotationAxis.x,  screenCoords.y + _rotationAxis.y }, //olc::vi2d(_rotationAxis.x - map->Origin.x,,
						_spriteSheet,
						_rotation,
						{ _rotationAxis.x, _rotationAxis.y }, 
						olc::vi2d(dx, dy),
						olc::vi2d(_spriteWidth, _spriteHeight),
						{ 1.0f, 1.0f },
						_tint
					);

#ifdef SHOW_WIRE_FRAME
					_pge->SetDrawTarget((uint8_t)0);
					_pge->DrawLine(
						{ (int)_rotationAxis.x + screenCoords.x - 10, (int)_rotationAxis.y + screenCoords.y },
						{ (int)_rotationAxis.x + screenCoords.x + 10, (int)_rotationAxis.y + screenCoords.y },
						olc::MAGENTA
					);
					_pge->DrawLine(
						{ (int)_rotationAxis.x + screenCoords.x, (int)_rotationAxis.y + screenCoords.y - 10 },
						{ (int)_rotationAxis.x + screenCoords.x, (int)_rotationAxis.y + screenCoords.y + 10 },
						olc::MAGENTA
					);
#endif
				}

#ifdef SHOW_WIRE_FRAME
				_pge->SetDrawTarget((uint8_t)0);
				Poly::Polygon p = _spriteData->PolyMasks[currentSprite];
				p.SetPosition(screenCoords.x, screenCoords.y, _rotation, { _rotationAxis.x, _rotationAxis.y });
				for (int i = 0; i < p.GetEdgeCount(); i++)
				{
					Poly::Edge face = p.GetEdge(i, Poly::COORDTYPE::ROTATED);
					_pge->DrawLine(
						olc::vi2d(face.line.start.x, face.line.start.y) ,
						olc::vi2d(face.line.end.x, face.line.end.y) ,
						olc::RED
					);
				}
#endif
			}
		}
	}

	olc::vf2d ActorBase::ApplyGravity(Assets::Map* map, float timeElapsed)
	{ 
		if (_props->ApplyGravity && (!_props->OnGround || _props->SlidingDownSlope))
			_velocity.y += map->Gravity * timeElapsed;
		return _velocity;
	}

	void ActorBase::Animate(float timeElapsed) {
		_animation->Update(timeElapsed);
	}

	int ActorBase::CheckFaceCollisions(Assets::Map* map, Poly::Polygon* playerPoly, olc::vf2d worldCoords, Poly::FACE face, float elapsedTime)
	{
		// Check for collisions in stages to prevent overshooting the target
		// Work out the number of iterations
		bool horizontal = (face == Poly::FACE::LEFT || face == Poly::FACE::RIGHT);
		float velocity = horizontal ? _velocity.x : _velocity.y;
		int steps = std::max((int)(velocity / 0.1), 1);
		float dv = velocity / steps;
		float vx, vy;

		if (horizontal) {
			vx = dv; vy = 0;
		}
		else {
			vx = 0; vy = dv;
		}

		int collisions = 0;
		int iteration = 1;
		bool resolved = false;

		while (iteration <= steps && !resolved) {
			// Where are we on this iteration
			olc::vi2d destCoord = { (int)floor(worldCoords.x + (vx * iteration)), (int)floor(worldCoords.y + (vy * iteration)) };
			olc::vi2d destMapRef = map->WorldCoordToMapRef(destCoord);
			playerPoly->UpdatePosition(vx, vy);

			// Determine cells to check
			std::vector<CheckInfo> checkCells;
			olc::vi2d checkMapRef;
			if (vx < 0) {
				for (int x = 0; x <= 1; x++)
				{
					for (int y = 0; y <= (destCoord.y % map->TileHeight <= 1 ? 0 : 1); y++)
					{
						checkMapRef = olc::vi2d(destMapRef.x - x, destMapRef.y + y);
						Assets::MapCell cell = map->GetTileInfo(checkMapRef);
						if (cell.IsSolid) {
							Poly::Polygon checkPoly = map->GetPolygon(checkMapRef);
							if (playerPoly->BoundingBoxOverlap(&checkPoly))
								checkCells.push_back({ cell, checkPoly });
						}
					}
				}
			}
			else if (vx > 0) {
				for (int x = 0; x <= 1; x++)
				{
					for (int y = 0; y <= (destCoord.y % map->TileHeight <= 1 ? 0 : 1); y++)
					{
						checkMapRef = olc::vi2d(destMapRef.x + x, destMapRef.y + y);
						Assets::MapCell cell = map->GetTileInfo(checkMapRef);
						if (cell.IsSolid) {
							Poly::Polygon checkPoly = map->GetPolygon(checkMapRef);
							if (playerPoly->BoundingBoxOverlap(&checkPoly))
								checkCells.push_back({ cell, checkPoly });
						}
					}
				}
			}
			else if (vy > 0) {
				for (int i = 0; i <= 1; i++)
				{
					checkMapRef = olc::vi2d(destMapRef.x + i, destMapRef.y + 1);
					Assets::MapCell cell = map->GetTileInfo(checkMapRef);
					if (cell.IsSolid) {
						Poly::Polygon checkPoly = map->GetPolygon(checkMapRef);
						if (playerPoly->BoundingBoxOverlap(&checkPoly))
							checkCells.push_back({ cell, checkPoly });
					}
				}
			}
			else if (vy < 0) {
				for (int i = 0; i <= 1; i++)
				{
					checkMapRef = olc::vi2d(destMapRef.x + i, destMapRef.y);
					Assets::MapCell cell = map->GetTileInfo(checkMapRef);
					if (cell.IsSolid) {
						Poly::Polygon checkPoly = map->GetPolygon(checkMapRef);
						if (playerPoly->BoundingBoxOverlap(&checkPoly))
							checkCells.push_back({ cell, checkPoly });
					}
				}
			}

			for (auto c : checkCells)
			{
				std::vector<Poly::Intersect> result = playerPoly->IntersectingFaces(&c.Poly, face)[(int)face];

				if (result.size() > 0) {

					collisions |= (int)pow(2, (int)face);

					bool applyX = true;

					if (face == Poly::FACE::RIGHT && result[0].face.line.m >= -1.1 && result[0].face.line.m <= -0.4) {
						collisions -= (int)Poly::FACE::RIGHT;
						applyX = false;
					}
					if (face == Poly::FACE::LEFT && result[0].face.line.m >= 0.4 && result[0].face.line.m <= 1.1) {
						collisions -= (int)Poly::FACE::LEFT;
						applyX = false;
					}

					_velocity.x = horizontal && applyX ? vx * (iteration - 1) : _velocity.x;
					_velocity.y = horizontal ? _velocity.y : vy * (iteration - 1);
					
					resolved = true;
					break;
				}
			}

			iteration++;
		}

		return collisions;
	}

	int ActorBase::CheckBackgroundCollision(Assets::Map* map, float elapsedTime)
	{
		// Where are we now?
		olc::vi2d worldCoords = GetWorldCoords();
		olc::vi2d srcMapRef = map->WorldCoordToMapRef(worldCoords);

		olc::vi2d destCoord;
		olc::vi2d destMapRef;
		std::vector<Poly::Polygon> checkPolys;
		float dx, dy;

		// Get Polygon for current animation frame
		Poly::Polygon playerPoly = _spriteData->PolyMasks[_animation->GetCurrentSprite()];
		playerPoly.SetPosition(worldCoords.x + map->TileWidth / 2, worldCoords.y + map->TileHeight / 2);

		// Where are we going to be after next update
		destCoord = { (int)floor(worldCoords.x + _velocity.y), (int)floor(worldCoords.y + _velocity.y) };
		destMapRef = map->WorldCoordToMapRef(destCoord);
		playerPoly.UpdatePosition(_velocity.x, _velocity.y);

		// Build vector of background cells to check, bounding box rough check used at this stage
		// to give the final list of cells to check.
		int collisions = 0;
		if (_velocity.x > 0) collisions |= CheckFaceCollisions(map, &playerPoly, worldCoords, Poly::FACE::RIGHT, elapsedTime);
		if (_velocity.x < 0) collisions |= CheckFaceCollisions(map, &playerPoly, worldCoords, Poly::FACE::LEFT, elapsedTime);
		//if (_velocity.y > 0) collisions |= CheckFaceCollisions(map, &playerPoly, worldCoords, Poly::FACE::BOTTOM, elapsedTime);
		//if (_velocity.y < 0) collisions |= CheckFaceCollisions(map, &playerPoly, worldCoords, Poly::FACE::TOP, elapsedTime);

		return collisions;
	}

}
