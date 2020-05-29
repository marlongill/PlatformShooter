#include "Assets.h"
#include "math.h"

namespace Assets
{
	const float LOW_FRICTION = 0.25f;
	const float MEDIUM_FRICTION = 0.5f;

	int Assets::ReadIntValue(std::ifstream& ifs, int bytes) {
		int value = 0;
		char byte;

		for (int i = 0; i < bytes; i++)
		{
			ifs.read(&byte, 1);
			value += pow(2, (i * 8)) * ((int)byte < 0 ? 256 + (int)byte : (int)byte);
		}

		return value;
	}

	std::string Assets::ReadStringFromStream(std::ifstream& stream) {
		std::string result;
		char ch;
		while ((ch = stream.get()) != '\0') {
			result += ch;
		}
		return result;
	}

	void Assets::LoadPAKFile(PAKFileType fileType, std::string pakFileName, bool createDecal, olc::PixelGameEngine* pge) {
		
		SpriteData* sd = new SpriteData();

		// Open PAK file
		std::ifstream pakData(ResourcesDirectory + pakFileName + ".pak", std::ios::in | std::ios::binary);
		if (pakData.is_open())
		{
			std::string buf[128];

			// Get Sprite Sheet Image 
			std::vector<char> img;
			int imgSize = ReadIntValue(pakData, 4);

			img.resize(imgSize);
			pakData.read(img.data(), img.size());

			sd->Sprite = new olc::Sprite(img);

			// Create decal if required
			if (createDecal)
				sd->Decal = new olc::Decal(sd->Sprite);

			// Sprite Dimensions
			sd->TileWidth = ReadIntValue(pakData, 2);
			sd->TileHeight = ReadIntValue(pakData, 2);
			sd->SpritesPerRow = sd->Sprite->width / sd->TileWidth;
			sd->Columns = sd->SpritesPerRow;
			sd->Rows = sd->Sprite->height / sd->TileHeight;

			// Attribute Names
			std::vector<std::string> attributeNames;
			int attributeCount = ReadIntValue(pakData, 1);
			for (int i = 0; i < attributeCount; i++)
				attributeNames.push_back(ReadStringFromStream(pakData));

			// Attribute Entries
			int attributeEntryCount = ReadIntValue(pakData, 2);
			sd->Attributes.resize(attributeEntryCount);
			for (int i = 0; i < attributeEntryCount; i++)
			{
				TileAttributes ta;
				for (int x = 0; x < attributeCount; x++)
				{
					std::string value = ReadStringFromStream(pakData);
					if (attributeNames[x] == "Solid")
						ta.IsSolid = value == "True" ? true : false;
					else if (attributeNames[x] == "Friction")
						ta.Friction = value == "H" ? 1.0 : value == "M" ? MEDIUM_FRICTION : LOW_FRICTION;
					else if (attributeNames[x] == "Damage")
						ta.Damage = stoi(value == "" ? "0" : value);
				}
				sd->Attributes[i] = ta;
			}

			// Animations
			int animCount = ReadIntValue(pakData, 4);
			int frameSize = ReadIntValue(pakData, 2);

			sd->Animations.clear();

			for (int i = 0; i < animCount; i++) {
				Actor::Animation anim;
				
				std::string name = ReadStringFromStream(pakData);
				anim.FPS = ReadIntValue(pakData, 1);
				anim.Loop = ReadIntValue(pakData, 1) == 1 ? true : false;

				int frameCount = ReadIntValue(pakData, 1);
				anim.Frames.resize(frameCount);
				for (int x = 0; x < frameCount; x++) {
					anim.Frames[x] = ReadIntValue(pakData, frameSize);
				}

				sd->Animations.emplace(name, anim);
			}

			// Scenery Collision Polygons
			std::vector<Poly::line2d> lines;
			lines.push_back({ {0, 0},{0, 0} }); // Default line for empty polygons

			// Load lines array first 
			int lineCount = ReadIntValue(pakData, 4);
			int linePointSize = ReadIntValue(pakData, 1);

			for (int i = 0; i < lineCount; i++)
				lines.push_back(Poly::line2d(
					{ (float)ReadIntValue(pakData, linePointSize), (float)ReadIntValue(pakData, linePointSize) },
					{ (float)ReadIntValue(pakData, linePointSize), (float)ReadIntValue(pakData, linePointSize) })
				);

			// Now create collision masks for each tile
			int maskCount = ReadIntValue(pakData, 4);
			sd->PolyMasks.resize(maskCount);
			for (int i = 0; i < maskCount; i++)
			{
				Poly::Polygon poly;
				int faceCount = ReadIntValue(pakData, 1);
				for (int f = 0; f < faceCount; f++)
				{
					int lineID = ReadIntValue(pakData, 2) + 1;

					Poly::Edge edge;
					edge.line = lines[lineID];

					char type;
					pakData >> type;
					if (type == 'T') edge.type = Poly::EDGETYPE::TOP; 
					else if (type == 'B') edge.type = Poly::EDGETYPE::BOTTOM;
					else if (type == 'L') edge.type = Poly::EDGETYPE::LEFT;
					else if (type == 'R') edge.type = Poly::EDGETYPE::RIGHT;
					else if (type == 'O') edge.type = Poly::EDGETYPE::OTHER;

					poly.AddEdge(edge);

				}
				sd->PolyMasks[i] = poly;
			}

			// The next sections only relate to Map PAK files
			if (fileType == PAKFileType::Map)
			{
				Map* map = new Map();

				// Map Size
				int mapWidth = ReadIntValue(pakData, 2);
				int mapHeight = ReadIntValue(pakData, 2);

				map->Create(sd, mapWidth, mapHeight, pge);

				// Map Cells
				for (int i = 0; i < mapWidth * mapHeight; i++)
				{
					int tileID = ReadIntValue(pakData, 2);
					
					MapCell cell = MapCell();
					cell.TileID = tileID;
					cell.Damage = sd->Attributes[tileID].Damage;
					cell.Friction = sd->Attributes[tileID].Friction;
					cell.IsSolid = sd->Attributes[tileID].IsSolid;

					map->SetTileInfo(i, cell);
				}

				// Map Polygon Masks
				int maskCount = ReadIntValue(pakData, 2);
				map->SetMaskCount(maskCount);

				for (int i = 0; i < maskCount; i++)
				{
					Poly::Polygon poly;
					Poly::vec2d firstPoint, previousPoint;

					int pointCount = ReadIntValue(pakData, 2);
					for (int p = 0; p < pointCount; p++)
					{
						int x = ReadIntValue(pakData, 2);
						int y = ReadIntValue(pakData, 2);

						if (p == 0) firstPoint = { (float)x, (float)y };
						else poly.AddEdge({ { Poly::vec2d(previousPoint), Poly::vec2d({ (float)x, (float)y }) }, Poly::EDGETYPE::OTHER });
						
						previousPoint = { (float)x, (float)y };
					}
					poly.AddEdge({ { Poly::vec2d(previousPoint), Poly::vec2d(firstPoint) }, Poly::EDGETYPE::OTHER });

					map->AddMask(i, poly);
				}

				// Add map to list
				maps.emplace(pakFileName, map);
			}

		}

		spriteData.emplace(pakFileName, sd);
	}

	SpriteData* Assets::GetSpriteData(std::string spriteName)
	{
		SpriteData* sd = nullptr;
		std::map<std::string, SpriteData*>::iterator result = spriteData.find(spriteName);
		if (result != spriteData.end())
			sd = spriteData[spriteName];
		return sd;
	}

	olc::Sprite* Assets::GetSprite(std::string spriteName)
	{
		std::map<std::string, SpriteData*>::iterator result = spriteData.find(spriteName);
		if (result != spriteData.end())
			return spriteData[spriteName]->Sprite;
		else
			return nullptr;
	}

	olc::Decal* Assets::GetDecal(std::string decalName)
	{
		std::map<std::string, SpriteData*>::iterator result = spriteData.find(decalName);
		if (result != spriteData.end())
			return spriteData[decalName]->Decal;
		else
			return nullptr;
	}

	Map* Assets::GetMap(std::string mapName)
	{
		Map* map = nullptr;
		std::map<std::string, Map*>::iterator result = maps.find(mapName);
		if (result != maps.end())
			map = maps[mapName];
		return map;

	}
}