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

// LoadExternalFont using LoadFileData + LoadFontFromMemory method (like the WeChat public account example)
bool AssetManager::LoadExternalFont(const char* fontPath, int fontSize) {
    // CRITICAL FIX: On Android, LoadFontEx() cannot access assets directory directly
    // We MUST use LoadFileData() + LoadFontFromMemory() method (like the working example)

    // Collect all Chinese text used in the game into one string
    const char* allChineseText =
        "方块吞噬者无尽模式关卡模式时间挑战设置返回开始继续退出重新开始"
        "生命能量等级分数时间"
        "主题蓝色深色绿色紫色红色"
        "语言中文英文"
        "控制虚拟摇杆触摸跟随"
        "字体思源黑体Zpix默认"
        "普通困难专家"
        "第关"
        "暂停游戏结束胜利失败"
        "技能方向不能同时响应"
        "字体加载成功加载失败"
        "使用默认字体中文可能无法正确显示"
        "当前语言"
        "已经切换到"
        "主题"
        "控制方式"
        "日志查看器"
        "所有日志已同步"
        "HP"
        "当前等级"
        "当前分数"
        "剩余时间"
        "关卡选择"
        "请选择关卡"
        "无尽模式"
        "关卡模式"
        "时间挑战"
        "设置"
        "主菜单"
        "暂停"
        "游戏结束"
        "你赢了"
        "你死了"
        "再玩一次"
        "继续游戏"
        "返回主菜单"
        "技能4没有出现弧形防护罩"
        "选关卡的模式一进去就直接死了"
        "ASCII测试中文字符测试";

    // Add ASCII range manually
    char allText[1024];
    snprintf(allText, sizeof(allText),
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"
        " !\"#$%%&'()*+,-./:;<=>?@[\\]^_`{|}~%s",
        allChineseText);

    TraceLog(LOG_INFO, "=== LoadExternalFont START (LoadFileData + LoadFontFromMemory method) ===");

    // Use LoadCodepoints to automatically extract characters from text
    int codepointCount = 0;
    int* codepoints = LoadCodepoints(allText, &codepointCount);

    TraceLog(LOG_INFO, TextFormat("LoadCodepoints extracted %d characters from text", codepointCount));

    // List of font paths to try
    const char* fontPaths[] = {
        "fonts/zpix.ttf",
        "fonts/SourceHanSansCN-Regular.otf",
        "fonts/vonwaon_pixel_12px.ttf",
        "zpix.ttf",
        "SourceHanSansCN-Regular.otf"
    };
    const int numPaths = sizeof(fontPaths) / sizeof(fontPaths[0]);

    // Try each font path
    for (int i = 0; i < numPaths; i++) {
        const char* path = fontPaths[i];

        TraceLog(LOG_INFO, TextFormat("Trying font: %s", path));

        // Load font file into memory (works on Android!)
        int fileSize = 0;
        unsigned char* fileData = LoadFileData(path, &fileSize);

        if (fileData == nullptr || fileSize == 0) {
            TraceLog(LOG_WARNING, TextFormat("LoadFileData failed for: %s", path));
            continue;
        }

        TraceLog(LOG_INFO, TextFormat("LoadFileData SUCCESS: %d bytes loaded from %s", fileSize, path));

        // Get file extension for font type detection
        const char* ext = (strstr(path, ".ttf") != nullptr) ? ".ttf" : ".otf";

        // Load font from memory using LoadFontFromMemory (like the WeChat example!)
        pixelFont = LoadFontFromMemory(ext, fileData, fileSize, fontSize, codepoints, codepointCount);

        // Free file data after loading
        UnloadFileData(fileData);

        if (pixelFont.texture.id != 0 && pixelFont.glyphCount > 100) {
            TraceLog(LOG_INFO, TextFormat("SUCCESS: Font loaded from memory! glyphs=%d (expected %d)",
                pixelFont.glyphCount, codepointCount));
            SetTextureFilter(pixelFont.texture, TEXTURE_FILTER_BILINEAR);
            GenTextureMipmaps(&pixelFont.texture);

            // Load small font
            fileData = LoadFileData(path, &fileSize);
            if (fileData != nullptr) {
                smallFont = LoadFontFromMemory(ext, fileData, fileSize, (int)(fontSize * 0.75f), codepoints, codepointCount);
                UnloadFileData(fileData);

                if (smallFont.texture.id != 0) {
                    SetTextureFilter(smallFont.texture, TEXTURE_FILTER_BILINEAR);
                    GenTextureMipmaps(&smallFont.texture);
                } else {
                    smallFont = pixelFont;
                }
            } else {
                smallFont = pixelFont;
            }

            UnloadCodepoints(codepoints);
            return true;
        } else {
            TraceLog(LOG_WARNING, TextFormat("Font loaded but glyphCount too low: %d (need > 100)", pixelFont.glyphCount));
        }
    }

    UnloadCodepoints(codepoints);
    TraceLog(LOG_ERROR, "All font loading attempts failed!");
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
