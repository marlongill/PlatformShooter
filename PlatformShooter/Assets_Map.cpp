#define SHOW_WIRE_FRAME

#include "olcPixelGameEngine.h"
#include "Assets.h"
#include "Text.h"

#include <fstream>

namespace Assets
{
	Map::Map()
	{
		Cells.resize(0);
		TileSheet = nullptr; 
		TileSheetData = nullptr;
		Masks.resize(0);
		
		MapHeight = 0; MapWidth = 0;
		TileHeight = 0; TileWidth = 0;
		Origin = olc::vi2d(0, 0);
		fOrigin = olc::vf2d(0.0, 0.0);
	}

	Map::~Map() { 
		Cells.clear();
		Masks.clear();
	}

	bool Map::Create(SpriteData* sd, int mapWidth, int mapHeight, olc::PixelGameEngine* p)
	{
		// Reference to Pixel Game Engine
		pge = p;
		TileSheetData = sd;

		// Basic Variables
		TileHeight = TileSheetData->TileHeight;
		TileWidth = TileSheetData->TileWidth;

		MapWidth = mapWidth;
		MapHeight = mapHeight;
		Cells.resize(mapWidth * mapHeight);

		// Get Sprite Pointers
		TileSheet = TileSheetData->Decal;
		
		return true;
	}

	void Map::SetMaskCount(int maskCount)
	{
		Masks.resize(maskCount);
	}

	void Map::AddMask(int index, Poly::Polygon poly)
	{
		Masks[index] = poly;
	}

	void Map::SetTileInfo(int index, MapCell cellInfo)
	{
		Cells[index] = cellInfo;
	}

	MapCell Map::GetTileInfo(int x, int y)
	{
		if (y < 0 || y >= MapHeight || x < 0 || x > MapWidth)
			return MapCell();
		else
			return Cells[(y * MapWidth) + x];
	}

	MapCell Map::GetTileInfo(olc::vi2d mapRef)
	{
		return GetTileInfo(mapRef.x, mapRef.y);
	}

	Poly::Polygon Map::GetPolygon(int x, int y)
	{
		if (y < 0 || y >= MapHeight || x < 0 || x > MapWidth)
			return Poly::Polygon();

		MapCell cell = GetTileInfo(x, y);
		if (!cell.IsSolid)
			return Poly::Polygon();

		Poly::Polygon poly = TileSheetData->PolyMasks[cell.TileID];
		poly.UpdatePosition((x * TileWidth), (y * TileHeight));

		return poly;
	}

	Poly::Polygon Map::GetPolygon(olc::vi2d mapRef)
	{
		return GetPolygon(mapRef.x, mapRef.y);
	}

	olc::vi2d Map::WorldCoordToMapRef(olc::vi2d worldCoords)
	{
		return olc::vi2d(floor(worldCoords.x / TileWidth), floor(worldCoords.y / TileHeight));
	}
	olc::vf2d Map::MapRefToWorldCoord(olc::vi2d mapRef)
	{
		return olc::vf2d(mapRef.x * TileWidth, mapRef.y * TileHeight);
	}

	Poly::line2d Map::GetIntersectingMaskFace(Poly::line2d l)
	{
		float minX = std::min(l.start.x, l.end.x);
		float maxX = std::max(l.start.x, l.end.x);
		float minY = std::min(l.start.y, l.end.y);

		for (auto mask : Masks)
		{
			Poly::rect2d aabb = mask.GetAABB();
			if ((minY >= aabb.tl.y && minY <= aabb.br.y) && 
				((minX >= aabb.tl.x &&  minX <= aabb.br.x) || (maxX >= aabb.tl.x && maxX <= aabb.br.x)))
			{
				for (auto edge : mask.GetFaces())
				{
					try
					{
						Poly::vec2d intersect = edge.line.Intersect(l);
						return edge.line;
					}
					catch(Poly::InvalidIntersect) { }
				}
			}
		}
		return { { -INFINITY, -INFINITY }, {-INFINITY, -INFINITY} };
	}

	MapCell Map::GetTileInfoWorldCoord(olc::vf2d coords)
	{
		return GetTileInfo(WorldCoordToMapRef(coords));
	}

	int Map::WorldWidth() { return TileWidth * MapWidth; }
	int Map::WorldHeight() { return TileHeight * MapHeight; }

	void Map::SetGravity(float g) { Gravity = g; };
	void Map::SetOrigin(olc::vf2d origin) {
		fOrigin = origin;
		Origin = { (int)floor(origin.x), (int)floor(origin.y) };
	}
	void Map::CenterOnPoint(olc::vf2d point) {
		float ox = std::max(point.x - (pge->ScreenWidth() / 2), 0.0f);
		ox = std::min(ox, float(WorldWidth() - pge->ScreenWidth()));
		float oy = std::max(point.y - (pge->ScreenHeight() / 2), 0.0f);
		oy = std::min(oy, float(WorldHeight() - pge->ScreenHeight()));

		fOrigin = { ox, oy };
		Origin = { (int)floor(ox), (int)floor(oy) };
	}

	void Map::Render(uint8_t layer, bool showMask)
	{
		pge->SetDrawTarget(layer);

		float ox = Origin.x, oy = Origin.y;

		int tileX = floor(ox / TileWidth);
		int tileY = floor(oy / TileHeight);
		int cols = ceil(pge->ScreenWidth() / TileWidth) + ((int)ox % TileWidth == 0 ? 0 : 1);
		int rows = ceil(pge->ScreenHeight() / TileHeight) + ((int)oy % TileHeight == 0 ? 0 : 1);
		int drawX = 0 - ((int)ox % 32);
		int drawY = 0 - ((int)oy % 32);

		for (int y = 0; y < rows; y++)
		{
			for (int x = 0; x < cols; x++)
			{
				pge->SetDrawTarget(3);
				MapCell cell = GetTileInfo(x + tileX, y + tileY);
				olc::vi2d screenCoords = olc::vi2d(x * TileWidth + drawX, y * TileHeight + drawY);
				olc::vi2d textureCoords = olc::vi2d(cell.TileID % 10 * TileWidth, cell.TileID / 10 * TileHeight);
				if (cell.TileID > 0)
					pge->DrawPartialDecal(screenCoords, TileSheet, textureCoords, olc::vi2d(TileWidth, TileHeight));
			}
		}

#ifdef SHOW_WIRE_FRAME
		pge->SetDrawTarget((uint8_t)0);
		for (int m = 0; m < Masks.size(); m++)
		{
			for (Poly::Edge e : Masks[m].GetFaces())
			{
				Poly::line2d l = e.line;
				if (((l.start.x >= fOrigin.x && l.start.x <= fOrigin.x + pge->ScreenWidth() && (l.start.y >= fOrigin.y && l.start.y <= fOrigin.y + pge->ScreenHeight())))
					|| ((l.end.x >= fOrigin.x && l.end.x <= fOrigin.x + pge->ScreenWidth() && (l.end.y >= fOrigin.y && l.end.y <= fOrigin.y + pge->ScreenHeight()))))
				{
					pge->DrawLine(
						olc::vi2d(l.start.x - ox, l.start.y - oy),
						olc::vi2d(l.end.x - ox, l.end.y - oy),
						olc::GREEN
					);
				}
			}
		}
		//int ox = screenCoords.x;
		//int oy = screenCoords.y;
		//for (int i = 0; i < TileSheetData->PolyMasks[cell.TileID].GetEdgeCount(); i++)
		//{
		//	Poly::Edge f = TileSheetData->PolyMasks[cell.TileID].GetEdge(i, Poly::COORDTYPE::MODEL);
		//	pge->DrawLine(
		//		olc::vi2d(f.line.start.x + ox, f.line.start.y + oy),
		//		olc::vi2d(f.line.end.x + ox, f.line.end.y + oy),
		//		f.type == Poly::EDGETYPE::TOP ? olc::YELLOW : olc::GREEN);
		//}
		//if (cell.TileID > 0)
		//{
		//	char textString[128];
		//	snprintf(textString, 128, "%d\n%d\n%d", (x + tileX), (y + tileY), cell.TileID);
		//	Text::TextRenderer Text = Text::TextRenderer(32, 111, "Typeface", pge);
		//	Text.RenderString(screenCoords.x + 2, screenCoords.y + 2, textString, 0.25f);
		//}
#endif
	}
}