#ifndef CONTROLS_H
#define CONTROLS_H

#include "raylib.h"
#include "game.h"

namespace BlockEater {

// Virtual Joystick
struct VirtualJoystick {
    Vector2 origin;
    float radius;
    Vector2 input;
    bool active;
    bool originSet;  // Track if origin has been set for dynamic positioning
    int touchPointId;  // Actual touch point ID from GetTouchPointId()

    VirtualJoystick() : origin{0, 0}, radius(120), input{0, 0}, active(false), originSet(false), touchPointId(-1) {}
};

class ControlSystem {
public:
    ControlSystem();
    ~ControlSystem();

    void init();
    void update();
    void draw();

    Vector2 getInputVector() const;
    Vector2 getInputVector(Vector2 playerPos) const;  // For touch follow mode
    ControlMode getMode() const { return mode; }
    void setMode(ControlMode m) { mode = m; }
    void toggleMode();

    bool isPaused() const { return paused; }
    void togglePause();

private:
    ControlMode mode;
    VirtualJoystick joystick;
    Vector2 touchPosition;
    bool paused;
    bool modeButtonPressed;

    void updateJoystick();
    void updateTouchFollow();
    void drawJoystick();
    void drawModeButton();
    void drawPauseButton();

    Vector2 getJoystickInput();
};

} // namespace BlockEater

#endif // CONTROLS_H
