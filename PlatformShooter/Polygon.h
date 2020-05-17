#pragma once
#include "olcPixelGameEngine.h"
#include <cmath>

namespace Poly
{
	enum class FACE { TOP = 0, RIGHT, BOTTOM, LEFT, LEFT_EDGE, CENTER_VERT, RIGHT_EDGE, ALL };
	enum class LINETYPE { DIAGONAL = 0, VERTICAL = 1, HORIZONTAL = 2 };
	enum class EDGETYPE { TOP = 0, RIGHT, BOTTOM, LEFT, OTHER};
	enum class COORDTYPE { MODEL, TRANSLATED, ROTATED };

	struct InvalidIntersect : public std::exception
	{
		const char* what() const throw ()
		{
			return "Lines do not intersect";
		}
	};

	struct vec2d
	{
		float x = INFINITY;
		float y = INFINITY;
		bool operator<(const vec2d& rhs) const {
			return y == rhs.x< rhs.x ? (y < rhs.y) : (x < rhs.x);
		}
		bool operator==(const vec2d& rhs) const {
			return x == rhs.x && y == rhs.y;
		}
		float distance(vec2d t);
	};

	struct rect2d
	{
		vec2d tl; // Top left coordinate of the rectangle
		vec2d br; // Bottom right coordinate of the rectangle
	};

	class line2d
	{
	public:
		vec2d start;
		vec2d end;
		float m;
		float c;
		line2d();
		line2d(vec2d s, vec2d e);
		vec2d Intersect(line2d l2);
		vec2d Midpoint();
		vec2d AxisOfProjection();
		line2d Normal();
		void CalculateC();
	};

	struct Edge
	{
		line2d line;
		EDGETYPE type;
	};

	struct Intersect
	{
		vec2d point;
		Edge face;
	};

	class Polygon
	{
	private:
		float _angle = 0.0f;				// Direction of shape
		vec2d _rotationAxis = { -1, -1 };	// Rotation axis point
		std::vector<Edge> _model;			// "Model" of shape							
		vec2d _pos = { 0,0 };				// Position of shape
		rect2d _aabb;						// Calculated bounding box for this shape
		vec2d _centroid;					// Center point of polygon
		std::vector<Edge> _translated;		// Model translated by position without rotation
		std::vector<Edge> _rotated;			// Model translated by position with rotation

	private:
		void CalculateTranslatedPoints();
		void CalculateCentroid();
		void CalculateAABB();
		void UpdateInternals();

		Edge FindEdge(FACE face, COORDTYPE type);
		Edge FindEdge(EDGETYPE face, COORDTYPE type);

	public:

		Polygon();
		Polygon(std::vector<Edge> faces);
		~Polygon();

		std::vector<Edge> GetFaces();
		vec2d GetCentre();
		rect2d GetAABB();
		vec2d GetPosition();
		float GetRotation();

		std::vector<line2d> GetVerticalCheckRays(bool vxsign, bool up);
		std::vector<line2d> GetUpCheckRays(bool vxsign);
		std::vector<line2d> GetDownCheckRays(bool vxsign);
		std::vector<line2d> GetHorizontalCheckRays(bool vxsign);

		bool IsRect();

		int GetEdgeCount();

		void SetPosition(float x, float y, float rotation, vec2d axis);
		void SetPosition(float x, float y, float rotation);
		void SetPosition(float x, float y);
		void SetPosition(float rotation);

		void UpdatePosition(float dx, float dy, float drotation);
		void UpdatePosition(float dx, float dy);

		void ClearEdges();
		void AddEdge(Edge face);

		Edge GetEdge(FACE face);
		Edge GetEdge(EDGETYPE type, COORDTYPE coordType = COORDTYPE::ROTATED);
		Edge GetEdge(int index, COORDTYPE type = COORDTYPE::ROTATED);

		int NearestFaceToPoint(vec2d point);
		bool Overlap_SAT(Polygon* r1);
		bool BoundingBoxOverlap(Polygon* poly);
		std::vector<std::vector<Intersect>> IntersectingFaces(Polygon* r2, FACE face);
		std::vector<std::vector<Intersect>> IntersectingFaces(Polygon* r2, std::vector<FACE> faces);

	};
}
