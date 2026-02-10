#include "game.h"

using namespace BlockEater;

// Android entry point
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
