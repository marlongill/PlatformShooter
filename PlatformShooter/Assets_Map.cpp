#define SHOW_WIRE_FRAME

#include "olcPixelGameEngine.h"
#include "Assets.h"
#include "Text.h"

#include <fstream>

namespace Assets
{
	Map::Map()
	{
		Cells = nullptr;
		TileSheet = nullptr; 
		TileSheetData = nullptr;
		MapHeight = 0; MapWidth = 0;
		TileHeight = 0; TileWidth = 0;
		Origin = olc::vi2d(0, 0);
		fOrigin = olc::vf2d(0.0, 0.0);
	}

	Map::~Map() { 
		delete[] Cells;
	}

	bool Map::Create(std::string mapName, std::string mapFile, std::string tileSheetName, olc::PixelGameEngine* p)
	{
		// Reference to Pixel Game Engine
		pge = p;
		TileSheetData = Assets::get().GetSpriteData(tileSheetName);

		// Basic Variables
		TileHeight = TileSheetData->TileHeight;
		TileWidth = TileSheetData->TileWidth;

		// Get Sprite Pointers
		TileSheet = TileSheetData->Decal;

		int blankCellID = 0;

		// Load Map File
		std::ifstream mapData(mapFile + ".map", std::ios::in | std::ios::binary);
		if (mapData.is_open())
		{
			std::string tile;
			mapData >> MapWidth >> MapHeight;
			Cells = new MapCell[MapWidth * MapHeight];

			for (int i = 0; i < MapWidth * MapHeight; i++)
			{
				mapData >> tile;
				int mapTileID = std::stoi(tile, nullptr, 10);
				if (mapTileID == -1) mapTileID = blankCellID;

				Cells[i].TileID = mapTileID;
				Cells[i].Damage = TileSheetData->Attributes[mapTileID].Damage;
				Cells[i].Friction = TileSheetData->Attributes[mapTileID].Friction;
				Cells[i].IsSolid = TileSheetData->Attributes[mapTileID].IsSolid;
				Cells[i].m = TileSheetData->Attributes[mapTileID].m;
				Cells[i].c = TileSheetData->Attributes[mapTileID].c;
			}

			mapData.close();
		}
		else
			return false;

		return true;
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

#ifdef SHOW_WIRE_FRAME
				if (TileSheetData->Attributes[cell.TileID].IsSolid)
				{
					pge->SetDrawTarget((uint8_t)0);
					int ox = screenCoords.x;
					int oy = screenCoords.y;
					for (int i = 0; i < TileSheetData->PolyMasks[cell.TileID].GetEdgeCount(); i++)
					{
						Poly::Edge f = TileSheetData->PolyMasks[cell.TileID].GetEdge(i, Poly::COORDTYPE::MODEL);
						pge->DrawLine(
							olc::vi2d( f.line.start.x + ox, f.line.start.y + oy ),
							olc::vi2d( f.line.end.x + ox, f.line.end.y + oy ),
							f.type == Poly::EDGETYPE::TOP ? olc::YELLOW : olc::GREEN);
					}
					if (cell.TileID > 0)
					{
						char textString[128];
						snprintf(textString, 128, "%d\n%d\n%d", (x + tileX), (y + tileY), cell.TileID);
						Text::TextRenderer Text = Text::TextRenderer(32, 111, "Typeface", pge);
						Text.RenderString(screenCoords.x + 2, screenCoords.y + 2, textString, 0.25f);
					}
				}
#endif
			}
		}
	}
}