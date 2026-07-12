// =========================================================
// PowerUp.cpp — No memory library
// =========================================================

#include "PowerUp.h"
#include "Player.h"
#include <cmath>

PowerUp::PowerUp(float x, float y, PowerUpType type)
    : m_type(type)
{
    m_position = sf::Vector2f(x, y);
    m_hitBox = new HitBox(24.f, 24.f, 0.f, 0.f);
    m_hitBox->update(m_position);

    switch (type)
    {
    case PowerUpType::SpeedBoost:       m_color = sf::Color(255, 200, 0); break;
    case PowerUpType::SnowballPower:    m_color = sf::Color(100, 200, 255); break;
    case PowerUpType::DistanceIncrease: m_color = sf::Color(0, 255, 100); break;
    case PowerUpType::BalloonMode:      m_color = sf::Color(255, 100, 200); break;
    case PowerUpType::ExtraLife:        m_color = sf::Color(255, 50, 50); break;
    case PowerUpType::StarChoice:       m_color = sf::Color(255, 255, 0); break;
    }
}

void PowerUp::update(float dt)
{
    if (!m_active) return;

    m_lifetime -= dt;
    if (m_lifetime <= 0.f) { m_active = false; return; }

    m_bobTimer += dt;
    // BUG FIX: original moved position by delta each frame — it drifted.
    // Store a base Y and oscillate around it instead.
    m_position.y = m_baseY + sinf(m_bobTimer * 3.f) * 8.f;
    m_hitBox->update(m_position);
}

void PowerUp::draw(sf::RenderWindow& window)
{
    if (!m_active) return;

    sf::CircleShape icon(12.f);
    icon.setOrigin(12.f, 12.f);
    icon.setPosition(m_position);
    icon.setFillColor(m_color);
    icon.setOutlineColor(sf::Color::White);
    icon.setOutlineThickness(2.f);
    window.draw(icon);
}

void PowerUp::drawDebug(sf::RenderWindow& window)
{
    if (m_hitBox) m_hitBox->drawDebug(window, sf::Color::Cyan);
}

// =========================================================
// Concrete PowerUp subclasses — implementations
// =========================================================

// --- SpeedBoostPowerUp ---
SpeedBoostPowerUp::SpeedBoostPowerUp(float x, float y)
    : PowerUp(x, y, PowerUpType::SpeedBoost) {
}

void SpeedBoostPowerUp::applyEffect(Player& player)
{
    player.applySpeedBoost();
}

// --- SnowballPowerPowerUp ---
SnowballPowerPowerUp::SnowballPowerPowerUp(float x, float y)
    : PowerUp(x, y, PowerUpType::SnowballPower) {
}

void SnowballPowerPowerUp::applyEffect(Player& player)
{
    player.applySnowballPower();
}

// --- DistanceIncreasePowerUp ---
DistanceIncreasePowerUp::DistanceIncreasePowerUp(float x, float y)
    : PowerUp(x, y, PowerUpType::DistanceIncrease) {
}

void DistanceIncreasePowerUp::applyEffect(Player& player)
{
    player.applyDistanceIncrease();
}

// --- BalloonModePowerUp ---
BalloonModePowerUp::BalloonModePowerUp(float x, float y)
    : PowerUp(x, y, PowerUpType::BalloonMode) {
}

void BalloonModePowerUp::applyEffect(Player& player)
{
    player.applyBalloonMode();
}

// --- ExtraLifePowerUp ---
ExtraLifePowerUp::ExtraLifePowerUp(float x, float y)
    : PowerUp(x, y, PowerUpType::ExtraLife) {
}

void ExtraLifePowerUp::applyEffect(Player& player)
{
    player.addLife();
}

// --- StarChoicePowerUp ---
StarChoicePowerUp::StarChoicePowerUp(float x, float y)
    : PowerUp(x, y, PowerUpType::StarChoice) {
}

void StarChoicePowerUp::applyEffect(Player& player)
{
    // StarChoice: give player everything — adjust to your game logic
    player.applySpeedBoost();
    player.applySnowballPower();
    player.applyDistanceIncrease();
}