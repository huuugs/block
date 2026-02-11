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
    
    // Try to load zpix pixel font (Chinese support) - preferred
    if (!loaded) {
        loaded = LoadExternalFont("fonts/zpix.ttf", 16);
    }
    
    // Fallback: Vonwaon pixel font
    if (!loaded) {
        loaded = LoadExternalFont("fonts/vonwaon_pixel_12px.ttf", 12);
    }
    
    // Fallback: Source Han Sans
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
    // Define codepoints array for Chinese character support
    // Only load characters actually used in the game to save memory
    static int* chineseCodepoints = nullptr;
    static int codepointCount = 0;

    // Initialize codepoints array on first call
    if (chineseCodepoints == nullptr) {
        // Build a list of only the Chinese characters used in the game
        // Common CJK characters used in UI
        const int gameChineseChars[] = {
            // Menu items
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
        };
        int chineseCharCount = sizeof(gameChineseChars) / sizeof(gameChineseChars[0]);

        // ASCII (32-126) + game Chinese characters
        codepointCount = 95 + chineseCharCount;
        chineseCodepoints = (int*)malloc(codepointCount * sizeof(int));

        int idx = 0;
        // Add ASCII characters (32-126)
        for (int i = 32; i <= 126; i++) {
            chineseCodepoints[idx++] = i;
        }
        // Add game-specific Chinese characters
        for (int i = 0; i < chineseCharCount; i++) {
            chineseCodepoints[idx++] = gameChineseChars[i];
        }

        TraceLog(LOG_INFO, TextFormat("Initialized font codepoints: %d ASCII + %d Chinese = %d total",
            95, chineseCharCount, codepointCount));
    }

    // Try loading from the given path first
    if (FileExists(fontPath)) {
        // Load font with Chinese character support
        pixelFont = LoadFontEx(fontPath, fontSize, chineseCodepoints, codepointCount);
        if (pixelFont.texture.id != 0) {
            GenTextureMipmaps(&pixelFont.texture);

            smallFont = LoadFontEx(fontPath, (int)(fontSize * 0.75f), chineseCodepoints, codepointCount);
            if (smallFont.texture.id != 0) {
                GenTextureMipmaps(&smallFont.texture);
            } else {
                smallFont = pixelFont;
            }

            TraceLog(LOG_INFO, TextFormat("Font loaded from %s with %d codepoints (including Chinese)", fontPath, codepointCount));
            return true;
        }
    }

    // On Android, try assets path
    #if defined(PLATFORM_ANDROID)
    const char* androidPaths[] = {
        "fonts/zpix.ttf",
        "zpix.ttf",
        "fonts/vonwaon_pixel_12px.ttf",
        "fonts/SourceHanSansCN-Regular.otf",
        "vonwaon_pixel_12px.ttf",
        "SourceHanSansCN-Regular.otf"
    };

    for (const char* path : androidPaths) {
        if (FileExists(path)) {
            // Load font with Chinese character support
            pixelFont = LoadFontEx(path, fontSize, chineseCodepoints, codepointCount);
            if (pixelFont.texture.id != 0) {
                GenTextureMipmaps(&pixelFont.texture);

                smallFont = LoadFontEx(path, (int)(fontSize * 0.75f), chineseCodepoints, codepointCount);
                if (smallFont.texture.id != 0) {
                    GenTextureMipmaps(&smallFont.texture);
                } else {
                    smallFont = pixelFont;
                }

                TraceLog(LOG_INFO, TextFormat("Font loaded from Android assets: %s with %d codepoints (including Chinese)", path, codepointCount));
                return true;
            }
        }
    }
    #endif

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
