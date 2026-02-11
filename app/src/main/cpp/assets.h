#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"
#include "game.h"

namespace BlockEater {

class AssetManager {
public:
    AssetManager();
    ~AssetManager();

    void init();
    void shutdown();

    // Procedurally generate pixel art assets
    Texture2D GeneratePixelBlock(Color color, int size);
    Texture2D GeneratePixelBackground();
    Texture2D GeneratePixelGrid();

    // Font loading
    void LoadFonts();
    Font& GetPixelFont() { return pixelFont; }
    Font& GetSmallFont() { return smallFont; }

    // External font loading for Chinese support
    bool LoadExternalFont(const char* fontPath, int fontSize);
    void UnloadExternalFonts();

    // Reload specific font by type (for font switching)
    bool LoadFontByType(int fontType);  // 0=SourceHanSans, 1=Zpix, 2=Default

    // Color palettes
    static constexpr Color PALETTE_RETRO[] = {
        {0, 0, 0, 255},           // Black
        {255, 255, 255, 255},     // White
        {170, 0, 0, 255},         // Red
        {0, 170, 0, 255},         // Green
        {170, 85, 0, 255},        // Brown
        {0, 85, 170, 255},        // Blue
        {170, 0, 170, 255},       // Magenta
        {0, 170, 170, 255},       // Cyan
        {170, 170, 170, 255},     // Gray
        {85, 85, 85, 255},        // Dark Gray
        {255, 85, 85, 255},       // Light Red
        {85, 255, 85, 255},       // Light Green
        {255, 255, 85, 255},      // Yellow
        {85, 85, 255, 255},       // Light Blue
        {255, 85, 255, 255},      // Light Magenta
        {85, 255, 255, 255}       // Light Cyan
    };

private:
    Font pixelFont;
    Font smallFont;

    // Helper functions
    Image CreatePixelBlockImage(Color color, int size);
    Image CreateGridImage(int cellSize, Color gridColor, Color bgColor);
};

} // namespace BlockEater

#endif // ASSETS_H
