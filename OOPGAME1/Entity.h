#pragma once


#include <SFML/Graphics.hpp>
#include "HitBox.h"

class Entity
{
public:
    virtual ~Entity()
    {
        delete m_hitBox;
        m_hitBox = nullptr;
    }

    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual void drawDebug(sf::RenderWindow& window) = 0;

    sf::Vector2f getPosition() const { return m_position; }
    void setPosition(const sf::Vector2f& pos)
    {
        m_position = pos;
        if (m_hitBox) m_hitBox->update(m_position);
    }

    bool isActive() const { return m_active; }
    void setActive(bool active) { m_active = active; }

    HitBox& getHitBox() { return *m_hitBox; }
    const HitBox& getHitBox() const { return *m_hitBox; }

protected:
    sf::Vector2f m_position = sf::Vector2f(0.f, 0.f);
    sf::Sprite   m_sprite;
    bool         m_active = true;
    bool         m_facingRight = true;
    HitBox* m_hitBox = nullptr;  // raw pointer, deleted in destructor
};