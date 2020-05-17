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
		//std::string s;
		//std::getline(stream, s, '\0');
		//return s;
	}

	void Assets::LoadSpriteSheetFromPAK(std::string pakFileName, bool createDecal) {
		
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
			lines.push_back({ {0,0},{0,0} }); // Default line for empty polygons

			// Load lines array first 
			int lineCount = ReadIntValue(pakData, 4);
			int linePointSize = ReadIntValue(pakData, 1);

			for (int i = 0; i < lineCount; i++)
				lines.push_back(Poly::line2d(
					{ (float)ReadIntValue(pakData, linePointSize), (float)ReadIntValue(pakData, linePointSize) },
					{ (float)ReadIntValue(pakData, linePointSize), (float)ReadIntValue(pakData, linePointSize)})
				);

			// Now create collision masks for each animation frame
			int maskCount = ReadIntValue(pakData, 4);
			sd->PolyMasks.resize(maskCount);
			for (int i = 0; i < maskCount; i++)
			{
				Poly::Polygon poly;
				int faceCount = ReadIntValue(pakData, 1);
				for (int f = 0; f < faceCount; f++)
				{
					int lineID = ReadIntValue(pakData, 2) + 1;

					int typeCount = ReadIntValue(pakData, 1);
					for (int t = 0; t < typeCount; t++)
					{
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
				}
				sd->PolyMasks[i] = poly;
			}
		}

		spriteData.emplace(pakFileName, sd);
	}

	void Assets::LoadSpriteSheet(std::string spriteSheetName, bool createDecal)	{
		SpriteData* sd = new SpriteData();
		sd->Sprite = new olc::Sprite(ResourcesDirectory + spriteSheetName + ".spr");

		if (createDecal)
			sd->Decal = new olc::Decal(sd->Sprite);

		std::ifstream metaData(ResourcesDirectory + spriteSheetName + ".tilemeta", std::ios::in | std::ios::binary);
		if (metaData.is_open())
		{
			std::string dataType;
			std::string buf[10];
			olc::vf2d origin;
			Poly::Polygon defaultPoly;
			std::vector<Poly::line2d> lines;
			std::vector<Poly::Polygon> polygons;

			lines.push_back({ {0,0},{0,0} });

			while (!metaData.eof())
			{
				int attCount, attributeEntryCount, polyCount;

				metaData >> dataType;

				if (dataType == "t") {
					metaData >> buf[0] >> buf[1] >> buf[2] >> buf[3];
					sd->Columns = std::stoi(buf[0], nullptr, 10);
					sd->Rows = std::stoi(buf[1], nullptr, 10);
					sd->TileWidth = std::stoi(buf[2], nullptr, 10);
					sd->TileHeight = std::stoi(buf[3], nullptr, 10);

					// Create default collision polygons. Default has no points and will be 
					// updated later if we are told otherwise in the metadata file
					origin = olc::vf2d(sd->TileWidth / 2, sd->TileHeight / 2);

					for (int i = 0; i < sd->Columns * sd->Rows; i++)
						sd->PolyMasks.push_back(Poly::Polygon());
				}
				else if (dataType == "a") {
					metaData >> buf[0];
					attCount = std::stoi(buf[0], nullptr, 10);
					if (attCount != 0)
					{
						metaData >> buf[0];
						attributeEntryCount = std::stoi(buf[0], nullptr, 10);
						for (int i = 0; i < attributeEntryCount; i++)
						{
							for (int j = 0; j < attCount; j++)
							{
								metaData >> buf[j];
							}
							TileAttributes ta;
							ta.IsSolid = attCount > 0 ? (buf[0] == "Y" ? true : false) : false;
							ta.Friction = attCount > 1 ? (buf[1] == "H" ? 1.0 : buf[1] == "M" ? MEDIUM_FRICTION : LOW_FRICTION) : 1.0f;
							ta.Damage = attCount > 2 ? std::stoi(buf[2], nullptr, 10) : 0;
							sd->Attributes.push_back(ta);
						}
					}
				}
				else if (dataType == "f") {
					metaData >> buf[0];
					int lineCount = std::stoi(buf[0]);

					for (int x = 0; x < lineCount; x++)
					{
						metaData >> buf[0];
						float x1, x2, y1, y2;
						int l = std::stoi(buf[0]);
						y2 = l & 31;
						l = l >> 5; x2 = l & 31;
						l = l >> 5; y1 = l & 31;
						l = l >> 5; x1 = l & 31;
						lines.push_back({ { x1, y1 }, { x2, y2 } });
					}
				}
				else if (dataType == "p") {
					metaData >> buf[0];
					int polyCount = std::stoi(buf[0]);

					for (int x = 0; x < polyCount; x++)
					{
						std::vector<Poly::Edge> pf;

						metaData >> buf[0];
						int f = std::stoi(buf[0]);

						for (int y = 0; y < 5; y++)
						{
							Poly::EDGETYPE ft = Poly::EDGETYPE::OTHER;
							switch (y) {
							case 0: ft = Poly::EDGETYPE::TOP; break;
							case 1: ft = Poly::EDGETYPE::RIGHT; break;
							case 2: ft = Poly::EDGETYPE::BOTTOM; break;
							case 3: ft = Poly::EDGETYPE::LEFT; break;
							case 4: ft = Poly::EDGETYPE::OTHER; break;
							}
							int lineID = f & 63;
							if (lineID != 0)
								pf.push_back({ {lines[lineID].start,lines[lineID].end}, ft });

							f = f >> 6;
						}

						polygons.push_back(pf);
					}
				}
				else if (dataType == "m") {
					
					metaData >> buf[0];
					int maskCount = std::stoi(buf[0]);

					while (maskCount > 0)
					{
						metaData >> buf[0];
						int tileInt = std::stoi(buf[0]);

						int polyID = tileInt & 255;
						tileInt = tileInt >> 8;
						int tileID = tileInt & 255;

						sd->PolyMasks[tileID] = polygons[polyID];

						maskCount--;
					}
				}
			}
		}

		spriteData.emplace(spriteSheetName, sd);
	}

	void Assets::LoadMap(std::string mapName, std::string tileSheetName, olc::PixelGameEngine* pge)
	{
		Map* map = new Map();
		map->Create(mapName, ResourcesDirectory + mapName, tileSheetName, pge);
		maps.emplace(mapName, map);
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