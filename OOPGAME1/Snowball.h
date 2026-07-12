#pragma once
// =========================================================
// Snowball.h — Pink energy-ball projectile (Blossom)
// No STL libraries
// =========================================================

#include <cmath>
#include "Projectile.h"

enum class SnowballState
{
    Flying,
    Encased,   // frozen enemy ball (does NOT move)
    Rolling,   // kicked ball (moves and kills enemies)
    Gone
};

class Snowball : public Projectile
{
public:
    Snowball(float x, float y, sf::Vector2f direction,
        bool powered = false, bool maxRange = false);
    virtual ~Snowball() = default;

    sf::Vector2f getDirection() const { return m_direction; }

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    // FIX: only declare onHit() override if Projectile base declares it virtual.
    // If your Projectile.h does NOT have: virtual void onHit() { }
    // then remove the override keyword — just declare it as a plain method.
    void onHit();   // <-- no 'override' unless Projectile declares virtual onHit()

    void resolvePlatformCollision(const sf::FloatRect* platforms, int count);
    void startRolling(sf::Vector2f direction);

    bool isRolling()     const { return m_state == SnowballState::Rolling; }
    int  getChainCount() const { return m_chainCount; }
    void addChainKill();
    void setEncased();

    SnowballState getState() const { return m_state; }

private:
    SnowballState m_state = SnowballState::Flying;
    bool          m_powered = false;
    bool          m_maxRange = false;
    float         m_distanceTravelled = 0.f;
    float         m_maxDistance = 0.f;
    int           m_chainCount = 0;
    float         m_rollSize = 24.f;
    float         m_rollVelocityY = 0.f;
    float         m_glowTimer = 0.f;
};