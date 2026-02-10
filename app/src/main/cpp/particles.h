#ifndef PARTICLES_H
#define PARTICLES_H

#include "raylib.h"
#include "game.h"
#include <vector>
#include <string>

namespace BlockEater {

// Particle types
enum class ParticleType {
    PIXEL,         // Square pixel particle
    CIRCLE,        // Circle particle
    TEXT,          // Text popup (+10, +100, etc.)
    LEVEL_UP,      // Level up effect
    EXPLOSION      // Explosion effect
};

class Particle {
public:
    Particle(ParticleType type, Vector2 pos, Color col, float lifeTime);
    virtual ~Particle();

    virtual void update(float dt);
    virtual void draw();

    bool isAlive() const { return lifeTime > 0; }

protected:
    Vector2 position;
    Vector2 velocity;
    Color color;
    float lifeTime;
    float maxLifeTime;
    ParticleType type;
    float size;
    float alpha;
};

class TextParticle : public Particle {
public:
    TextParticle(Vector2 pos, const char* text, Color col);
    void draw() override;

private:
    std::string text;
    int fontSize;
};

class PixelParticle : public Particle {
public:
    PixelParticle(Vector2 pos, Color col, Vector2 vel);
};

class LevelUpParticle : public Particle {
public:
    LevelUpParticle(Vector2 pos, int level);
    void update(float dt) override;
    void draw() override;

private:
    int level;
    float scale;
    float rotation;
};

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    void update(float dt);
    void draw();

    void spawnPixelExplosion(Vector2 pos, Color color, int count);
    void spawnTextPopup(Vector2 pos, const char* text, Color color);
    void spawnLevelUp(Vector2 pos, int level);
    void spawnDamageNumber(Vector2 pos, int damage, bool isCrit);
    void spawnExplosion(Vector2 pos, Color color, float size);

private:
    std::vector<Particle*> particles;

    void cleanup();
};

} // namespace BlockEater

#endif // PARTICLES_H
