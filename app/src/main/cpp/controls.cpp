#include "controls.h"
#include <cmath>

namespace BlockEater {

ControlSystem::ControlSystem()
    : mode(ControlMode::VIRTUAL_JOYSTICK)
    , touchPosition{0, 0}
    , paused(false)
    , modeButtonPressed(false)
{
    // Joystick radius is set to 120 in VirtualJoystick constructor (controls.h)
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

    // Fall back to touch input - FIX: Always return joystick input, remove threshold check
    // This ensures even small finger movements are registered
    if (mode == ControlMode::VIRTUAL_JOYSTICK) {
        return joystick.input;
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
        joystick.touchPointId = -1;
        joystick.originSet = false;
        return;
    }

    // If joystick is active, look for same touch point by actual touch ID
    if (joystick.active && joystick.touchPointId >= 0) {
        bool foundTouch = false;
        for (int i = 0; i < touchCount; i++) {
            int currentTouchId = GetTouchPointId(i);
            if (currentTouchId == joystick.touchPointId) {
                // Found our tracked touch point, update input
                Vector2 touchPos = GetTouchPosition(i);
                Vector2 delta = touchPos - joystick.origin;

                float dist = Vector2Length(delta);
                if (dist > joystick.radius) {
                    delta = Vector2Normalize(delta) * joystick.radius;
                }

                // IMPORTANT: Use normalized value from -1 to 1 based on displacement from origin
                // This ensures the joystick responds even to small finger movements
                joystick.input = {delta.x / joystick.radius, delta.y / joystick.radius};
                foundTouch = true;

                // Debug: Always log joystick input for diagnosis
                // Use integer values (x1000) to avoid float formatting issues
                TraceLog(LOG_INFO, "Joystick input: %d,%d (delta: %d,%d dist: %d)",
                        (int)(joystick.input.x * 1000), (int)(joystick.input.y * 1000),
                        (int)(delta.x), (int)(delta.y), (int)(dist));
                break;
            }
        }

        if (!foundTouch) {
            // Our tracked touch point was lifted, reset joystick
            joystick.active = false;
            joystick.input = {0, 0};
            joystick.touchPointId = -1;
            joystick.originSet = false;
            TraceLog(LOG_INFO, "Joystick deactivated - touch lifted");
        }
        return;
    }

    // Check for new touch in left half of screen (for activating joystick)
    if (joystick.touchPointId == -1) {
        for (int i = 0; i < touchCount; i++) {
            Vector2 touchPos = GetTouchPosition(i);

            // Check if touch is in left half of screen
            if (touchPos.x < SCREEN_WIDTH / 2) {
                joystick.active = true;
                joystick.touchPointId = GetTouchPointId(i);  // Store actual touch ID
                joystick.origin = touchPos;  // Set origin at touch position
                joystick.originSet = true;
                joystick.input = {0, 0};  // Start with no input
                // Use integers to avoid float formatting issues
                TraceLog(LOG_INFO, "Joystick ACTIVATED: origin=%d,%d touchID=%d screen=%dx%d",
                        (int)(joystick.origin.x), (int)(joystick.origin.y),
                        joystick.touchPointId, SCREEN_WIDTH, SCREEN_HEIGHT);
                break;  // Only handle one touch for joystick
            }
        }
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
    DrawCircleLines((int)joystick.origin.x, (int)joystick.origin.y, joystick.radius, {100, 100, 150, 200});

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
