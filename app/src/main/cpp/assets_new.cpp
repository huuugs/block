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
