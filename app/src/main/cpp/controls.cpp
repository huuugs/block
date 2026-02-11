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

    drawModeButton();
    drawPauseButton();
}

Vector2 ControlSystem::getInputVector() const {
    if (mode == ControlMode::VIRTUAL_JOYSTICK) {
        return joystick.input;
    } else {
        if (GetTouchPointCount() > 0) {
            Vector2 touchPos = GetTouchPosition(0);
            Vector2 playerPos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};  // Will be updated by game
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
                break;  // Only handle one touch for joystick
            }
        }
    }

    // Update joystick input
    if (joystick.active && joystick.touchId >= 0 && joystick.touchId < touchCount) {
        Vector2 touchPos = GetTouchPosition(joystick.touchId);
        Vector2 delta = touchPos - joystick.origin;

        float dist = Vector2Length(delta);
        if (dist > joystick.radius) {
            delta = Vector2Normalize(delta) * joystick.radius;
        }

        joystick.input = {delta.x / joystick.radius, delta.y / joystick.radius};

        // Release if finger is lifted
        bool touchFound = false;
        for (int i = 0; i < touchCount; i++) {
            if (i == joystick.touchId) {
                touchFound = true;
                break;
            }
        }

        if (!touchFound) {
            joystick.active = false;
            joystick.input = {0, 0};
            joystick.touchId = -1;
            joystick.originSet = false;
        }
    }
}

void ControlSystem::updateTouchFollow() {
    // Touch follow is handled in getInputVector()
}

void ControlSystem::drawJoystick() {
    if (!joystick.active && Vector2Length(joystick.input) < 0.01f) {
        return;
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

Vector2 ControlSystem::getTouchFollowInput() {
    if (GetTouchPointCount() > 0) {
        Vector2 touchPos = GetTouchPosition(0);
        Vector2 playerPos = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
        return Vector2Normalize(touchPos - playerPos);
    }
    return {0, 0};
}

} // namespace BlockEater
