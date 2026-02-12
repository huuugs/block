#include "game.h"
#include "raylib.h"

using namespace BlockEater;

// Raylib Android will call this from its android_main() wrapper
int main(int argc, char* argv[]) {
    // CRITICAL: Enable multi-touch support on Android
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    
    // Initialize window
    TraceLog(LOG_INFO, "SCREEN: initializing window");
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Block Eater");
    SetTargetFPS(TARGET_FPS);
    
    // Log actual screen dimensions
    TraceLog(LOG_INFO, TextFormat("SCREEN: actual=%dx%d virtual=%dx%d", 
            GetScreenWidth(), GetScreenHeight(), SCREEN_WIDTH, SCREEN_HEIGHT));

    // Initialize audio
    InitAudioDevice();

    // Create and run game
    Game* game = new Game();
    game->init();
    game->run();
    game->shutdown();
    delete game;

    // Cleanup
    CloseAudioDevice();
    CloseWindow();

    return 0;
}
