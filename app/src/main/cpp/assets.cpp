#include "assets.h"
#include <cmath>

namespace BlockEater {

AssetManager::AssetManager() : pixelFont{0}, smallFont{0} {
}

AssetManager::~AssetManager() {
    UnloadExternalFonts();
}

void AssetManager::init() {
    LoadFonts();
}

void AssetManager::shutdown() {
    UnloadExternalFonts();
}

void AssetManager::LoadFonts() {
    // First try to load external font with Chinese support
    bool loaded = false;
    
    // Try to load Vonwaon pixel font (Chinese support)
    if (!loaded) {
        loaded = LoadExternalFont("fonts/vonwaon_pixel_12px.ttf", 12);
    }
    
    // Fallback: try to load Source Han Sans if available
    if (!loaded) {
        loaded = LoadExternalFont("fonts/SourceHanSansCN-Regular.otf", 16);
    }
    
    // Fallback: use default raylib font if external fonts not available
    if (!loaded) {
        pixelFont = GetFontDefault();
        smallFont = GetFontDefault();
        TraceLog(LOG_INFO, "Using default font (no Chinese support)");
    } else {
        TraceLog(LOG_INFO, "External font loaded successfully");
    }
}

Image AssetManager::CreatePixelBlockImage(Color color, int size) {
    Image img = {
        .data = malloc(size * size * 4),
        .width = size,
        .height = size,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    // Create pixel block with slight pixelation effect
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            // Add pixelated edge effect
            int edgeSize = 2;
            bool isEdge = (x < edgeSize || x >= size - edgeSize ||
                          y < edgeSize || y >= size - edgeSize);

            Color pixelColor = color;
            if (isEdge) {
                pixelColor.r = (unsigned char)(color.r * 0.7f);
                pixelColor.g = (unsigned char)(color.g * 0.7f);
                pixelColor.b = (unsigned char)(color.b * 0.7f);
            }

            // Add highlight
            if (x < size / 3 && y < size / 3) {
                pixelColor.r = (unsigned char)fmin(255, color.r + 40);
                pixelColor.g = (unsigned char)fmin(255, color.g + 40);
                pixelColor.b = (unsigned char)fmin(255, color.b + 40);
            }

            int index = (y * size + x) * 4;
            ((unsigned char*)img.data)[index] = pixelColor.r;
            ((unsigned char*)img.data)[index + 1] = pixelColor.g;
            ((unsigned char*)img.data)[index + 2] = pixelColor.b;
            ((unsigned char*)img.data)[index + 3] = pixelColor.a;
        }
    }

    return img;
}

Texture2D AssetManager::GeneratePixelBlock(Color color, int size) {
    Image img = CreatePixelBlockImage(color, size);
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Image AssetManager::CreateGridImage(int cellSize, Color gridColor, Color bgColor) {
    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;

    Image img = {
        .data = malloc(width * height * 4),
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    // Fill with background color
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            bool isGridLine = (x % cellSize == 0) || (y % cellSize == 0);

            int index = (y * width + x) * 4;
            if (isGridLine) {
                ((unsigned char*)img.data)[index] = gridColor.r;
                ((unsigned char*)img.data)[index + 1] = gridColor.g;
                ((unsigned char*)img.data)[index + 2] = gridColor.b;
                ((unsigned char*)img.data)[index + 3] = 40;  // Semi-transparent
            } else {
                ((unsigned char*)img.data)[index] = bgColor.r;
                ((unsigned char*)img.data)[index + 1] = bgColor.g;
                ((unsigned char*)img.data)[index + 2] = bgColor.b;
                ((unsigned char*)img.data)[index + 3] = bgColor.a;
            }
        }
    }

    return img;
}

Texture2D AssetManager::GeneratePixelGrid() {
    Image img = CreateGridImage(40, {100, 100, 150, 255}, {20, 20, 40, 255});
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

Texture2D AssetManager::GeneratePixelBackground() {
    return GeneratePixelGrid();
}

bool AssetManager::LoadExternalFont(const char* fontPath, int fontSize) {
    if (FileExists(fontPath)) {
        pixelFont = LoadFontEx(fontPath, fontSize, 0, 0);
        if (pixelFont.texture.id != 0) {
            // Generate mipmap for better quality
            GenTextureMipmaps(&pixelFont.texture);
            
            // Create a smaller version for UI elements
            smallFont = LoadFontEx(fontPath, fontSize * 0.75, 0, 0);
            if (smallFont.texture.id != 0) {
                GenTextureMipmaps(&smallFont.texture);
            } else {
                smallFont = pixelFont;  // Fallback to main font
            }
            
            TraceLog(LOG_INFO, TextFormat("Font loaded from %s", fontPath));
            return true;
        }
    }
    return false;
}

void AssetManager::UnloadExternalFonts() {
    // Only unload if it's not the default font
    if (pixelFont.texture.id != 0 && pixelFont.texture.id != GetFontDefault().texture.id) {
        UnloadFont(pixelFont);
    }
    if (smallFont.texture.id != 0 && 
        smallFont.texture.id != GetFontDefault().texture.id && 
        smallFont.texture.id != pixelFont.texture.id) {
        UnloadFont(smallFont);
    }
    pixelFont = GetFontDefault();
    smallFont = GetFontDefault();
}

} // namespace BlockEater
