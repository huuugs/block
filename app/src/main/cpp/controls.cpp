#include "controls.h"
#include <cmath>

namespace BlockEater {

ControlSystem::ControlSystem()
    : mode(ControlMode::VIRTUAL_JOYSTICK)
    , touchPosition{0, 0}
    , paused(false)
    , modeButtonPressed(false)
{
    // Joystick - INCREASED radius from 80 to 120 for better sensitivity
    // IMPORTANT: Use proper initialization with radius 120
    VirtualJoystick joystick = {{0, 0}, 120, {0, 0}, false, false, -1};

    controls() = ControlSystem() {
    // Joystick will be initialized with radius=120 in the constructor initializer list
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

    // If joystick is active, look for the same touch point by actual touch ID
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
                // INCREASED RESPONSIVENESS: Doubled radius from 80 to 120 for better sensitivity
                joystick.input = {delta.x / joystick.radius, delta.y / joystick.radius};

                // DEBUG: Log input details for diagnosis
                TraceLog(LOG_DEBUG, "JOYSTICK INPUT: input=(%.3f,%.3f) delta=(%.2f,%.2f) dist=%.2f radius=%.0f",
                         joystick.input.x, joystick.input.y, delta.x, delta.y, dist, joystick.radius);
                foundTouch = true;

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

                TraceLog(LOG_INFO, "Joystick ACTIVATED: radius=120 origin=(%.0f,%.0f) touchID=%d",
                         joystick.radius, joystick.origin.x, joystick.origin.y, joystick.touchPointId);
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

    // IMPORTANT: Draw with INCREASED radius (120 instead of 80) for better visibility
    // Draw base - larger semi-transparent circle
    DrawCircleV(joystick.origin, 120, {50, 50, 80, 150});

    // Draw stick - also with increased radius
    Vector2 stickPos = {
        joystick.origin.x + joystick.input.x * 120,
        joystick.origin.y + joystick.input.y * 120
    };
    DrawCircleV(stickPos, 120 * 0.5f, {150, 150, 200, 200});

    // Draw outline circles for better visibility
    DrawCircleLines(joystick.origin, 120, {100, 100, 150, 200});
}

void ControlSystem::drawPauseButton() {
    int x = SCREEN_WIDTH - 100;
    int y = 30;
    int size = 50;

    DrawRectangle(x, y, size, size, {60, 60, 100, 200});
    DrawRectangleLines(x, y, size, size, {255, 100, 100, 255});

    // Draw pause symbol
    DrawRectangle(x + 15, y + 12, 8, 26, WHITE);
    DrawRectangle(x + 27, y + 12, 8, 26, WHITE);
}

Vector2 ControlSystem::getJoystickInput() {
    return joystick.input;
}

} // namespace BlockEater
