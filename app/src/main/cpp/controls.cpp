#include "controls.h"
#include <cmath>

namespace BlockEater {

ControlSystem::ControlSystem()
    : mode(ControlMode::VIRTUAL_JOYSTICK)
    , touchPosition{0, 0}
    , paused(false)
    , modeButtonPressed(false)
{
    // Joystick origin will be set dynamically on touch
}

ControlSystem::~ControlSystem() {
}

void ControlSystem::init() {
}

void ControlSystem::update() {
    if (mode == ControlMode::VIRTUAL_JOYSTICK) {
        updateJoystick();
    } else {
        updateTouchFollow();
    }
}

void ControlSystem::draw() {
    if (mode == ControlMode::VIRTUAL_JOYSTICK) {
        drawJoystick();
    }

    drawPauseButton();
}

Vector2 ControlSystem::getInputVector() const {
    if (mode == ControlMode::VIRTUAL_JOYSTICK) {
        return joystick.input;
    } else {
        // Touch follow mode - requires player position to work properly
        // This version is a fallback that will be replaced by the overload
        return {0, 0};
    }
}

Vector2 ControlSystem::getInputVector(Vector2 playerPos) const {
    // Check keyboard input first (WASD or Arrow keys)
    Vector2 keyboardInput = {0, 0};
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) keyboardInput.y -= 1.0f;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) keyboardInput.y += 1.0f;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) keyboardInput.x -= 1.0f;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) keyboardInput.x += 1.0f;

    // Normalize keyboard input if diagonal
    if (Vector2Length(keyboardInput) > 0.1f) {
        return Vector2Normalize(keyboardInput);
    }

    // Fall back to touch input
    if (mode == ControlMode::VIRTUAL_JOYSTICK) {
        // Check if joystick has any meaningful input
        if (Vector2Length(joystick.input) > 0.01f) {
            return joystick.input;
        }
    } else {
        // Touch follow mode - calculate direction from touch to player
        if (GetTouchPointCount() > 0) {
            Vector2 touchPos = GetTouchPosition(0);
            // Calculate direction from player toward touch point
            return Vector2Normalize(touchPos - playerPos);
        }
    }
    return {0, 0};
}

void ControlSystem::toggleMode() {
    mode = (mode == ControlMode::VIRTUAL_JOYSTICK)
        ? ControlMode::TOUCH_FOLLOW
        : ControlMode::VIRTUAL_JOYSTICK;
}

void ControlSystem::togglePause() {
    paused = !paused;
}

void ControlSystem::updateJoystick() {
    int touchCount = GetTouchPointCount();

    // If no touches, reset joystick
    if (touchCount == 0) {
        joystick.active = false;
        joystick.input = {0, 0};
        joystick.touchId = -1;
        joystick.originSet = false;
        return;
    }

    // If joystick is active, try to find the same touch point by checking position
    if (joystick.active && joystick.originSet) {
        bool foundTouch = false;
        for (int i = 0; i < touchCount; i++) {
            Vector2 touchPos = GetTouchPosition(i);
            // Check if this touch is near our origin (same touch point)
            float distToOrigin = Vector2Length(touchPos - joystick.origin);
            if (distToOrigin < joystick.radius * 1.5f) {
                // This is our joystick touch
                joystick.touchId = i;
                foundTouch = true;

                // Calculate input
                Vector2 delta = touchPos - joystick.origin;
                float dist = Vector2Length(delta);
                if (dist > joystick.radius) {
                    delta = Vector2Normalize(delta) * joystick.radius;
                }
                joystick.input = {delta.x / joystick.radius, delta.y / joystick.radius};
                break;
            }
        }

        if (!foundTouch) {
            // Touch point lost, reset joystick
            joystick.active = false;
            joystick.input = {0, 0};
            joystick.touchId = -1;
            joystick.originSet = false;
        }
        return;
    }

    // Check for new touch in left half of screen (dynamic positioning)
    if (joystick.touchId == -1) {
        for (int i = 0; i < touchCount; i++) {
            Vector2 touchPos = GetTouchPosition(i);

            // Check if touch is in left half of screen
            if (touchPos.x < SCREEN_WIDTH / 2) {
                joystick.active = true;
                joystick.touchId = i;
                joystick.origin = touchPos;  // Dynamic origin at touch position
                joystick.originSet = true;
                joystick.input = {0, 0};  // Start with no input
                break;  // Only handle one touch for joystick
            }
        }
    }

    // Update joystick input if active (using stored touchId)
    if (joystick.active && joystick.touchId >= 0 && joystick.touchId < touchCount) {
        Vector2 touchPos = GetTouchPosition(joystick.touchId);
        Vector2 delta = touchPos - joystick.origin;

        float dist = Vector2Length(delta);
        if (dist > joystick.radius) {
            delta = Vector2Normalize(delta) * joystick.radius;
        }

        joystick.input = {delta.x / joystick.radius, delta.y / joystick.radius};
    }
}

void ControlSystem::updateTouchFollow() {
    // Touch follow is handled in getInputVector()
}

void ControlSystem::drawJoystick() {
    // Only draw when joystick is active (finger is touching)
    if (!joystick.active) {
        return;  // Don't draw anything when not active
    }

    // Draw base
    DrawCircleV(joystick.origin, joystick.radius, {50, 50, 80, 150});
    DrawCircleLines(joystick.origin.x, joystick.origin.y, joystick.radius, {100, 100, 150, 200});

    // Draw stick
    Vector2 stickPos = {
        joystick.origin.x + joystick.input.x * joystick.radius,
        joystick.origin.y + joystick.input.y * joystick.radius
    };
    DrawCircleV(stickPos, joystick.radius * 0.5f, {150, 150, 200, 200});
}

void ControlSystem::drawModeButton() {
    int x = SCREEN_WIDTH - 100;
    int y = SCREEN_HEIGHT - 80;
    int width = 80;
    int height = 40;

    const char* text = (mode == ControlMode::VIRTUAL_JOYSTICK) ? "Joystick" : "Touch";

    DrawRectangle(x, y, width, height, {60, 60, 100, 200});
    DrawRectangleLines(x, y, width, height, {150, 150, 200, 255});

    int fontSize = 12;
    int textWidth = MeasureText(text, fontSize);
    DrawText(text, x + (width - textWidth) / 2, y + 12, fontSize, WHITE);
}

void ControlSystem::drawPauseButton() {
    int x = SCREEN_WIDTH - 100;
    int y = 30;
    int size = 50;

    DrawRectangle(x, y, size, size, {200, 50, 50, 200});
    DrawRectangleLines(x, y, size, size, {255, 100, 100, 255});

    // Draw pause symbol
    DrawRectangle(x + 15, y + 12, 8, 26, WHITE);
    DrawRectangle(x + 27, y + 12, 8, 26, WHITE);
}

Vector2 ControlSystem::getJoystickInput() {
    return joystick.input;
}

} // namespace BlockEater
