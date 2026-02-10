#include "game.h"
#include <android/native_activity.h>

using namespace BlockEater;

// Android entry point - called by NativeActivity framework
void android_main(android_app* app) {
    // Initialize window
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Block Eater");
    SetTargetFPS(TARGET_FPS);

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
}
