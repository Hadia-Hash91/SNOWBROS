#pragma once
#pragma once


#include <SFML/Graphics.hpp>
#include "Enemy.h"
#include "DropItem.h"

enum class BotomAnim
{
    Idle = 0,
    Idle2 = 1,
    Charging = 2,
    Walking = 3,
    Jumping = 4,
    Falling = 5,
    Trapped = 6,
    Escape = 7,
    Staring = 8,
    Dying = 9,
};

class Botom : public Enemy
{
private:
    float m_moveDir = 1.f;


protected:
    static bool loadAndRecolor(sf::Texture& tex,
        const std::string& path,
        EnemyVariant targetVariant,
        bool skipRecolor = false);

public:
    Botom(float x, float y, EnemyVariant variant = EnemyVariant::Red);
    virtual ~Botom() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawDebug(sf::RenderWindow& window) override;

    void resolvePlatformCollision(const sf::FloatRect* platforms, int count);
    void tryJump(); 

    bool     takeHit()             override;
    void     onChainKill()         override;
    void     setOnGround(bool val) { m_onGround = val; }
    bool     hasDrop()       const { return m_hasDrop; }
    DropType getRandomDrop() const;
    void     clearDrop() { m_hasDrop = false; }

protected:
    sf::Texture m_texture;
    sf::Sprite  m_sprite;

    float      m_directionTimer = 0.f;
    float      m_directionInterval = 2.f;
    bool       m_hasDrop = false;
    bool       m_onGround = false;
    float      m_speed = 80.f;
    bool       m_hasTexture = false;  

    int        m_frame = 0;
    float      m_animTimer = 0.f;
    float      m_animSpeed = 0.15f;

    int        m_frameWidth = 96;
    int        m_frameHeight = 106;

    BotomAnim  m_animState = BotomAnim::Walking;
    BotomAnim  m_prevAnimState = BotomAnim::Walking;

    static constexpr int ANIM_FRAMES[10] = { 2, 1, 2, 3, 2, 2, 2, 3, 2, 6 };

    static constexpr int ANIM_ROW_Y[10] = {
        0,     // row 0 — Unused/Idle
        115,   // row 1 — Idle
        230,   // row 2 — Charging
        344,   // row 3 — Walking
        459,   // row 4 — Jumping
        570,   // row 5 — Falling
        681,   // row 6 — Trapped
        800,   // row 7 — Escape
        916,   // row 8 — Staring
        1030   // row 9 — Dying
    };

    void updateAnimation(float dt);
    void setAnimState(BotomAnim state);
    void applyFrameRect();
};