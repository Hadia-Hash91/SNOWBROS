#pragma once
// =========================================================
// PowerUp.h — No STL libraries
// =========================================================

#include "Entity.h"

class Player;

enum class PowerUpType
{
    SpeedBoost,
    SnowballPower,
    DistanceIncrease,
    BalloonMode,
    ExtraLife,
    StarChoice
};

class PowerUp : public Entity
{
public:
    virtual ~PowerUp() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawDebug(sf::RenderWindow& window) override;

    virtual void applyEffect(Player& player) = 0;
    PowerUpType  getType() const { return m_type; }

    // Call this right after spawning so the bob has a stable base
    void setBaseY(float y) { m_baseY = y; m_position.y = y; }

protected:
    PowerUp(float x, float y, PowerUpType type);

    PowerUpType  m_type;
    float        m_lifetime = 8.f;
    float        m_bobTimer = 0.f;
    float        m_baseY = 0.f;   // stable Y origin for sine bob
    sf::Color    m_color;
};

// =========================================================
// Concrete subclasses — declare here, define in PowerUp.cpp
// =========================================================

class SpeedBoostPowerUp : public PowerUp
{
public:
    SpeedBoostPowerUp(float x, float y);
    void applyEffect(Player& player) override;
};

class SnowballPowerPowerUp : public PowerUp
{
public:
    SnowballPowerPowerUp(float x, float y);
    void applyEffect(Player& player) override;
};

class DistanceIncreasePowerUp : public PowerUp
{
public:
    DistanceIncreasePowerUp(float x, float y);
    void applyEffect(Player& player) override;
};

class BalloonModePowerUp : public PowerUp
{
public:
    BalloonModePowerUp(float x, float y);
    void applyEffect(Player& player) override;
};

class ExtraLifePowerUp : public PowerUp
{
public:
    ExtraLifePowerUp(float x, float y);
    void applyEffect(Player& player) override;
};

class StarChoicePowerUp : public PowerUp
{
public:
    StarChoicePowerUp(float x, float y);
    void applyEffect(Player& player) override;
};