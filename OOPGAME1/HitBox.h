#pragma once
// =========================================================
// HitBox.h
// =========================================================

#include <SFML/Graphics.hpp>

class HitBox
{
public:
    HitBox(float width, float height, float offsetX = 0.f, float offsetY = 0.f);

    void          update(const sf::Vector2f& entityPos);
    sf::FloatRect getRect() const;
    bool          intersects(const HitBox& other) const;
    void          drawDebug(sf::RenderWindow& window, sf::Color color) const;

private:
    sf::FloatRect m_rect;
    float m_offsetX;
    float m_offsetY;
    float m_width;
    float m_height;
};