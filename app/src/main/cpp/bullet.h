#ifndef BULLET_H
#define BULLET_H

#include "raylib.h"
#include "game.h"

namespace BlockEater {

class Bullet {
public:
    Bullet(Vector2 pos, Vector2 dir, int damage, int playerId);
    ~Bullet();

    void update(float dt);
    void draw();

    bool isAlive() const { return alive; }
    Vector2 getPosition() const { return position; }
    int getSize() const { return size; }
    int getDamage() const { return damage; }
    int getPlayerId() const { return playerId; }

    void kill() { alive = false; }

private:
    Vector2 position;
    Vector2 velocity;
    int size;
    int damage;
    int playerId;
    float lifetime;
    bool alive;
    Color color;
};

} // namespace BlockEater

#endif // BULLET_H
