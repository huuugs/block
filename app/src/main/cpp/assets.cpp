#include "assets.h"
#include <cmath>
#include <cstdlib>

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
    TraceLog(LOG_INFO, "=== LoadFonts() START [TRY ZPIX FIRST] ===");

    // Try Zpix FIRST - it's smaller and more likely to work on Android
    bool loaded = false;

    TraceLog(LOG_INFO, "Attempting to load Zpix font first...");
    if (!loaded) {
        loaded = LoadExternalFont("fonts/zpix.ttf", 20);
    }

    // Fallback: Source Han Sans
    if (!loaded) {
        TraceLog(LOG_INFO, "Zpix failed, trying Source Han Sans...");
        loaded = LoadExternalFont("fonts/SourceHanSansCN-Regular.otf", 18);
    }

    // Fallback: Vonwaon pixel font
    if (!loaded) {
        TraceLog(LOG_INFO, "Source Han Sans failed, trying vonwaon...");
        loaded = LoadExternalFont("fonts/vonwaon_pixel_12px.ttf", 12);
    }

    // Fallback: use default raylib font if external fonts not available
    if (!loaded) {
        TraceLog(LOG_ERROR, "All external fonts failed!");
        pixelFont = GetFontDefault();
        smallFont = GetFontDefault();
        TraceLog(LOG_WARNING, "Using default font (no Chinese support)");
    } else {
        TraceLog(LOG_INFO, TextFormat("FINAL: Font loaded with %d glyphs", pixelFont.glyphCount));
    }

    TraceLog(LOG_INFO, "=== LoadFonts() END ===");
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
    // Generate space-themed background with stars instead of dizzying grid
    int width = SCREEN_WIDTH;
    int height = SCREEN_HEIGHT;

    Image img = {
        .data = malloc(width * height * 4),
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    // Create space background with stars
    // Deep space gradient (dark blue to black)
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int index = (y * width + x) * 4;

            // Create subtle gradient from top to bottom
            float t = (float)y / (float)height;
            unsigned char r = (unsigned char)(10 * (1.0f - t * 0.5f));
            unsigned char g = (unsigned char)(10 * (1.0f - t * 0.3f));
            unsigned char b = (unsigned char)(25 * (1.0f - t * 0.2f));

            ((unsigned char*)img.data)[index] = r;
            ((unsigned char*)img.data)[index + 1] = g;
            ((unsigned char*)img.data)[index + 2] = b;
            ((unsigned char*)img.data)[index + 3] = 255;
        }
    }

    // Add random stars with varying brightness
    const int numStars = 200;
    for (int i = 0; i < numStars; i++) {
        int x = rand() % width;
        int y = rand() % height;
        int starSize = 1 + (rand() % 2);  // 1-2 pixels
        int brightness = 150 + (rand() % 106);  // 150-255

        for (int dy = 0; dy < starSize && y + dy < height; dy++) {
            for (int dx = 0; dx < starSize && x + dx < width; dx++) {
                int index = ((y + dy) * width + (x + dx)) * 4;
                ((unsigned char*)img.data)[index] = (unsigned char)brightness;
                ((unsigned char*)img.data)[index + 1] = (unsigned char)brightness;
                ((unsigned char*)img.data)[index + 2] = (unsigned char)brightness;
                ((unsigned char*)img.data)[index + 3] = 255;
            }
        }
    }

    // Add a few brighter "stars" with slight blue tint
    const int numBrightStars = 20;
    for (int i = 0; i < numBrightStars; i++) {
        int x = rand() % width;
        int y = rand() % height;

        int index = (y * width + x) * 4;
        ((unsigned char*)img.data)[index] = 200;      // R
        ((unsigned char*)img.data)[index + 1] = 220;  // G
        ((unsigned char*)img.data)[index + 2] = 255;  // B
        ((unsigned char*)img.data)[index + 3] = 255;

        // Add glow around bright stars
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                if (dx == 0 && dy == 0) continue;
                int nx = x + dx;
                int ny = y + dy;
                if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                    int nIndex = (ny * width + nx) * 4;
                    ((unsigned char*)img.data)[nIndex] = 150;
                    ((unsigned char*)img.data)[nIndex + 1] = 170;
                    ((unsigned char*)img.data)[nIndex + 2] = 200;
                    ((unsigned char*)img.data)[nIndex + 3] = 180;
                }
            }
        }
    }

    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

// CLEAN VERSION of LoadExternalFont - Replace the function in assets.cpp with this
bool AssetManager::LoadExternalFont(const char* fontPath, int fontSize) {
    // Use FULL CJK range for complete Chinese character support
    static int* chineseCodepoints = nullptr;
    static int codepointCount = 0;

    if (chineseCodepoints == nullptr) {
        // ASCII (32-126) + Full CJK Unified Ideographs (0x4E00-0x9FFF)
        // This covers ALL common Chinese characters (~21,000 characters)
        const int cjkStart = 0x4E00;
        const int cjkEnd = 0x9FFF;
        const int cjkCount = cjkEnd - cjkStart + 1;
        const int asciiCount = 95; // 32-126

        codepointCount = asciiCount + cjkCount;
        chineseCodepoints = (int*)malloc(codepointCount * sizeof(int));

        int idx = 0;
        // Add ASCII characters first
        for (int i = 32; i <= 126; i++) {
            chineseCodepoints[idx++] = i;
        }
        // Add FULL CJK range (all Chinese characters)
        for (int i = cjkStart; i <= cjkEnd; i++) {
            chineseCodepoints[idx++] = i;
        }

        TraceLog(LOG_INFO, TextFormat("CJK codepoints created: %d total (95 ASCII + %d CJK)",
            codepointCount, cjkCount));
    }

    // Try loading from the given path
    if (FileExists(fontPath)) {
        TraceLog(LOG_INFO, TextFormat("Loading font: %s with %d codepoints", fontPath, codepointCount));
        pixelFont = LoadFontEx(fontPath, fontSize, chineseCodepoints, codepointCount);

        if (pixelFont.texture.id != 0) {
            TraceLog(LOG_INFO, TextFormat("SUCCESS: Font loaded! glyphs=%d", pixelFont.glyphCount));
            SetTextureFilter(pixelFont.texture, TEXTURE_FILTER_BILINEAR);
            GenTextureMipmaps(&pixelFont.texture);

            smallFont = LoadFontEx(fontPath, (int)(fontSize * 0.75f), chineseCodepoints, codepointCount);
            if (smallFont.texture.id != 0) {
                SetTextureFilter(smallFont.texture, TEXTURE_FILTER_BILINEAR);
                GenTextureMipmaps(&smallFont.texture);
            } else {
                smallFont = pixelFont;
            }
            return true;
        }
    }

    // On Android, try alternative paths
    #if defined(PLATFORM_ANDROID)
    const char* altPaths[] = {"fonts/zpix.ttf", "zpix.ttf",
                               "fonts/SourceHanSansCN-Regular.otf", "SourceHanSansCN-Regular.otf"};

    for (const char* path : altPaths) {
        if (FileExists(path)) {
            TraceLog(LOG_INFO, TextFormat("Loading Android font: %s", path));
            pixelFont = LoadFontEx(path, fontSize, chineseCodepoints, codepointCount);
            if (pixelFont.texture.id != 0) {
                TraceLog(LOG_INFO, TextFormat("SUCCESS: Font loaded! glyphs=%d", pixelFont.glyphCount));
                SetTextureFilter(pixelFont.texture, TEXTURE_FILTER_BILINEAR);
                GenTextureMipmaps(&pixelFont.texture);

                smallFont = LoadFontEx(path, (int)(fontSize * 0.75f), chineseCodepoints, codepointCount);
                if (smallFont.texture.id != 0) {
                    SetTextureFilter(smallFont.texture, TEXTURE_FILTER_BILINEAR);
                    GenTextureMipmaps(&smallFont.texture);
                } else {
                    smallFont = pixelFont;
                }
                return true;
            }
        }
    }
    #endif

    TraceLog(LOG_ERROR, "Font loading failed!");
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

bool AssetManager::LoadFontByType(int fontType) {
    // Unload current fonts first
    if (pixelFont.texture.id != 0 && pixelFont.texture.id != GetFontDefault().texture.id) {
        UnloadFont(pixelFont);
    }
    if (smallFont.texture.id != 0 &&
        smallFont.texture.id != GetFontDefault().texture.id &&
        smallFont.texture.id != pixelFont.texture.id) {
        UnloadFont(smallFont);
    }

    // Load new font based on type
    const char* fontPath = nullptr;
    int fontSize = 16;

    switch (fontType) {
        case 0:  // Source Han Sans
            fontPath = "fonts/SourceHanSansCN-Regular.otf";
            fontSize = 18;
            TraceLog(LOG_INFO, "Loading Source Han Sans font");
            break;
        case 1:  // Zpix
            fontPath = "fonts/zpix.ttf";
            fontSize = 16;
            TraceLog(LOG_INFO, "Loading Zpix font");
            break;
        case 2:  // Default
            pixelFont = GetFontDefault();
            smallFont = GetFontDefault();
            TraceLog(LOG_INFO, "Using default font");
            return true;
        default:
            return false;
    }

    return LoadExternalFont(fontPath, fontSize);
}

} // namespace BlockEater
