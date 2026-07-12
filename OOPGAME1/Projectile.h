#pragma once
// =========================================================
// Projectile.h
// =========================================================

#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "HitBox.h"

class Projectile : public Entity
{
public:
    Projectile(float x, float y, sf::Vector2f direction,
        float speed, int damage);
    virtual ~Projectile() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawDebug(sf::RenderWindow& window) override;

    // FIX 1: virtual onHit() so Snowball::onHit() override compiles (C3668)
    virtual void onHit() { m_active = false; }

    bool isActive()  const { return m_active; }
    int  getDamage() const { return m_damage; }

    // FIX 2: return HitBox& not HitBox* — fixes C2664 at ALL call sites
    // in LevelManager where intersects(HitBox&) was being passed a HitBox*
    HitBox& getHitBox() const { return *m_hitBox; }

protected:
    sf::Vector2f m_direction;
    float        m_speed;
    int          m_damage;
};