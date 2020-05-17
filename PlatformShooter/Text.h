#pragma once
#include "olcPixelGameEngine.h"
#include "Assets.h"

namespace Text
{
	class TextRenderer
	{
	private:
		olc::PixelGameEngine* pge;
		Assets::SpriteData* spriteSheetData;
		olc::Decal* Typeface;
		int TileWidth, TileHeight, TilesPerRow;
		int FirstChar, LastChar;

	public:
		TextRenderer();
		TextRenderer(int firstChar, int lastChar, std::string spriteSheetName, olc::PixelGameEngine* p);
		void RenderString(int x, int y, std::string text, float scale = 1.0f);
	};
}
