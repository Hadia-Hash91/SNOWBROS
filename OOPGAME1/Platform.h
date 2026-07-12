#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include<cstdio>
#include "HitBox.h"

class Platform
{
public:
    Platform();
    Platform(float x, float y, float width, float height = 16.f);

    void load(const std::string& imagePath);
    void draw(sf::RenderWindow& window);
    void drawDebug(sf::RenderWindow& window);

    sf::FloatRect getRect() const { return m_rect; }
    float getTop() const { return m_rect.top; }
    bool hasImage() const { return m_hasImage; }

private:
    sf::FloatRect m_rect;
    sf::Texture   m_texture;
    sf::Sprite    m_sprite;
    HitBox        m_hitBox;
    bool          m_hasImage = false;
};