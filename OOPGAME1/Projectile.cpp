// =========================================================
// Projectile.cpp
// =========================================================

#include "Projectile.h"
#include "Constants.h"   // WINDOW_WIDTH, WINDOW_HEIGHT

Projectile::Projectile(float x, float y, sf::Vector2f direction,
    float speed, int damage)
    : m_direction(direction), m_speed(speed), m_damage(damage)
{
    m_position = sf::Vector2f(x, y);
    m_hitBox = new HitBox(12.f, 12.f, 0.f, 0.f);
    m_hitBox->update(m_position);
}

void Projectile::update(float dt)
{
    if (!m_active) return;

    // Move in direction at speed
    m_position.x += m_direction.x * m_speed * dt;
    m_position.y += m_direction.y * m_speed * dt;

    m_hitBox->update(m_position);

    // Deactivate if off screen
    if (m_position.x < -20.f || m_position.x > WINDOW_WIDTH + 20.f ||
        m_position.y < -20.f || m_position.y > WINDOW_HEIGHT + 20.f)
    {
        m_active = false;
    }
}

void Projectile::draw(sf::RenderWindow& window)
{
    if (!m_active) return;

    sf::CircleShape shape(6.f);
    shape.setOrigin(6.f, 6.f);
    shape.setPosition(m_position);
    shape.setFillColor(sf::Color(200, 230, 255));        // light blue snowball
    shape.setOutlineColor(sf::Color::White);
    shape.setOutlineThickness(1.5f);
    window.draw(shape);
}

void Projectile::drawDebug(sf::RenderWindow& window)
{
    if (m_hitBox) m_hitBox->drawDebug(window, sf::Color::Yellow);
}