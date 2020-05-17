#include "olcPixelGameEngine.h"
#include "PlatformShooter.h"
#include "Enemies.h"
#include "Assets.h"

namespace PlatformShooter
{
    bool PlatformShooterDemo::LoadResources()
    {
        // Load Sprite Sheets
        auto LoadSpriteSheetFromPAK = [&](std::string PAKFileName, bool createDecal)
        {
            Assets::Assets::get().LoadSpriteSheetFromPAK(PAKFileName, createDecal);
        };

        auto LoadSpriteSheet = [&](std::string spriteSheetName, bool createDecal)
        {
            Assets::Assets::get().LoadSpriteSheet(spriteSheetName, createDecal);
        };
        
        auto LoadMap = [&](std::string mapName, std::string tileSheetName, olc::PixelGameEngine* pge)
        {
            Assets::Assets::get().LoadMap(mapName, tileSheetName, pge);
        };

        LoadSpriteSheetFromPAK("Player2", true);
        LoadSpriteSheet("TestTile", true);
        LoadSpriteSheet("Typeface", true);
        //LoadSpriteSheet("Player", true);
        LoadMap("TestMap", "TestTile", this);

        return true;
    }
}