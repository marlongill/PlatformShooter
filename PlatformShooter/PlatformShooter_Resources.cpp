#include "olcPixelGameEngine.h"
#include "PlatformShooter.h"
#include "Enemies.h"
#include "Assets.h"

namespace PlatformShooter
{
    bool PlatformShooterDemo::LoadResources()
    {
        // Load Sprite Sheets
        auto LoadSpriteSheetPAK = [&](std::string spriteSheetName, bool createDecal)
        {
            Assets::Assets::get().LoadPAKFile(Assets::PAKFileType::SpriteSheet, spriteSheetName, createDecal, this);
        };
        
        auto LoadMapPAK = [&](std::string mapName, bool createDecal)
        {
            Assets::Assets::get().LoadPAKFile(Assets::PAKFileType::Map, mapName, createDecal, this);
        };

        LoadSpriteSheetPAK("Player", true);
        LoadMapPAK("Map", true);
        LoadSpriteSheetPAK("Text", true);

        return true;
    }
}