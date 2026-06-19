#pragma once


#include "Entity.h"
#include "Constants.h"

enum class SnowState { None, Partial, Full };
enum class EnemyVariant { Red = 0, Green = 1, Blue = 2, Purple = 3, Orange = 4 };

class Enemy : public Entity
{
public:
    virtual ~Enemy() = default;

    virtual void update(float dt) override = 0;
    virtual void draw(sf::RenderWindow& window) override = 0;
    virtual void drawDebug(sf::RenderWindow& window) override;

    virtual bool takeHit() = 0;
    virtual void onChainKill() = 0;

    bool      isEncased()     const { return m_snowState == SnowState::Full; }
    bool      isPartial()     const { return m_snowState == SnowState::Partial; }
    SnowState getSnowState()  const { return m_snowState; }
    int       getHealth()     const { return m_health; }
    int       getScoreValue() const { return m_scoreValue; }
    int       getGemDrop()    const { return m_gemDrop; }

    void setVariant(EnemyVariant variant);

    sf::Color getVariantTint() const;

    
    void static recolorImageToVariant(sf::Image& img, EnemyVariant variant);

protected:
    Enemy(float x, float y, EnemyVariant variant = EnemyVariant::Red);

    int          m_health = 1;
    int          m_maxHealth = 1;
    int          m_hitsToEncase = 2;
    int          m_snowHits = 0;
    SnowState    m_snowState = SnowState::None;
    EnemyVariant m_variant = EnemyVariant::Red;
    bool m_hasTexture;
    float        m_speed = 80.f;
    sf::Vector2f m_velocity = sf::Vector2f(0.f, 0.f);

    int          m_scoreValue = 100;
    int          m_gemDrop = 1;
    float        m_escapeTimer = 5.f;
    float m_hitFlashTimer = 0.f;

    void applyGravity(float dt);
    void applyVariantModifiers();
    bool m_alive = true;

};