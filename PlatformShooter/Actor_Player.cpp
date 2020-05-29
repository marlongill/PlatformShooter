#define SHOW_COLLISION_DATA

#include "Actor_Player.h"

namespace Actor
{
	Player::Player() { }

	Player::Player(std::string decalName, olc::PixelGameEngine* p, olc::Pixel tint) : ActorBase(decalName, p, tint)
	{ 
	}

	// Player Implementation
	Player::~Player()
	{
	}

	bool Player::Update(Assets::Map* map, float timeElapsed)
	{
		bool checkCollisions = InternalUpdate(timeElapsed);

		if (_fineCoords.x < 0) { _fineCoords.x = 0; _velocity.x = 0; }
		if (_fineCoords.x > map->WorldWidth() - _spriteWidth) { _fineCoords.x = map->WorldWidth() - _spriteWidth; _velocity.x = 0; }
		if (_fineCoords.y < 0) { _fineCoords.y = 0; _velocity.y = 0; }
		if (_fineCoords.y > map->WorldHeight() - _spriteHeight) { _fineCoords.y = map->WorldHeight() - _spriteHeight; _velocity.y = 0; }

		return checkCollisions;
	}
	
	void Player::Render(Assets::Map* map)
	{
		ActorBase::InternalRender(map);
	}

	void Player::SetMaxSpeed(float s) { _props->MaxSpeed = s; }

	void Player::Jump(float timeElapsed)
	{
		if (_props->CanJump) {
			_state = PlayerState::Jumping;
			_velocity.y = -4.5f;
			_props->CanJump = false;
			_props->OnGround = false;
			SetRotation(0);
			if (_direction == FacingDirection::RIGHT)
				SetAnimation("Jump Right");
			else
				SetAnimation("Jump Left");
		}
	}

	void Player::Land()
	{
		if (!_props->OnGround) {
			_props->OnGround = true;
			_props->CanJump = true;
			_velocity.y = 0;
			
			if (_velocity.x == 0) {
				SetAnimation(_direction == FacingDirection::LEFT ? "Idle Left" : "Idle Right");
				_state = PlayerState::Idle;
			}
			else if (_xSpeed == RUN_SPEED) {
				SetAnimation(_direction == FacingDirection::LEFT ? "Run Left" : "Run Right");
				_state = PlayerState::Running;
			}
			else if (_xSpeed == JOG_SPEED) {
				SetAnimation(_direction == FacingDirection::LEFT ? "Jog Left" : "Jog Right");
				_state = PlayerState::Jogging;
			}
			else if (_xSpeed == WALK_SPEED) {
				SetAnimation(_direction == FacingDirection::LEFT ? "Run Left" : "Run Right");
				_state = PlayerState::Walking;
			}
		}
	}

	void Player::Float()
	{
		_props->OnGround = false;
		_props->CanJump = false;
	}
	
	void Player::HandleInput(float timeElapsed)
	{
		if (_state == PlayerState::Jumping || _state == PlayerState::Falling)
		{
			_velocity.x = _xSpeed * timeElapsed * (_direction == FacingDirection::LEFT ? -1.0f : 1.0f);
			return;
		}

		_xInputHeld += timeElapsed;

		bool movementKeyPressed = false;

		bool left = _pge->GetKey(olc::LEFT).bHeld;
		bool right = _pge->GetKey(olc::RIGHT).bHeld;
		bool jump = _pge->GetKey(olc::SPACE).bHeld;

		if (left || right) {
			if (_acceptXInput && _props->OnGround) {

				if ((_direction == FacingDirection::RIGHT && left)
					|| (_direction == FacingDirection::LEFT && right)
					|| _velocity.x == 0) {

					_direction = left ? FacingDirection::LEFT : FacingDirection::RIGHT;
					_xInputHeld = 0.0;
					SetAnimation(left ? "Walk Left" : "Walk Right");
					_xSpeed = WALK_SPEED;
					_state = PlayerState::Walking;
				}

				if (_xInputHeld > 0.4f)
				{
					if (_xSpeed == WALK_SPEED) {
						_xInputHeld -= 0.4f;
						SetAnimation(_direction == FacingDirection::LEFT ? "Jog Left" : "Jog Right");
						_xSpeed = JOG_SPEED;
						_state = PlayerState::Jogging;
					}
					else if (_xSpeed = JOG_SPEED) {
						_xInputHeld -= 0.4f;
						SetAnimation(_direction == FacingDirection::LEFT ? "Run Left" : "Run Right");
						_xSpeed = RUN_SPEED;
					}
				}

				_velocity.x = _xSpeed * timeElapsed * (left ? -1.0f : 1.0f);
			}
			movementKeyPressed = true;
		}

		if (jump)
			Jump(timeElapsed);

		if (!(left || right || jump)) {

			// Reset x input blocker
			_acceptXInput = true;

			if (_velocity.x != 0) {
				// Round off our x velocity and stop if very, very small - Makes sure we don't constantly move
				if (_velocity.x != 0)
				{
					int ix = round(_velocity.x * 10000);
					if (ix == 0) {
						_velocity.x = 0;
						SetAnimation(_direction == FacingDirection::LEFT ? "Idle Left" : "Idle Right");
					}
				}
			}

			if (_velocity.y != 0) {
				int iy = round(_velocity.y * 10000);
				if (iy == 0) {
					_velocity.y = 0;
					SetAnimation(_direction == FacingDirection::LEFT ? "Idle Left" : "Idle Right");
				}
			}

			// Determine deceleration and apply to our x velocity
			if (_velocity.x != 0) {
				float deceleration;
				//if (!_props->OnGround)
				//	deceleration = (_xSpeed / 2) * timeElapsed;
				//else 
				if (abs(_cellBelow.topEdge.m) > 1.1)
					deceleration = 0.0f;
				else if (_cellBelow.cell.Friction == 1.0f) {
					deceleration = abs(_velocity.x);
					SetAnimation(_direction == FacingDirection::LEFT ? "Idle Left" : "Idle Right");
				}
				else
				{
					deceleration = (1.0f - _cellBelow.cell.Friction) * ((_xSpeed / 2) * timeElapsed);
				}

				if (_velocity.x < 0) {
					float v = _velocity.x + deceleration;
					if (v > 0) {
						_velocity.x = 0;
						_xSpeed = 0;
					}
					else {
						_velocity.x = v;
					}
				}
				else if (_velocity.x > 0)
				{
					float v = _velocity.x - deceleration;
					if (v < 0) {
						_velocity.x = 0;
						_xSpeed = 0;
					}
					else {
						_velocity.x = v;
					}
				}

				// Minimum velocity is based on the slope and friction of the cell below
				// us when we are not on level ground
				float minv = _props->MaxSpeed * _cellBelow.topEdge.m;
				minv *= (1.0f - _cellBelow.cell.Friction);

				if (_velocity.x == 0 && _props->OnGround)
					_velocity.x = minv;
				else if (_velocity.x < 0 && _props->OnGround && minv < 0)
					_velocity.x = std::min(minv, _velocity.x);
				else if (_velocity.x > 0 && _props->OnGround && minv > 0)
					_velocity.x = std::max(minv, _velocity.x);
			}
		}

		if (_pge->GetKey(olc::ESCAPE).bPressed)
		{
			_velocity = { 0, 0 };
			SetWorldCoords({ 1068, 1069 });
		}
	}

	bool Player::TooSteep(bool left, Poly::line2d line)
	{
		float m = line.m * (left ? 1.0f : -1.0f);
		return m > 1.1 || m == INFINITY || m == -INFINITY;
	}

	void Player::CheckAboveCharacter(Assets::Map* map)
	{
		if (_velocity.y >= 0)
			return;

		Poly::Polygon objPoly = _spriteData->PolyMasks[_animation->GetCurrentSprite()];
		objPoly.SetPosition(
			_velocity.x < 0 ? ceil(_fineCoords.x) : floor(_fineCoords.x),
			_fineCoords.y,
			_rotation,
			Poly::vec2d{ _rotationAxis.x, _rotationAxis.y }
		);

		std::vector<Poly::line2d> rays = objPoly.GetUpCheckRays(signbit(_velocity.x));
		std::vector<float> dy = { -100, -100, -100 };

		// Check intersection with these rays and the bottom faces of the target cells in the y direction
		Poly::line2d checkFace;
		int maxY = -INFINITY;
		for (int x = 0; x < 3; x++)
		{
			// Get Face of Check Polygon from Map Masks
			if (x == 0) {
				checkFace = map->GetIntersectingMaskFace(rays[0]);
			}
			else {
				if (rays[x].start.x < checkFace.start.x || rays[x].start.x > checkFace.end.x)
					checkFace = map->GetIntersectingMaskFace(rays[x]);
			}

			if (checkFace.start.x != -INFINITY)
			{
				// Get intersection between the current check ray and the bottom face of the target polygon
				try {
					Poly::vec2d ix = rays[x].Intersect(checkFace);

					// Intersect distance is the y coord of the intersection minus the midpoint of
					// our check polygon (which is the y coordinate of the bottom face)
					maxY = std::max(ix.y, (float)maxY);
					dy[x] = ix.y - rays[x].Midpoint().y;
				}
				catch (Poly::InvalidIntersect)
				{
					// Set the intersect distance for this ray to be out of bounds for our range checking
					dy[x] = -100;
				}
			}
		}

		if (dy[0] >= -1 || dy[1] >= -1 || dy[2] >= -1)
		{
			_velocity.y = 0;

			int fallingSpriteID = _spriteData->Animations[_direction == Actor::FacingDirection::LEFT ? "Fall Left" : "Fall Right"].Frames[0];
			Poly::Polygon adjustMask = _spriteData->PolyMasks[fallingSpriteID];
			adjustMask.SetPosition(objPoly.GetPosition().x, objPoly.GetPosition().y, 0.0f, { 0, 0 });
			Poly::Edge adjustEdge = adjustMask.GetEdge(Poly::FACE::TOP);

			_fineCoords.y = (adjustEdge.line.start.y - _fineCoords.y) + maxY;
		}
		else
			_fineCoords.y += _velocity.y;

#ifdef SHOW_COLLISION_DATA
		_pge->SetDrawTarget(nullptr);
		for (auto l : rays)
			_pge->DrawLine(
				{ (int)l.start.x - map->Origin.x, (int)l.start.y - map->Origin.y },
				{ (int)l.end.x - map->Origin.x, (int)l.end.y - map->Origin.y },
				olc::CYAN
			);
#endif

		rays.clear();
		dy.clear();
	}

	void Player::CheckBelowCharacter(Assets::Map* map)
	{
		_cellBelow = { Assets::MapCell(), {{-1, -1}, {-1,-1}} };
		_polyBelow = Poly::Polygon();
		_props->SlidingDownSlope = false;

		if (_velocity.y < 0) 
			return;

		// Create downward rays for checking collisions - Start by getting destination polygon
		// and then cast downwards from the left, right and center of bottom face
		Poly::Polygon objPoly = _spriteData->PolyMasks[_animation->GetCurrentSprite()];
		objPoly.SetPosition(
			_velocity.x < 0 ? ceil(_fineCoords.x) : floor(_fineCoords.x),
			_fineCoords.y ,
			_rotation ,
			Poly::vec2d{ _rotationAxis.x, _rotationAxis.y }
		);

		std::vector<Poly::line2d> rays = objPoly.GetDownCheckRays(signbit(_velocity.x));
		std::vector<Poly::vec2d> ix; ix.resize(3);
		std::vector<float> dy = { 100,100,100 };
		std::vector<CellUnderPlayer> cellBelow; cellBelow.resize(3);

		int intersectMask = 0;

		// Check intersection with these rays and the top faces of the target cells in the y direction
		for (int x = 0; x < 3; x++)
		{
			int cellX = (int)(floor(rays[x].start.x) / map->TileWidth);
			int cellY = (int)(floor(rays[x].Midpoint().y / map->TileWidth));

			// Check Current Cell - if not solid get the one below it
			Assets::MapCell checkCell = map->GetTileInfo(cellX, cellY);
			if (!checkCell.IsSolid) {
				cellY++;
				checkCell = map->GetTileInfo(cellX, cellY);
			}

			// Get polygon for check cell - and update to world position
			Poly::Polygon checkPoly = map->GetPolygon(cellX, cellY);
			checkPoly.SetPosition(cellX * map->TileWidth, cellY * map->TileHeight);

			// Does the polygon have any edges?
			if (checkPoly.GetEdgeCount() > 0)
			{
				// Get the top edge of the target polygon
				Poly::line2d checkEdge = checkPoly.GetEdge(Poly::EDGETYPE::TOP, Poly::COORDTYPE::TRANSLATED).line;
				
				// Get intersection between the current check ray and the top face of the taget polygon
				try {
					ix[x] = rays[x].Intersect(checkEdge);
					// Intersect distance is the y coord of the intersection minus the midpoint of
					// our check polygon (which is the y coordinate of the bottom face)
					dy[x] = ix[x].y - rays[x].Midpoint().y - 1;
					intersectMask |= (int)pow(2, x);
					cellBelow[x] = { checkCell, checkEdge };
				}
				catch (Poly::InvalidIntersect)
				{
					// Set the intersect distance for this ray to be out of bounds for our range checking
					dy[x] = 100;
				}
			}
		}

		float dpy = _velocity.y;
		bool landed = false;
		float rotation = 0.0f;

		bool onLevelGround[3];
		for (int x = 0; x < 3; x++)
			onLevelGround[x] = ((dy[x] <= _velocity.y && cellBelow[x].topEdge.m == 0) || cellBelow[x].cell.TileID == -1);

		// If we have no intersections, leave velocity as is
		if (intersectMask == 0) {
			landed = false;
		}
		// If all intersect distances are greater than the current velocity, leave it as it is
		else if (dy[0] > _velocity.y && dy[1] > _velocity.y && dy[2] > _velocity.y)
		{
			landed = false;
		}
		// If all rays are on level ground or in mid air, we are on solid ground, adjust velocity accordingly
		else if (onLevelGround[0] && onLevelGround[1] && onLevelGround[2])
		{
			dpy = std::min(std::min(std::max(dy[0], -2.0f), std::max(dy[1], -2.0f)), std::max(dy[2], -2.0f));
			_rotation = 0;
			landed = true;
			_cellBelow = dy[1] <= 1 ? cellBelow[1] : dy[0] <= 1 ? cellBelow[0] : cellBelow[2];
		}
		// If the center check ray is above a slope
		else if (cellBelow[1].cell.TileID != -1)
		{
			if (dy[1] > 1)
			{
				landed = false;
			}
			else
			{
				// If the slope has a gradient between - 1.1 and 1.1, use the center check ray 
				// for our rotation calculation
				if (cellBelow[1].topEdge.m >= -1.1 && cellBelow[1].topEdge.m <= 1.1)
				{
					float baseGrad = cellBelow[1].topEdge.m;

					// Axis of rotation is the centre of the bottom face of our polygon
					rotationPoint = RotationPoint::BOTTOMCENTRE;
					Poly::Edge e = objPoly.GetEdge(Poly::EDGETYPE::BOTTOM, Poly::COORDTYPE::MODEL);
					SetRotation(atan(baseGrad), { e.line.Midpoint().x, e.line.Midpoint().y });
					dpy = dy[1];
					landed = true;
				}
				_cellBelow = cellBelow[1];
			}
		}
		else if (dy[0] <= _velocity.y)
		{
			// Get end point to determine rotation
			float baseGrad = cellBelow[0].topEdge.m;
			if (dy[2] <= _velocity.y)
				baseGrad = (ix[2].y - ix[0].y) / (ix[2].x - ix[0].x);
			else if (dy[1] <= _velocity.y)
				baseGrad = (ix[1].y - ix[0].y) / (ix[1].x - ix[0].x);

			// Axis of rotation is the bottom left corner of our polygon
			rotationPoint = RotationPoint::BOTTOMLEFT;
			Poly::Edge e = objPoly.GetEdge(Poly::EDGETYPE::BOTTOM, Poly::COORDTYPE::MODEL);
			SetRotation(atan(baseGrad), { e.line.start.x, e.line.end.y });
			dpy = _props->SlidingDownSlope ? _velocity.y : dy[0];
			landed = true;
			_cellBelow = cellBelow[0];
		}
		// If right edge is on a slope
		else if (dy[2] <= _velocity.y)
		{
			// Get end point to determine rotation
			float baseGrad = cellBelow[2].topEdge.m;
			if (dy[0] <= _velocity.y)
				baseGrad = (ix[2].y - ix[0].y) / (ix[2].x - ix[0].x);
			else if (dy[1] <= _velocity.y)
				baseGrad = (ix[1].y - ix[0].y) / (ix[1].x - ix[0].x);

			// Axis of rotation is the bottom right corner of our polygon
			rotationPoint = RotationPoint::BOTTOMRIGHT;
			Poly::Edge e = objPoly.GetEdge(Poly::EDGETYPE::BOTTOM, Poly::COORDTYPE::MODEL);
			SetRotation(atan(baseGrad), { e.line.end.x, e.line.end.y });
			dpy = _props->SlidingDownSlope ? _velocity.y : dy[2];
			landed = true;
			_cellBelow = cellBelow[2];
		}

		// Adjust X Velocity based on friction and slope of cell below us
		if (_cellBelow.cell.TileID != -1)
		{
			float belowGrad = _cellBelow.topEdge.m;
			if (belowGrad != 0)
			{
				float calcGrad = (signbit(belowGrad) ? -1.0f : 1.0f) * ceil(abs(belowGrad) * 10) / 10;
				if (abs(belowGrad) > 1.1)
				{
					// If we are above a steep slope, disable x input and adjust x and y velocity
					// to propel the character down the slope.
					_acceptXInput = false;
					_props->SlidingDownSlope = true;
					_velocity.x = (_velocity.y / calcGrad) + 0.5;
				}
			}
		}

		// Implement changes from above logic checks
		_fineCoords.y += dpy;

		if (landed)
		{
			Land();
			if (!_props->SlidingDownSlope)
				_velocity.y = 0;
		}
		else 
			Float();

#ifdef SHOW_COLLISION_DATA
		_pge->SetDrawTarget(nullptr);
		for (auto l : rays)
			_pge->DrawLine(
				{ (int)l.start.x - map->Origin.x, (int)l.start.y - map->Origin.y },
				{ (int)l.end.x - map->Origin.x, (int)l.end.y - map->Origin.y },
				olc::CYAN
			);
#endif

		// Clean up
		cellBelow.clear();
		rays.clear();
		ix.clear();
		dy.clear();
	}

	void Player::CheckHorizontalCollisions(Assets::Map* map)
	{
		if (_velocity.x == 0 || _props->SlidingDownSlope)
		{
			_fineCoords.x += _velocity.x;
			return;
		}

		// Get our check polygon and cast rays horizontally for collision checks
		Poly::Polygon objPoly = _spriteData->PolyMasks[_animation->GetCurrentSprite()];
		objPoly.SetPosition(_fineCoords.x, _fineCoords.y, _rotation, Poly::vec2d{ _rotationAxis.x, _rotationAxis.y });
		std::vector<Poly::line2d> rays = objPoly.GetHorizontalCheckRays(signbit(_velocity.x));
		float checkMaskWidth = objPoly.GetAABB().br.x - objPoly.GetAABB().tl.x;

		// Work out our changes to x and y based on velocity and our current rotation
		float dvx = (_velocity.x * cosf(_rotation));
		float dvy = (_velocity.x * sinf(_rotation));

		// Check collisions with our check rays
		std::vector<Poly::vec2d> ix; ix.resize(3);
		std::vector<float> dx = { 100,100,100 };
		std::vector<CellBesidePlayer> cbp; cbp.resize(3);

		Poly::line2d footEdge;
		int intersectMask = 0;
		int closestCollisionX = _velocity.x < 0 ? -2147483647 : 2147483647;

  		for (int x = 0; x < 3; x++)
		{
			int mapx = _velocity.x < 0 ? (int)floor(rays[x].Midpoint().x + _velocity.x) : (int)ceil(rays[x].Midpoint().x + _velocity.x);
			olc::vi2d checkMapRef = map->WorldCoordToMapRef({ mapx, (int)ceil(rays[x].Midpoint().y)});
			Assets::MapCell checkCell = map->GetTileInfo(checkMapRef);
			Poly::Polygon checkPoly = map->GetPolygon(checkMapRef);
			checkPoly.SetPosition(checkMapRef.x * map->TileWidth, checkMapRef.y * map->TileHeight);

			if (checkPoly.GetEdgeCount() > 0)
			{
				Poly::line2d checkEdge = checkPoly.GetEdge(_velocity.x > 0 ? Poly::EDGETYPE::LEFT : Poly::EDGETYPE::RIGHT, Poly::COORDTYPE::TRANSLATED).line;
				try {
					ix[x] = rays[x].Intersect(checkEdge);
					dx[x] = ix[x].x - rays[x].Midpoint().x;
					cbp[x] = { checkCell, checkEdge };
					intersectMask |= (int)pow(2, x);
					closestCollisionX = _velocity.x < 0 ? std::max((int)ix[x].x, closestCollisionX) : std::min((int)ix[x].x, closestCollisionX);
				}
				catch (Poly::InvalidIntersect)
				{
					dx[x] = 100;
				}
			}
		}

		// if we have a collision with a wall that has a gradient greater than 1.1,
		// kill x and y velocity and stop x direction input. Offset for new position 
		// is based on the idle collision mask.
		bool collided = (dx[0] * (_direction == FacingDirection::LEFT ? -1.0f : 1.0f)) <= 0
			|| (dx[1] * (_direction == FacingDirection::LEFT ? -1.0f : 1.0f)) <= 0
			|| (dx[2] * (_direction == FacingDirection::LEFT ? -1.0f : 1.0f)) <= 0;

		if (collided && TooSteep(_velocity.x < 0, cbp[0].verticalEdge))
		{
			dvx = 0; dvy = _props->OnGround ? 0.0f : _velocity.y;
			int xAdjust = 0;
			
			int idleSpriteID = _spriteData->Animations[_direction == Actor::FacingDirection::LEFT ? "Idle Left" : "Idle Right"].Frames[0];
			Poly::Polygon adjustMask = _spriteData->PolyMasks[idleSpriteID];
			Poly::Edge adjustEdge = adjustMask.GetEdge(Poly::FACE::BOTTOM);
			Poly::vec2d adjustAxis = rotationPoint == RotationPoint::BOTTOMCENTRE ? adjustEdge.line.Midpoint() : rotationPoint == RotationPoint::BOTTOMLEFT ? adjustEdge.line.start : adjustEdge.line.end;
			adjustMask.SetPosition(objPoly.GetPosition().x, objPoly.GetPosition().y, objPoly.GetRotation(), adjustAxis);

			if (_velocity.x < 0)
				xAdjust = closestCollisionX - (adjustMask.GetAABB().tl.x - adjustMask.GetPosition().x) + 1;
			else
				xAdjust = closestCollisionX - (adjustMask.GetAABB().br.x - adjustMask.GetPosition().x) - 1;

			//if (_velocity.x < 0)
			//	xAdjust = -(objPoly.GetAABB().tl.x - _fineCoords.x) + 1;
			//else
			//	xAdjust = -(objPoly.GetAABB().br.x - _fineCoords.x) - 1;
			//_fineCoords.x = closestCollisionX + xAdjust;

			_fineCoords.x = xAdjust;
			_velocity.x = 0;
			_acceptXInput = false;
			SetAnimation(_direction == FacingDirection::LEFT ? "Idle Left" : "Idle Right");
		}

#ifdef SHOW_COLLISION_DATA

		_pge->SetDrawTarget(nullptr);
		for (auto l : rays)
			_pge->DrawLine(
				{ (int)l.start.x - map->Origin.x, (int)l.start.y - map->Origin.y },
				{ (int)l.end.x - map->Origin.x, (int)l.end.y - map->Origin.y },
				olc::CYAN
		);
#endif

		// Update the character world position
		_fineCoords.x += dvx;    
		_fineCoords.y += dvy;
		
		return;

	}

	void Player::HandleCollisions(Assets::Map* map, float timeElapsed)
	{
		CheckAboveCharacter(map);
		CheckBelowCharacter(map);
		CheckHorizontalCollisions(map);
	}
}
