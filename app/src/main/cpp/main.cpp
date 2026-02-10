#include "game.h"

using namespace BlockEater;

// Raylib Android will call this from its android_main() wrapper
int main(int argc, char* argv[]) {
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

    return 0;
}
