#pragma once


#include <SFML/Graphics.hpp>

class KnifeProjectile
{
public:
    KnifeProjectile() = default;
    KnifeProjectile(float x, float y, sf::Vector2f direction);

    void update(float dt);
    void draw(sf::RenderWindow& window);

    sf::FloatRect getRect()  const;
    bool isActive()          const { return m_active; }
    void setActive(bool v) { m_active = v; }

private:
    sf::Vector2f m_position;
    sf::Vector2f m_direction;   // normalised
    float        m_speed = 320.f;
    bool         m_active = false;

    
    static sf::Texture s_texture;
    static bool        s_loaded;

    sf::Sprite  m_sprite;
    float       m_animTimer = 0.f;
    int         m_frame = 0;        // 0..3

    
    static const sf::IntRect FRAMES[4];

    void ensureLoaded();
    void applyFrame();
};