// =========================================================
// HitBox.cpp
// =========================================================

#include "HitBox.h"

HitBox::HitBox(float width, float height, float offsetX, float offsetY)
    : m_width(width), m_height(height), m_offsetX(offsetX), m_offsetY(offsetY)
{
    m_rect = sf::FloatRect(0.f, 0.f, width, height);
}

void HitBox::update(const sf::Vector2f& entityPos)
{
    m_rect.left = entityPos.x + m_offsetX;
    m_rect.top = entityPos.y + m_offsetY;
}

sf::FloatRect HitBox::getRect() const
{
    return m_rect;
}

bool HitBox::intersects(const HitBox& other) const
{
    return m_rect.intersects(other.getRect());
}

void HitBox::drawDebug(sf::RenderWindow& window, sf::Color color) const
{
    sf::RectangleShape outline(sf::Vector2f(m_rect.width, m_rect.height));
    outline.setPosition(m_rect.left, m_rect.top);
    outline.setFillColor(sf::Color::Transparent);
    outline.setOutlineColor(color);
    outline.setOutlineThickness(2.f);
    window.draw(outline);
}