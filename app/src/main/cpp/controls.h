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
    int touchId;

    VirtualJoystick() : origin{0, 0}, radius(80), input{0, 0}, active(false), touchId(-1) {}
};

class ControlSystem {
public:
    ControlSystem();
    ~ControlSystem();

    void init();
    void update();
    void draw();

    Vector2 getInputVector() const;
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
    Vector2 getTouchFollowInput();
};

} // namespace BlockEater

#endif // CONTROLS_H
