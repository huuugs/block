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
    // First try to load external font with Chinese support
    bool loaded = false;

    // IMPORTANT: Try Source Han Sans FIRST for full Chinese character support
    // zpix.ttf is a pixel font with limited Chinese character support
    // Source Han Sans has comprehensive CJK character coverage
    if (!loaded) {
        loaded = LoadExternalFont("fonts/SourceHanSansCN-Regular.otf", 18);
    }

    // Fallback: zpix pixel font (limited Chinese support)
    if (!loaded) {
        loaded = LoadExternalFont("fonts/zpix.ttf", 16);
    }

    // Fallback: Vonwaon pixel font (likely no Chinese)
    if (!loaded) {
        loaded = LoadExternalFont("fonts/vonwaon_pixel_12px.ttf", 12);
    }

    // Fallback: use default raylib font if external fonts not available
    if (!loaded) {
        pixelFont = GetFontDefault();
        smallFont = GetFontDefault();
        TraceLog(LOG_WARNING, "Using default font (no Chinese support)");
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

bool AssetManager::LoadExternalFont(const char* fontPath, int fontSize) {
    // Use character RANGES instead of individual characters for better Chinese support
    // According to raylib docs, LoadFontEx can take ranges like {start, end, 0}
    static int* chineseCodepoints = nullptr;
    static int codepointCount = 0;

    if (chineseCodepoints == nullptr) {
        // Define character ranges for:
        // 1. ASCII (32-126)
        // 2. CJK Unified Ideographs (0x4E00-0x9FFF) - common Chinese characters
        // 3. CJK punctuation and symbols (0x3000-0x303F)
        // Format: each range is {start, end, 0}

        // We'll use individual character listing for exact control
        // But add the full CJK range as fallback
        const int gameChineseChars[] = {
            // ASCII range will be added separately

            // Menu items - game specific Chinese characters
            0x65B9, 0x5757, 0x541E, 0x5410, 0x8005,  // 方块吞噬者
            0x65E0, 0x5C3D, 0x6A21, 0x5F0F,           // 无尽模式
            0x5173, 0x5361, 0x6A21, 0x5F0F,           // 关卡模式
            0x65F6, 0x95F4, 0x6311, 0x6218,           // 时间挑战
            0x8BBE, 0x7F6E,                           // 设置
            0x9000, 0x51FA,                           // 退出
            0x6682, 0x505C,                           // 暂停
            0x7EE7, 0x7EED,                           // 继续
            0x8FD4, 0x56DE,                           // 返回
            0x4E3B, 0x83DC, 0x5355,                   // 主菜单
            0x6E38, 0x620F, 0x7ED3, 0x675F,           // 游戏结束
            0x518D, 0x8BD5, 0x4E00, 0x6B21,           // 再试一次
            0x6700, 0x7EC8, 0x5F97, 0x5206,           // 最终得分
            0x8FBE, 0x5230, 0x7B49, 0x7EA7,           // 达到等级
            0x9009, 0x62E9, 0x5173, 0x5361,           // 选择关卡
            0x8BED, 0x8A00,                           // 语言
            0x4E2D, 0x6587,                           // 中文
            0x4E3B, 0x9898,                           // 主题
            0x63A7, 0x5236,                           // 控制
            0x6447, 0x6746,                           // 摇杆
            0x97F3, 0x91CF,                           // 音量
            0x89E6, 0x6478, 0x5DE6, 0x534A, 0x5C4F, 0x79FB, 0x52A8,  // 触摸左半屏移动
            0x83F1, 0x5F62,                           // 菱形
            0x80CC, 0x666F,                           // 背景
            0x751F, 0x547D,                           // 生命 (HP label)
            0x80FD, 0x91CF,                           // 能量 (Energy label)
            0x7CFB, 0x7EDF,                           // 系统 - for log viewer title
            0x65E5, 0x5FD7,                           // 日志 - for log viewer title
            0x6761,                                   // 条 - for "条日志"
            0x5230,                                   // 到 - "退出到菜单", "触摸跟随"
            0xFF1A,                                   // ： - Chinese colon (for labels like "语言:")
            0x8DDF, 0x968F,                           // 跟随 - "触摸跟随"
            0x6D88,                                   // 消 - "取消静音"
            0x9759,                                   // 静 - "静音", "取消静音" (音 already exists)
            0x67E5,                                   // 查 - "查看日志"
            0x770B,                                   // 看 - "查看日志"
            0x7ECF, 0x9A8C,                           // 经验 - XP label
        };

        int chineseCharCount = sizeof(gameChineseChars) / sizeof(gameChineseChars[0]);

        // ASCII (32-126) + game Chinese characters
        codepointCount = 95 + chineseCharCount;
        chineseCodepoints = (int*)malloc(codepointCount * sizeof(int));

        int idx = 0;
        // Add ASCII characters (32-126) first
        for (int i = 32; i <= 126; i++) {
            chineseCodepoints[idx++] = i;
        }
        // Add Chinese characters
        for (int i = 0; i < chineseCharCount; i++) {
            chineseCodepoints[idx++] = gameChineseChars[i];
        }

        TraceLog(LOG_INFO, TextFormat("Font codepoints initialized: %d total (95 ASCII + %d Chinese)",
            codepointCount, chineseCharCount));
    }

    // Try loading from the given path first
    if (FileExists(fontPath)) {
        TraceLog(LOG_INFO, TextFormat("Loading font from: %s", fontPath));

        pixelFont = LoadFontEx(fontPath, fontSize, chineseCodepoints, codepointCount);

        if (pixelFont.texture.id != 0) {
            TraceLog(LOG_INFO, TextFormat("SUCCESS: Font loaded from %s", fontPath));
            TraceLog(LOG_INFO, TextFormat("Font: ID=%u, size=%dx%d, codepoints=%d",
                pixelFont.texture.id, pixelFont.texture.width, pixelFont.texture.height, codepointCount));
            TraceLog(LOG_INFO, TextFormat("Font baseSize: %d", pixelFont.baseSize));

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
        } else {
            TraceLog(LOG_ERROR, TextFormat("FAILED: Font texture ID is 0 for: %s", fontPath));
        }
    }

    // On Android, try assets path - Source Han Sans FIRST for Chinese support
    #if defined(PLATFORM_ANDROID)
    const char* androidPaths[] = {
        "fonts/SourceHanSansCN-Regular.otf",  // Try Source Han Sans first - full CJK support
        "SourceHanSansCN-Regular.otf",
        "fonts/zpix.ttf",                      // Pixel font - limited Chinese
        "zpix.ttf",
        "fonts/vonwaon_pixel_12px.ttf",        // Pixel font - likely no Chinese
        "vonwaon_pixel_12px.ttf"
    };

    for (const char* path : androidPaths) {
        if (FileExists(path)) {
            TraceLog(LOG_INFO, TextFormat("Loading Android font: %s", path));

            pixelFont = LoadFontEx(path, fontSize, chineseCodepoints, codepointCount);
            if (pixelFont.texture.id != 0) {
                TraceLog(LOG_INFO, TextFormat("SUCCESS: Android font loaded from %s", path));
                TraceLog(LOG_INFO, TextFormat("Font: ID=%u, size=%dx%d, codepoints=%d",
                    pixelFont.texture.id, pixelFont.texture.width, pixelFont.texture.height, codepointCount));

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
            } else {
                TraceLog(LOG_WARNING, TextFormat("FAILED: Font load returned texture ID 0 for: %s", path));
            }
        } else {
            TraceLog(LOG_WARNING, TextFormat("File not found: %s", path));
        }
    }
    #endif

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

} // namespace BlockEater
