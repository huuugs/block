#include "particles.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

namespace BlockEater {

// Particle base class
Particle::Particle(ParticleType type, Vector2 pos, Color col, float lifeTime)
    : position(pos)
    , velocity{0, 0}
    , color(col)
    , lifeTime(lifeTime)
    , maxLifeTime(lifeTime)
    , type(type)
    , size(5.0f)
    , alpha(1.0f)
{
}

Particle::~Particle() {
}

void Particle::update(float dt) {
    position.x += velocity.x * dt;
    position.y += velocity.y * dt;
    lifeTime -= dt;
    alpha = lifeTime / maxLifeTime;
}

void Particle::draw() {
    Color c = color;
    c.a = (unsigned char)(alpha * 255);

    if (type == ParticleType::PIXEL) {
        DrawRectangle((int)position.x, (int)position.y, (int)size, (int)size, c);
    } else if (type == ParticleType::CIRCLE) {
        DrawCircleV(position, size, c);
    }
}

// Text Particle
TextParticle::TextParticle(Vector2 pos, const char* text, Color col)
    : Particle(ParticleType::TEXT, pos, col, 1.5f)
    , text(text)
    , fontSize(20)
{
    velocity.y = -50.0f;  // Float upward
}

void TextParticle::draw() {
    Color c = color;
    c.a = (unsigned char)(alpha * 255);
    DrawText(text.c_str(), (int)position.x, (int)position.y, fontSize, c);
}

// Pixel Particle
PixelParticle::PixelParticle(Vector2 pos, Color col, Vector2 vel)
    : Particle(ParticleType::PIXEL, pos, col, 0.8f + (float)(rand() % 100) / 500.0f)
{
    velocity = vel;
    size = 3.0f + (float)(rand() % 100) / 50.0f;
}

// Level Up Particle
LevelUpParticle::LevelUpParticle(Vector2 pos, int level)
    : Particle(ParticleType::LEVEL_UP, pos, {255, 215, 0, 255}, 2.0f)
    , level(level)
    , scale(1.0f)
    , rotation(0.0f)
{
    size = 50.0f;
}

void LevelUpParticle::update(float dt) {
    Particle::update(dt);
    scale += dt * 2.0f;
    rotation += dt * 180.0f;
}

void LevelUpParticle::draw() {
    Color c = color;
    c.a = (unsigned char)(alpha * 255);

    char text[32];
    sprintf(text, "LEVEL %d!", level);

    int fontSize = (int)(30 * scale);
    int textWidth = MeasureText(text, fontSize);

    // Draw with rotation effect (simulated by oscillating position)
    float wave = sinf(rotation * DEG2RAD) * 5.0f;
    DrawText(text, (int)(position.x - textWidth/2 + wave), (int)position.y, fontSize, c);

    // Draw star burst
    for (int i = 0; i < 8; i++) {
        float angle = (i * 45 + rotation) * DEG2RAD;
        float dist = 30 * scale;
        Vector2 starPos = {
            position.x + cosf(angle) * dist,
            position.y + sinf(angle) * dist
        };
        DrawCircleV(starPos, 3 * scale, c);
    }
}

// Particle System
ParticleSystem::ParticleSystem() {
}

ParticleSystem::~ParticleSystem() {
    for (auto* p : particles) {
        delete p;
    }
}

void ParticleSystem::update(float dt) {
    for (auto* p : particles) {
        p->update(dt);
    }
    cleanup();
}

void ParticleSystem::draw() {
    for (auto* p : particles) {
        p->draw();
    }
}

void ParticleSystem::spawnPixelExplosion(Vector2 pos, Color color, int count) {
    for (int i = 0; i < count; i++) {
        float angle = ((float)(rand() % 360)) * DEG2RAD;
        float speed = 100.0f + (float)(rand() % 200);
        Vector2 vel = {
            cosf(angle) * speed,
            sinf(angle) * speed
        };
        particles.push_back(new PixelParticle(pos, color, vel));
    }
}

void ParticleSystem::spawnTextPopup(Vector2 pos, const char* text, Color color) {
    particles.push_back(new TextParticle(pos, text, color));
}

void ParticleSystem::spawnLevelUp(Vector2 pos, int level) {
    particles.push_back(new LevelUpParticle(pos, level));

    // Add pixel explosion
    spawnPixelExplosion(pos, {255, 215, 0, 255}, 30);
}

void ParticleSystem::spawnDamageNumber(Vector2 pos, int damage, bool isCrit) {
    char text[16];
    if (isCrit) {
        sprintf(text, "%d!", damage);
    } else {
        sprintf(text, "-%d", damage);
    }
    Color color = isCrit ? Color{255, 50, 50, 255} : Color{255, 150, 150, 255};
    particles.push_back(new TextParticle(pos, text, color));
}

void ParticleSystem::spawnExplosion(Vector2 pos, Color color, float size) {
    int count = (int)(size * 2);
    spawnPixelExplosion(pos, color, count);
}

void ParticleSystem::cleanup() {
    auto it = particles.begin();
    while (it != particles.end()) {
        if (!(*it)->isAlive()) {
            delete *it;
            it = particles.erase(it);
        } else {
            ++it;
        }
    }
}

} // namespace BlockEater
