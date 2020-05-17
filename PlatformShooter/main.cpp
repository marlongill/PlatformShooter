#define OLC_PGE_APPLICATION

#include "olcPixelGameEngine.h"
#include "PlatformShooter.h"
#include "Polygon.h"
#include <cmath>

class test : public olc::PixelGameEngine
{
	std::vector<Poly::Polygon> polygons;

	bool OnUserCreate() override
	{

		Poly::vec2d pos;

		Poly::vec2d v1 = Poly::vec2d(); v1.x = -7; v1.y = -15;
		Poly::vec2d v2 = Poly::vec2d(); v2.x = 15; v2.y = -15;
		Poly::vec2d v3 = Poly::vec2d(); v3.x = 15; v3.y = 15;
		Poly::vec2d v4 = Poly::vec2d(); v4.x = -7; v4.y = 15;
		polygons.push_back(Poly::Polygon());
		polygons[0].AddEdge({ { v1, v2 }, Poly::EDGETYPE::TOP });
		polygons[0].AddEdge({ { v2, v3 }, Poly::EDGETYPE::RIGHT });
		polygons[0].AddEdge({ { v3, v4 }, Poly::EDGETYPE::BOTTOM });
		polygons[0].AddEdge({ { v4, v1 }, Poly::EDGETYPE::LEFT });
		polygons[0].UpdatePosition(16, 16);

		polygons.push_back(Poly::Polygon());
		polygons[1].UpdatePosition(ScreenWidth() / 2, ScreenHeight() / 2);

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Poly::vec2d pos = polygons[0].GetPosition();
		float rotation = polygons[0].GetRotation();

		float oldx = pos.x; float oldy = pos.y;

		if (GetKey(olc::Key::RIGHT).bHeld) {
			pos.x += (20.0f * fElapsedTime) * cos(rotation);
			pos.y += (20.0f * fElapsedTime) * sin(rotation);
		}
		if (GetKey(olc::Key::LEFT).bHeld) {
			pos.x -= (20.0f * fElapsedTime) * cos(rotation);
			pos.y -= (20.0f * fElapsedTime) * sin(rotation);
		}
		if (GetKey(olc::Key::UP).bHeld)
			pos.y -= (20.0f * fElapsedTime);
		if (GetKey(olc::Key::DOWN).bHeld)
			pos.y += (20.0f * fElapsedTime);
		if (GetKey(olc::Key::ESCAPE).bHeld) {
			pos.x = 0; pos.y = 0;
			polygons[0].SetPosition(0.0f);
			polygons[1].ClearEdges();
		};
		if (GetMouse(0).bPressed) {
			Poly::Edge f = polygons[1].GetEdge(polygons[1].GetEdgeCount() - 1, Poly::COORDTYPE::MODEL);
			float x = (float)GetMouseX() - (ScreenWidth() / 2);
			float y = (float)GetMouseY() - (ScreenHeight() / 2);
			if (GetKey(olc::SHIFT).bHeld)
				x = f.line.Midpoint().x;
			if (GetKey(olc::CTRL).bHeld)
				y = f.line.Midpoint().y;

			polygons[1].AddEdge({ {f.line.end, Poly::vec2d{ x, y }}, Poly::EDGETYPE::OTHER });
		};

		polygons[0].SetPosition(pos.x, pos.y);

		Clear(olc::BLACK);
		std::vector<Poly::FACE> checkFaces = { Poly::FACE::LEFT, Poly::FACE::RIGHT, Poly::FACE::CENTER_VERT, Poly::FACE::LEFT_EDGE, Poly::FACE::RIGHT_EDGE };
		std::vector<std::vector<Poly::Intersect>> faces = polygons[0].IntersectingFaces(&polygons[1], checkFaces);

		for (int x = 0; x < polygons.size(); x++)
		{
			Poly::Polygon* poly = &polygons[x];
			Poly::Polygon* poly2 = &polygons[1 - x];

			for (int i = 0; i < poly->GetEdgeCount(); i++)
			{
				Poly::Edge f = poly->GetEdge(i, Poly::COORDTYPE::ROTATED);
				DrawLine(
					olc::vf2d(f.line.start.x, f.line.start.y),
					olc::vf2d(f.line.end.x, f.line.end.y),
					faces[i].size() > 0 && x == 0 ? olc::RED : olc::WHITE
				);
			}
			if (x == 0)
			{
				Poly::line2d l = poly->GetEdge(Poly::FACE::LEFT_EDGE).line;
				DrawLine(olc::vf2d(l.start.x, l.start.y), olc::vf2d(l.end.x, l.end.y), olc::CYAN);
				l = poly->GetEdge(Poly::FACE::RIGHT_EDGE).line;
				DrawLine(olc::vf2d(l.start.x, l.start.y), olc::vf2d(l.end.x, l.end.y), olc::CYAN);
				l = poly->GetEdge(Poly::FACE::CENTER_VERT).line;
				DrawLine(olc::vf2d(l.start.x, l.start.y), olc::vf2d(l.end.x, l.end.y), olc::CYAN);
			}

			// Draw Center Point of Polygon
			Draw(olc::vi2d(poly->GetCentre().x, poly->GetCentre().y));
		}

		// Draw Intersection Calculated correcting offset
		for (auto points : faces) {
			for (auto point : points) {
				FillRect(olc::vf2d(point.point.x - 1, point.point.y - 1), olc::vf2d(3, 3), olc::GREEN);
			}
		}

		// Positional and Rotational Logic is Here 
		Poly::vec2d vl = faces[(int)Poly::FACE::LEFT_EDGE].size() > 0 ? faces[(int)Poly::FACE::LEFT_EDGE][0].point : Poly::vec2d();
		Poly::vec2d vc = faces[(int)Poly::FACE::CENTER_VERT].size() > 0 ? faces[(int)Poly::FACE::CENTER_VERT][0].point : Poly::vec2d();
		Poly::vec2d vr = faces[(int)Poly::FACE::RIGHT_EDGE].size() > 0 ? faces[(int)Poly::FACE::RIGHT_EDGE][0].point : Poly::vec2d();
		float ly = polygons[0].GetEdge(Poly::EDGETYPE::BOTTOM).line.start.y; 
		float ry = polygons[0].GetEdge(Poly::EDGETYPE::BOTTOM).line.end.y;
		
		float dl = vl.y - ly;
		float dc = vc.y - (ly + ((ry - ly) / 2));
		float dr = vr.y - ry;

		std::string status = "";

		//if (dl > 1 && dc > 1 && dr > 1)
		//	status = "Floating";
		//else if (dl != INFINITY || dc != INFINITY || dr != INFINITY)
		//{
		//	float miny = std::min(dl, std::min(dr, dc));
		//	float maxy = std::max(dl == INFINITY ? 0 : dl, std::max(dr == INFINITY ? 0 : dr, dc == INFINITY ? 0 : dc));
		//	float minx = std::min(vl.x, std::min(vr.x, vc.x));
		//	float maxx = std::max(vl.x == INFINITY ? 0 : vl.x, std::max(vr.x == INFINITY ? 0 : vr.x, vc.x == INFINITY ? 0 : vc.x));
		//	float dx = maxx - minx; float dy = maxy - miny;
		//	float lefty = vl.y != INFINITY ? vl.y : vc.y != INFINITY ? vc.y : vr.y;
		//	float righty = vr.y != INFINITY ? vr.y : vc.y != INFINITY ? vc.y : vl.y;

		//	if (miny == maxy)
		//		status = "On Level Ground";
		//	else
		//	{
		//		status = "On Slope";
		//		float slope = dy / dx;
		//		if (slope < 0.6) {
		//			float theta = sinh(dy / sqrtf((dx * dx) + (dy * dy))) * (lefty > righty ? -1.0f : 1.0f);
		//			polygons[0].SetPosition(theta);
		//		}
		//		else 
		//		{
		//			pos.x = oldx;
		//			pos.y = oldy;
		//		}
		//	}
		//}

		DrawString(olc::vf2d(0, 0), status);

		return true;
	}
};

// Override base class with your custom functionality
int main()
{
	//test demo;
	//if (demo.Construct(200, 100, 8, 8))
	//	demo.Start();
	//return 0;

	float g = -1.0f;
	float rad = atan(g) + (2 * 3.1415922 / 2);
	float dx = 0.0f, dy = 100.0f;
	float x = ((dx * cosf(rad)) - (dy * sinf(rad))) + 0;
	float y = ((dx * sinf(rad)) + (dy * cosf(rad))) + 0;

	PlatformShooter::PlatformShooterDemo demo;
	if (demo.Construct(480, 480, 2, 2))
		demo.Start();
	return 0;
}
