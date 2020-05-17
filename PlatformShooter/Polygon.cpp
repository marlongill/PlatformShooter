#include "Polygon.h"

namespace Poly
{
	float vec2d::distance(vec2d t)
	{
		float dx = t.x - x;
		float dy = t.y - y;
		return sqrtf((dx * dx) + (dy * dy));
	}

	line2d::line2d()
	{
	}

	line2d::line2d(vec2d s, vec2d e)
	{
		if (s.x > e.x) std::swap(s, e);
		start = s; end = e;
		m = (e.y - s.y) / (e.x - s.x);
		CalculateC();
	}

	void line2d::CalculateC()
	{
		c = start.y - m * start.x;
	}

	line2d line2d::Normal()
	{
		return {
			{ -(end.y - start.y), (end.x - start.x) } ,
			{ (end.y - start.y), -(end.x - start.x) }
		};
	}

	vec2d line2d::Intersect(line2d l2)
	{
		line2d l1 = line2d(this->start, this->end);
		vec2d result = { -INFINITY, -INFINITY };

		LINETYPE l1HV = LINETYPE::DIAGONAL, l2HV = LINETYPE::DIAGONAL;

		if (l1.start.x == l1.end.x) l1HV = LINETYPE::VERTICAL;
		if (l1.start.y == l1.end.y) l1HV = LINETYPE::HORIZONTAL;
		if (l2.start.x == l2.end.x) l2HV = LINETYPE::VERTICAL;
		if (l2.start.y == l2.end.y) l2HV = LINETYPE::HORIZONTAL;

		float min1y = std::min(l1.start.y, l1.end.y); float max1y = std::max(l1.start.y, l1.end.y);
		float min2y = std::min(l2.start.y, l2.end.y); float max2y = std::max(l2.start.y, l2.end.y);

		if (l1HV != LINETYPE::DIAGONAL || l2HV != LINETYPE::DIAGONAL)
		{
			if (l1HV == l2HV)
			{
				if (l1HV == LINETYPE::HORIZONTAL
					&& (int)l1.start.y == (int)l2.start.y
					&& ((l1.start.x >= l2.start.x and l1.start.x <= l2.end.x)
						|| (l2.start.x >= l1.start.x and l2.start.x <= l1.end.x))
				)
				{ 
					result = { std::max(l1.start.x, l2.start.x), l1.start.y  };
				}
				else if (l1HV == LINETYPE::VERTICAL 
						 && (int)l1.start.x == (int)l2.start.x
						 && ((l1.start.y >= l2.start.y and l1.start.y <= l2.end.y)
						  	 || (l2.start.y >= l1.start.y and l2.start.y <= l1.end.y))
						)
				{
						result = { l1.start.x, std::max(l1.start.y, l2.start.y) };
				}
				else return result; // Parallel Cannot Converge
			}
			if (l1HV != LINETYPE::DIAGONAL && l2HV != LINETYPE::DIAGONAL) // One Vertical, One Horizontal
			{
				if (l1HV == LINETYPE::HORIZONTAL) { // Line 1 Vertical, Line 2 Horizontal
					std::swap(l1, l2);
					std::swap(min1y, min2y);
					std::swap(max1y, max2y);
				}

				if (l1.start.x >= l2.start.x && l1.start.x <= l2.end.x
					&& min2y >= min1y && min2y <= max1y)
				{
					result = { l1.start.x, min2y };
				}
			}
			else // One diagonal, one not
			{
				// If line 2 is the diagonal, swap the lines around to save code repetition
				if (l2HV != LINETYPE::DIAGONAL) { // Line 1 is horizontal or vertical
					std::swap(l1, l2);
					std::swap(min1y, min2y);
					std::swap(max1y, max2y);
					std::swap(l1HV, l2HV);
				}

				if (l1HV == LINETYPE::HORIZONTAL) // Line 1 Horizontal, line 2 not horizonal or vertical
				{
					if (min1y >= min2y && min1y <= max2y)
					{
						float ix = (min1y - l2.c) / l2.m;
						if (ix >= l1.start.x && ix <= l1.end.x)
							result = { ix, min1y };
					}
				}
				else // Line 1 Vertical, line 2 not horizonal or vertical 
				{
					if (l1.start.x >= l2.start.x && l1.start.x <= l2.end.x)
					{
						float iy = (l2.m * (l1.start.x - l2.start.x)) + l2.start.y;
						if (iy >= min1y && iy <= max1y)
							result = { l1.start.x, iy };
					}
				}
			}
		}
		else // We have two diagonals
		{
			if ((l1.m - l2.m) != 0)
			{
				float ix = (l2.c - l1.c) / (l1.m - l2.m);
				float iy = l1.m * ix + l1.c;
				if ((ix >= l1.start.x && ix <= l1.end.x && iy >= min1y && iy <= max1y))
					result = { ix, iy };
			}
		}

		if (result.x == -INFINITY)
			throw InvalidIntersect();

		return result;
	}

	vec2d line2d::Midpoint()
	{
		return { start.x + ((end.x - start.x) / 2), start.y + ((end.y - start.y) / 2) };
	}

	vec2d line2d::AxisOfProjection()
	{
		vec2d aop = { -end.y - start.y, end.x - start.x };
		float d = sqrtf(aop.x * aop.x + aop.y * aop.y);
		return { aop.x / d, aop.y / d };
	}

	Polygon::Polygon() {}

	Polygon::Polygon(std::vector<Edge> faces)
	{
		_model.clear();
		for (Edge v : faces)
			_model.push_back(v);
		_translated.resize(faces.size());
		_rotated.resize(faces.size());

		UpdateInternals();
	}

	Polygon::~Polygon()
	{
		_model.clear();
		_translated.clear();
		_rotated.clear();
	}

	void Polygon::CalculateCentroid()
	{
		_centroid = { 0, 0 };
		double signedArea = 0.0;
		double x0 = 0.0; double y0 = 0.0; 
		double x1 = 0.0; double y1 = 0.0; 
		double a = 0.0;  

		// For all vertices
		if (IsRect())
		{
			_centroid = {
				_aabb.tl.x + ((_aabb.br.x - _aabb.tl.x) / 2),
				_aabb.tl.y + ((_aabb.br.y - _aabb.tl.y) / 2),
			};
		}
		else
		{
			int faceCount = _model.size();
			for (int i = 0; i < faceCount; ++i)
			{
				x0 = _model[i].line.start.x;
				y0 = _model[i].line.start.y;
				x1 = _model[i].line.end.x;
				y1 = _model[i].line.end.y;
				a = x0 * y1 - x1 * y0;
				signedArea += a;
				_centroid.x += (x0 + x1) * a;
				_centroid.y += (y0 + y1) * a;
			}

			signedArea *= 0.5;
			_centroid.x /= (6.0 * signedArea);
			_centroid.y /= (6.0 * signedArea);
		}
	}

	void Polygon::CalculateAABB()
	{
		_aabb.tl = { INFINITY, INFINITY }; _aabb.br = { -INFINITY, -INFINITY };
		for (Edge v : _translated)
		{
			_aabb.tl.x = std::min(v.line.start.x, _aabb.tl.x); _aabb.tl.y = std::min(v.line.start.y, _aabb.tl.y);
			_aabb.br.x = std::max(v.line.end.x, _aabb.br.x); _aabb.br.y = std::max(v.line.end.y, _aabb.br.y);
		}
		
	}

	void Polygon::CalculateTranslatedPoints()
	{
		for (int i = 0; i < _model.size(); i++)
		{
			float rx = _rotationAxis.x == -1 ? _centroid.x : _rotationAxis.x;
			float ry = _rotationAxis.y == -1 ? _centroid.y : _rotationAxis.y;

			_rotated[i] = {
				{
					{
						(float)((_model[i].line.start.x - rx) * cosf(_angle)) - ((_model[i].line.start.y - ry) * sinf(_angle)) + _pos.x + rx,
						(float)((_model[i].line.start.x - rx) * sinf(_angle)) + ((_model[i].line.start.y - ry) * cosf(_angle)) + _pos.y + ry
					},
					{
						(float)((_model[i].line.end.x - rx) * cosf(_angle)) - ((_model[i].line.end.y - ry) * sinf(_angle)) + _pos.x + rx,
						(float)((_model[i].line.end.x - rx) * sinf(_angle)) + ((_model[i].line.end.y - ry) * cosf(_angle)) + _pos.y + ry
					},
				},
				_model[i].type
			};

			_translated[i] = {
				{
					{ _model[i].line.start.x + _pos.x,_model[i].line.start.y + _pos.y } ,
					{ _model[i].line.end.x + _pos.x,_model[i].line.end.y + _pos.y } 
				},
				_model[i].type
			};
		}
	}

	void Polygon::UpdateInternals()
	{
		CalculateCentroid();
		CalculateTranslatedPoints();
		CalculateAABB();
	}
	
	Edge Polygon::FindEdge(FACE face, COORDTYPE type)
	{
		EDGETYPE toFind = EDGETYPE::OTHER;

		switch (face)
		{
		case FACE::TOP: toFind = EDGETYPE::TOP; break;
		case FACE::LEFT: toFind = EDGETYPE::LEFT; break;
		case FACE::BOTTOM: toFind = EDGETYPE::BOTTOM; break;
		case FACE::RIGHT: toFind = EDGETYPE::RIGHT; break;
		}

		return FindEdge(toFind, type);
	}

	Edge Polygon::FindEdge(EDGETYPE toFind, COORDTYPE type)
	{
		if (toFind == EDGETYPE::OTHER)
			return { { { 0, 0 }, { 0,0 } }, EDGETYPE::OTHER };
		else
		{
			int idx = -1;
			for (int x = 0; x < _model.size(); x++)
			{
				if (_model[x].type == toFind) {
					idx = x;
					break;
				}
			}

			if (idx == -1)
				return { { { 0, 0 }, { 0,0 } }, EDGETYPE::OTHER };
			else
			{
				switch (type)
				{
				case COORDTYPE::MODEL: return _model[idx];
				case COORDTYPE::TRANSLATED: return _translated[idx];
				case COORDTYPE::ROTATED: return _rotated[idx];
				}
			}
		}
	}

	Edge Polygon::GetEdge(FACE face)
	{
		if (face == FACE::TOP || face == FACE::BOTTOM || face == FACE::LEFT || face == FACE::RIGHT)
			return FindEdge(face, COORDTYPE::ROTATED);
		else {
			int h = 4; // (int)(_aabb.br.y - _aabb.tl.y) / 2;
			line2d f = line2d({ 0, 0 }, { 0,0 });
			float x, y;

			switch (face)
			{
			case FACE::LEFT_EDGE : 
				f = FindEdge(EDGETYPE::LEFT, COORDTYPE::ROTATED).line;
				x = std::min(f.start.x, f.end.x);
				y = std::max(f.start.y, f.end.y);
				return { { { x, y - h }, { x, y + h } }, EDGETYPE::OTHER };
			case FACE::RIGHT_EDGE: 
				f = FindEdge(EDGETYPE::RIGHT, COORDTYPE::ROTATED).line;
				x = std::max(f.start.x, f.end.x);
				y = std::max(f.start.y, f.end.y);
				return { { { x, y - h }, { x, y + h } }, EDGETYPE::OTHER };
			case FACE::CENTER_VERT:
				line2d f1 = FindEdge(EDGETYPE::LEFT, COORDTYPE::ROTATED).line;
				line2d f2 = FindEdge(EDGETYPE::RIGHT, COORDTYPE::ROTATED).line;
				float x1 = std::min(f1.start.x, f1.end.x);
				float y1 = std::max(f1.start.y, f1.end.y);
				float x2 = std::max(f2.start.x, f2.end.x);
				float y2 = std::max(f2.start.y, f2.end.y);
				float x = x1 + ((x2 - x1) / 2);
				float y = y1 + ((y2 - y1) / 2);
				return { {{ x, y - h }, { x, y + h }}, EDGETYPE::OTHER };
			}
		}
	}

	std::vector<Edge> Polygon::GetFaces() { return _rotated; }
	vec2d Polygon::GetCentre() { return { _centroid.x + _pos.x, _centroid.y + _pos.y }; }
	rect2d Polygon::GetAABB() { return _aabb; }
	vec2d Polygon::GetPosition() { return _pos; }
	float Polygon::GetRotation() { return _angle; }

	std::vector<line2d> Polygon::GetVerticalCheckRays(bool vxsign, bool up)
	{
		std::vector<line2d> result;

		line2d e;
		int maskHeight = _aabb.br.y -_aabb.tl.y;
		if (up)
			e = FindEdge(EDGETYPE::TOP, COORDTYPE::ROTATED).line;
		else
			e = FindEdge(EDGETYPE::BOTTOM, COORDTYPE::ROTATED).line;

		float lx = e.start.x;
		float ly = e.start.y;

		float rx = e.end.x;
		float ry = e.end.y;
		float midx = lx + ((rx - lx) / 2);
		float midy = ly + ((ry - ly) / 2);

		if (vxsign)
		{
			// Moving left
			lx = floor(lx); midx = floor(midx); rx = floor(rx);
		}
		else
		{
			// Moving right
			lx = ceil(lx); midx = ceil(midx); rx = ceil(rx);
		}

		result.push_back(line2d({ lx, ly - maskHeight }, { lx, ly + maskHeight }));
		result.push_back(line2d({ midx, midy - maskHeight }, { midx, midy + maskHeight }));
		result.push_back(line2d({ rx, ry - maskHeight }, { rx, ry + maskHeight }));
		return result;
	}


	std::vector<line2d> Polygon::GetUpCheckRays(bool vxsign) {
		return GetVerticalCheckRays(vxsign, true);
	}
		
	std::vector<line2d> Polygon::GetDownCheckRays(bool vxsign) 
	{
		return GetVerticalCheckRays(vxsign, false);
	}

	std::vector<line2d> Polygon::GetHorizontalCheckRays(bool left)
	{
		std::vector<line2d> result;

		// Horizontal collision check rays are parallel to the surface we are standing on
		line2d lb = FindEdge(EDGETYPE::BOTTOM, COORDTYPE::MODEL).line;
		line2d edge = FindEdge(left ? EDGETYPE::LEFT : EDGETYPE::RIGHT, COORDTYPE::ROTATED).line;

		float w = abs(lb.end.x - lb.start.x);

		float tx, ty, bx, by, mx, my;
		float dxl, dxr, dyl, dyr, dx, dy;

		if (_angle == 0)
		{
			tx = edge.start.x;
			ty = std::min(edge.start.y, edge.end.y);

			bx = edge.start.x;
			by = std::max(edge.start.y, edge.end.y);

			mx = edge.start.x;
			my = ty + ((by - ty) / 2);

			dxl = -w; dxr = w;
			dyl = 0; dyr = 0;
		}
		else
		{
			dxl = cos(_angle) * -w;
			dyl = sin(_angle) * -w;
			dxr = -dxl; dyr = -dyl; 

			if (edge.start.y > edge.end.y) 
				std::swap(edge.start, edge.end);
			tx = edge.start.x;
			ty = edge.start.y;
			bx = edge.end.x;
			by = edge.end.y;

			mx = bx + ((tx - bx) / 2);
			my = by + ((ty - by) / 2);
		}

		// Create result lines
		result.push_back(line2d({ tx + dxl, ty + dyl }, { tx + dxr, ty + dyr }));
		result.push_back(line2d({ mx + dxl, my + dyl }, { mx + dxr, my + dyr }));
		result.push_back(line2d({ bx + dxl, by + dyl }, { bx + dxr, by + dyr }));

		return result;
	}

	bool Polygon::IsRect() 
	{
		if (_translated.size() != 4)
			return false;
		else
		{
			line2d top = FindEdge(EDGETYPE::TOP, COORDTYPE::MODEL).line;
			line2d bottom = FindEdge(EDGETYPE::BOTTOM, COORDTYPE::MODEL).line;
			line2d left = FindEdge(EDGETYPE::LEFT, COORDTYPE::MODEL).line;
			line2d right = FindEdge(EDGETYPE::RIGHT, COORDTYPE::MODEL).line;
			return (
				top.m == 0 
				&& bottom.m == 0 
				&& (left.m == INFINITY || left.m == -INFINITY) 
				&& (right.m == INFINITY || right.m == -INFINITY) 
			);
		}
	}

	int Polygon::GetEdgeCount() { return _model.size(); }

	void Polygon::SetPosition(float x, float y, float rotation, vec2d axis)
	{
		_pos = { x, y }; _angle = rotation; _rotationAxis = axis;
		UpdateInternals();
	}
	void Polygon::SetPosition(float x, float y, float rotation)
	{
		_pos = { x, y }; _angle = rotation;
		UpdateInternals();
	}
	void Polygon::SetPosition(float x, float y)
	{
		_pos = { x, y };
		UpdateInternals();
	}
	void Polygon::SetPosition(float rotation)
	{
		_angle = rotation;
		UpdateInternals();
	}
	void Polygon::UpdatePosition(float dx, float dy, float drotation)
	{
		_pos = { _pos.x + dx, _pos.y + dy }; _angle += drotation;
		UpdateInternals();
	}
	void Polygon::UpdatePosition(float dx, float dy)
	{
		for (int i = 0; i < _translated.size(); i++)
		{
			_translated[i].line.start.x += dx; _translated[i].line.start.y += dy;
			_translated[i].line.end.x += dx; _translated[i].line.end.y += dy;
			_translated[i].line.CalculateC();

			_rotated[i].line.start.x += dx; _rotated[i].line.start.y += dy;
			_rotated[i].line.end.x += dx; _rotated[i].line.end.y += dy;
			_rotated[i].line.CalculateC();
		}
		_pos = { _pos.x + dx, _pos.y + dy };
		CalculateAABB();
	}

	void Polygon::ClearEdges()
	{
		_model.clear();
		_translated.clear();
		_rotated.clear();
	}

	void Polygon::AddEdge(Edge face)
	{
		_model.push_back(face);
		_translated.resize(_model.size());
		_rotated.resize(_model.size());
		UpdateInternals();
	}

	Edge Polygon::GetEdge(int index, COORDTYPE type)
	{
		switch (type) {
			case COORDTYPE::MODEL: return _model[index];
			case COORDTYPE::ROTATED: return _rotated[index];
			case COORDTYPE::TRANSLATED: return _translated[index];
		}
	}

	Edge Polygon::GetEdge(EDGETYPE type, COORDTYPE coordType)
	{
		return FindEdge(type, coordType);
	}

	/* ==========================================================================
	   == Returns the face in this polygon closest to the point passed         ==
	   ==========================================================================*/
	int Polygon::NearestFaceToPoint(vec2d point)
	{
		int faceStartPoint = 0;
		float minD = INFINITY;

		for (int i = 0; i < _translated.size(); i++)
		{
			// Calculate distance from point
			float d = _translated[i].line.Midpoint().distance(point);
			// Replace index if closer
			if (d < minD) {
				minD = d;
				faceStartPoint = i;
			}
		}

		return faceStartPoint;
	}

	bool Polygon::Overlap_SAT(Polygon* r1)
	{
		Polygon* poly1 = r1;
		Polygon* poly2 = this;

		for (int shape = 0; shape < 2; shape++)
		{
			if (shape == 1)
			{
				poly1 = this;
				poly2 = r1;
			}

			std::vector<Edge> p1 = poly1->GetFaces();
			std::vector<Edge> p2 = poly2->GetFaces();

			for (int a = 0; a < poly1->GetEdgeCount(); a++)
			{
				vec2d axisProj = p1[a].line.AxisOfProjection();

				// Work out min and max 1D points for r1
				float min_r1 = INFINITY, max_r1 = -INFINITY;
				for (int p = 0; p < p1.size(); p++)
				{
					float q = (p1[p].line.start.x * axisProj.x + p1[p].line.start.y * axisProj.y);
					min_r1 = std::min(min_r1, q);
					max_r1 = std::max(max_r1, q);
					q = (p1[p].line.end.x * axisProj.x + p1[p].line.end.y * axisProj.y);
					min_r1 = std::min(min_r1, q);
					max_r1 = std::max(max_r1, q);
				}

				// Work out min and max 1D points for r2
				float min_r2 = INFINITY, max_r2 = -INFINITY;
				for (int p = 0; p < p2.size(); p++)
				{
					float q = (p2[p].line.start.x * axisProj.x + p2[p].line.start.y * axisProj.y);
					min_r2 = std::min(min_r2, q);
					max_r2 = std::max(max_r2, q);
					q = (p2[p].line.end.x * axisProj.x + p2[p].line.end.y * axisProj.y);
					min_r2 = std::min(min_r2, q);
					max_r2 = std::max(max_r2, q);
				}

				if (!(max_r2 >= min_r1 && max_r1 >= min_r2))
					return false;
			}
		}

		return true;
	}

	bool Polygon::BoundingBoxOverlap(Polygon* poly)
	{
		float d1x = poly->GetAABB().tl.x - _aabb.br.x;
		float d1y = poly->GetAABB().tl.y - _aabb.br.y;
		float d2x = _aabb.tl.x - poly->GetAABB().br.x;
		float d2y = _aabb.tl.y - poly->GetAABB().br.y;

		if (d1x > 0.0f || d1y > 0.0f)
			return false;

		if (d2x > 0.0f || d2y > 0.0f)
			return false;

		return true;
	}

	std::vector<std::vector<Intersect>> Polygon::IntersectingFaces(Polygon* r2, FACE face)
	{
		std::vector<FACE> faces;
		if (face == FACE::ALL) {
			faces = { FACE::TOP, FACE::BOTTOM, FACE::LEFT, FACE::RIGHT };
		}
		else {
			faces.push_back(face);
		}
		return IntersectingFaces(r2, face);
	}

	std::vector<std::vector<Intersect>> Polygon::IntersectingFaces(Polygon* r2, std::vector<FACE> faces)
	{
		Polygon* r1 = this;

		std::vector<std::vector<Intersect>> result;
		result.resize((int)FACE::ALL + 1);

		if (r2->GetEdgeCount() == 0)
			return result;

		for (FACE face : faces)
		{
			line2d l1 = r1->GetEdge(face).line;

			// If both polygons are AABB, we can shortcut the checks
			if (r1->IsRect() && r2->IsRect())
			{
				int x = l1.start.x; int y = l1.start.y;
				rect2d destAABB = r2->GetAABB();
				FACE op1;

				if (face == FACE::TOP || face == FACE::BOTTOM)
				{
					op1 = face == FACE::TOP ? FACE::BOTTOM : FACE::TOP;
					if (y >= destAABB.tl.y && y <= destAABB.br.y)
						result[(int)face].push_back({ { (float)x, (float)y }, r2->GetEdge(op1) });
				}
				else
				{
					op1 = face == FACE::LEFT ? FACE::RIGHT : FACE::LEFT;
					if (x >= destAABB.tl.x && x <= destAABB.br.x)
						result[(int)face].push_back({ { (float)x, (float)y }, r2->GetEdge(op1) });
				}
			}
			else
			{
				// Check current face of source object against all faces of the target object
				line2d l1 = r1->GetEdge(face).line;
				for (int x = 0; x < r2->GetEdgeCount(); x++)
				{
					Edge l2 = r2->GetEdge(x, COORDTYPE::ROTATED);
					vec2d intersection = l1.Intersect(l2.line);
					if (intersection.x != -INFINITY) {
						result[(int)face].push_back({ intersection, l2 });
					}
				}
			}
		}
		return result;
	}
}