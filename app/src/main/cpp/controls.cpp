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
        // DEBUG: Log what we're returning - use TextFormat for Android compatibility
        TraceLog(LOG_INFO, TextFormat("getInputVector: returning joystick input=%i,%i active=%i",
                (int)(joystick.input.x * 1000), (int)(joystick.input.y * 1000),
                joystick.active ? 1 : 0));
        return joystick.input;
    } else {
        // Touch follow mode - calculate direction from touch to player
        if (GetTouchPointCount() > 0) {
            Vector2 touchPos = GetTouchPosition(0);
            // Calculate direction from player toward touch point
            return Vector2Normalize(touchPos - playerPos);
        }
    }
    TraceLog(LOG_INFO, "getInputVector: returning ZERO");
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
    
    // DEBUG: Always log touch count to verify touch detection is working
    static int lastTouchCount = -1;
    if (touchCount != lastTouchCount) {
        TraceLog(LOG_INFO, TextFormat("TOUCH DEBUG: touchCount=%d screen=%dx%d", 
                touchCount, GetScreenWidth(), GetScreenHeight()));
        lastTouchCount = touchCount;
    }

    // If no touches, reset joystick
    if (touchCount == 0) {
        if (joystick.active) {
            joystick.active = false;
            joystick.input = {0, 0};
            joystick.touchPointId = -1;
            joystick.originSet = false;
            TraceLog(LOG_INFO, "Joystick reset - no touches");
        }
        return;
    }

    // Log all touch positions for debugging
    for (int i = 0; i < touchCount; i++) {
        Vector2 pos = GetTouchPosition(i);
        TraceLog(LOG_INFO, TextFormat("TOUCH %d: pos=%.0f,%.0f (half=%d)", 
                i, pos.x, pos.y, SCREEN_WIDTH/2));
    }

    // CRITICAL FIX: Always find the leftmost touch in left half of screen
    // This is more reliable than tracking by touch ID or index on Android
    int leftTouchIndex = -1;
    float leftmostX = SCREEN_WIDTH;  // Start from right edge
    
    for (int i = 0; i < touchCount; i++) {
        Vector2 pos = GetTouchPosition(i);
        // Find touch in left half that is leftmost
        // CRITICAL: Use GetScreenWidth() for actual screen dimensions on Android
        float halfScreen = GetScreenWidth() / 2.0f;
        if (pos.x < halfScreen && pos.x < leftmostX) {
            leftmostX = pos.x;
            leftTouchIndex = i;
        }
    }
    
    if (leftTouchIndex >= 0) {
        TraceLog(LOG_INFO, TextFormat("Found left touch: index=%d x=%.0f", leftTouchIndex, leftmostX));
    }

    // If joystick is active, update it
    if (joystick.active) {
        if (leftTouchIndex >= 0) {
            // We have a valid touch in left half, update joystick
            Vector2 touchPos = GetTouchPosition(leftTouchIndex);
            
            // If this is a new touch (different from where we started), reset origin
            if (!joystick.originSet) {
                joystick.origin = touchPos;
                joystick.originSet = true;
                TraceLog(LOG_INFO, TextFormat("Joystick origin set: %i,%i", (int)touchPos.x, (int)touchPos.y));
            }
            
            Vector2 delta = touchPos - joystick.origin;
            float dist = Vector2Length(delta);
            
            // Clamp to radius
            if (dist > joystick.radius) {
                delta = Vector2Normalize(delta) * joystick.radius;
                dist = joystick.radius;
            }

            // Calculate normalized input
            joystick.input = {delta.x / joystick.radius, delta.y / joystick.radius};

            // Debug: Log joystick input
            TraceLog(LOG_INFO, TextFormat("Joystick input: %i,%i (delta: %i,%i dist: %i active=1)",
                    (int)(joystick.input.x * 1000), (int)(joystick.input.y * 1000),
                    (int)(delta.x), (int)(delta.y), (int)(dist)));
        } else {
            // No touch in left half, keep joystick active but reset input
            joystick.input = {0, 0};
            TraceLog(LOG_INFO, "Joystick active but no left touch - input reset to 0,0");
        }
        return;
    }

    // Check for new joystick activation (touch in left half)
    if (!joystick.active && leftTouchIndex >= 0) {
        Vector2 touchPos = GetTouchPosition(leftTouchIndex);
        joystick.active = true;
        joystick.touchPointId = leftTouchIndex;
        joystick.origin = touchPos;
        joystick.originSet = true;
        joystick.input = {0, 0};
        TraceLog(LOG_INFO, TextFormat("Joystick ACTIVATED: origin=%i,%i touchIdx=%i",
                (int)(joystick.origin.x), (int)(joystick.origin.y), leftTouchIndex));
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
