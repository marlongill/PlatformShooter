#include "Text.h"

namespace Text
{
	TextRenderer::TextRenderer() {}

	TextRenderer::TextRenderer(int firstChar, int lastChar, std::string spriteSheetName, olc::PixelGameEngine* p)
	{
		pge = p;
		spriteSheetData = Assets::Assets::get().GetSpriteData(spriteSheetName);
		Typeface = spriteSheetData->Decal;
		TileWidth = spriteSheetData->TileWidth;
		TileHeight = spriteSheetData->TileHeight; 
		TilesPerRow = spriteSheetData->Columns;
		FirstChar = firstChar; LastChar = lastChar;
	}

	void TextRenderer::RenderString(int x, int y, std::string text, float scale)
	{
		pge->SetPixelMode(olc::Pixel::Mode::ALPHA);

		int charIndex = 0;
		int tx = x;
		int ty = y;
		for (char& c : text)
		{
			if (c == 13 || c == 10) {
				ty += TileHeight * scale * 0.9;
				tx = x;
			}
			else {
				if (c < FirstChar || c > LastChar) c = FirstChar;
				c -= FirstChar;

				int sprXPos = (c % TilesPerRow) * TileWidth;
				int sprYPos = (c / TilesPerRow) * TileHeight;
				pge->DrawPartialDecal(olc::vf2d(tx, ty), Typeface, olc::vf2d(sprXPos, sprYPos), olc::vf2d(TileWidth, TileHeight), { scale, scale });

				tx += TileWidth * scale * 0.9;
			}
		}

		pge->SetPixelMode(olc::Pixel::Mode::NORMAL);
	}
}