#pragma once
#include "olcPixelGameEngine.h"
#include "Polygon.h"
#include "Actor_Animation.h"

#include <map>

// Assets
namespace Assets
{
	struct TileAttributes
	{
		bool IsSolid = false;
		float Friction = 1.0;
		int Damage = 0;
		float m = 0;
		float c = 0;
	};

	struct SpriteData
	{
		olc::Sprite* Sprite;
		olc::Decal* Decal;

		int Columns;
		int Rows;
		int TileWidth;
		int TileHeight;
		int SpritesPerRow;

		std::vector<TileAttributes> Attributes; 
		std::vector<Poly::Polygon> PolyMasks;
		std::map<std::string, Actor::Animation> Animations;
	};

	struct MapCell
	{
		int TileID;
		bool IsSolid;
		float Friction;
		int Damage;
		float m, c;

		MapCell() {
			TileID = -1;
			IsSolid = false;
			Friction = 0.0;
			Damage = 0;
			m = 0.0;
			c = 0.0;
		}

		MapCell(int tileID, bool isSolid, float friction, int damage, float gradient)
		{
			TileID = tileID;
			IsSolid = isSolid;
			Friction = friction;
			Damage = damage;
			m = 0.0;
			c = 0.0;
		}
	};

	class Map
	{
	private:
		olc::Decal* TileSheet;
		olc::PixelGameEngine* pge;
		MapCell *Cells;
		SpriteData* TileSheetData;

	public:
		int TileWidth, TileHeight;
		int MapWidth, MapHeight;
		float Gravity;

		olc::vf2d fOrigin;
		olc::vi2d Origin;

	public:
		Map();
		~Map();

		bool Create(std::string mapName, std::string mapFile, std::string tileSheetName, olc::PixelGameEngine* p);

		MapCell GetTileInfo(int x, int y);
		MapCell GetTileInfo(olc::vi2d mapRef);
		MapCell GetTileInfoWorldCoord(olc::vf2d coords);
		Poly::Polygon GetPolygon(int x, int y);
		Poly::Polygon GetPolygon(olc::vi2d mapRef);

		olc::vi2d WorldCoordToMapRef(olc::vi2d worldCoords);
		olc::vf2d MapRefToWorldCoord(olc::vi2d mapRef);

		int WorldWidth();
		int WorldHeight();

		void SetGravity(float g);
		void SetOrigin(olc::vf2d origin);
		void CenterOnPoint(olc::vf2d point);

		void Render(uint8_t layer, bool showMask = false);
	};

	class Assets
	{
	public:
		static Assets& get()
		{
			static Assets me;
			return me;
		}

		Assets(Assets const&) = delete;
		void operator=(Assets const&) = delete;


	private:
		Assets() {}
		~Assets() {}

		std::string ResourcesDirectory = "Resources\\";
		std::map<std::string, SpriteData*> spriteData;
		std::map<std::string, Map*> maps;

	private:
		int ReadIntValue(std::ifstream& ifs, int bytes);
		std::string ReadStringFromStream(std::ifstream& stream);

	public:
		void LoadSpriteSheet(std::string spriteSheetName, bool createDecal);
		void LoadSpriteSheetFromPAK(std::string spriteSheetName, bool createDecal);
		void LoadMap(std::string mapName, std::string tileSheetName, olc::PixelGameEngine* pge);

		SpriteData* GetSpriteData(std::string spriteName);
		olc::Sprite* GetSprite(std::string spriteName);
		olc::Decal* GetDecal(std::string decalName);

		Map* GetMap(std::string mapName);

	};
}

