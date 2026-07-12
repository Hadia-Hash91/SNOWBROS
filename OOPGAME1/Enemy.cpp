// Enemy.cpp

#include "Enemy.h"

Enemy::Enemy(float x, float y, EnemyVariant variant)
    : m_variant(variant)
{
    m_position = sf::Vector2f(x, y);
    m_health = 1;
    m_maxHealth = 1;
    m_hitsToEncase = 2;
    m_speed = 100.f;
    m_scoreValue = SCORE_BOTOM_MIN;
    applyVariantModifiers();
}

void Enemy::applyVariantModifiers()
{
    switch (m_variant)
    {
    case EnemyVariant::Red:                                          break;
    case EnemyVariant::Orange: m_speed *= 1.15f; m_hitsToEncase += 1; break;
    case EnemyVariant::Green:  m_speed *= 1.25f; m_hitsToEncase += 1; break;
    case EnemyVariant::Blue:   m_speed *= 1.50f; m_hitsToEncase += 2; break;
    case EnemyVariant::Purple: m_speed *= 1.75f; m_hitsToEncase += 3; break;
    }
}


sf::Color Enemy::getVariantTint() const
{
    switch (m_variant)
    {
    case EnemyVariant::Orange: return sf::Color(255, 180, 80);    // orange wash
    case EnemyVariant::Green:  return sf::Color(160, 255, 160);   // green wash
    case EnemyVariant::Blue:   return sf::Color(160, 200, 255);   // blue wash
    case EnemyVariant::Purple: return sf::Color(210, 140, 255);   // purple wash
    default:                   return sf::Color::White;             // Red — no tint
    }
}

void Enemy::setVariant(EnemyVariant variant)
{
    m_variant = variant;
    applyVariantModifiers();
}


void  Enemy::recolorImageToVariant(sf::Image& img, EnemyVariant variant)
{
    unsigned int tr, tg, tb;
    switch (variant)
    {
    case EnemyVariant::Red:    tr = 220; tg = 80;  tb = 80;  break;
    case EnemyVariant::Orange: tr = 230; tg = 130; tb = 30;  break;
    case EnemyVariant::Green:  tr = 80;  tg = 210; tb = 80;  break;
    case EnemyVariant::Purple: tr = 180; tg = 80;  tb = 255; break;
    default:                   return;
    }

    unsigned int w = img.getSize().x;
    unsigned int h = img.getSize().y;

    for (unsigned int y = 0; y < h; ++y)
    {
        for (unsigned int x = 0; x < w; ++x)
        {
            sf::Color px = img.getPixel(x, y);
            if (px.a < 10) continue;
            unsigned int lum = px.r;
            if (px.g > lum) lum = px.g;
            if (px.b > lum) lum = px.b;

            px.r = static_cast<sf::Uint8>((lum * tr) / 255u);
            px.g = static_cast<sf::Uint8>((lum * tg) / 255u);
            px.b = static_cast<sf::Uint8>((lum * tb) / 255u);
            img.setPixel(x, y, px);
        }
    }
}

void Enemy::applyGravity(float dt)
{
    m_velocity.y += GRAVITY * dt;
}

void Enemy::drawDebug(sf::RenderWindow& window)
{
    if (m_hitBox) m_hitBox->drawDebug(window, sf::Color::Red);
}