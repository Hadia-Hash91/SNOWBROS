#pragma once

#include "FlyingEnemy.h"

enum class TornadoAnim
{
    Idle,           // Row 0 frame 0 only
    Walking,        // Row 0 all 4 frames
    Flying_,        // Row 1 frames 0-2 (wing flap)
    Hurt,           // Row 2 frames 0-2
    Crouch,         // Row 3 frames 0-1 (snow-trapped)
    AttackPose,     // Row 4 frames 0-1 (pre-throw stance)
    Throw,          // Row 5 frames 0-1 (plays once)
    FireDeath,      // Row 6 frames 0-7 (plays once)
    Dying,          // Row 7 frames 0-7 (plays once)
    COUNT
};

class Tornado : public FlyingEnemy
{
public:
    Tornado(float x, float y, EnemyVariant variant = EnemyVariant::Red);
    virtual ~Tornado() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;

    bool         shouldFireKnife();
    void         setPlayerPosition(const sf::Vector2f& playerPos);
    sf::Vector2f getKnifeDirection() const;

    // Returns the sprite-sheet rect for the chosen knife colour (0-3)
    sf::IntRect  getKnifeRect(int variant = 0) const;
    int          getLastKnifeVariant() const { return m_knifeVariant; }

private:
    // ── Sprite 
    sf::Texture   m_tornadoTexture;
    sf::Sprite    m_tornadoSprite;
    bool          m_tornadoHasTexture = false;

    struct TornadoClip
    {
        sf::IntRect frames[8];
        int         frameCount = 1;
        float       frameTime = 0.15f;
        bool        loop = true;
    };
    TornadoClip   m_tornadoClips[(int)TornadoAnim::COUNT];
    TornadoAnim   m_currentAnim = TornadoAnim::Idle;
    int           m_currentFrame = 0;
    float         m_animTimer = 0.f;
    bool          m_animDone = false;

    void setupTornadoClips();
    void setTornadoAnim(TornadoAnim anim, bool restart = false);
    void updateTornadoAnim(float dt);
    void applyTornadoFrame();

    // ── Attack 
    float        m_knifeTimer = 0.f;
    float        m_knifeInterval = 2.5f;
    bool         m_fireKnife = false;
    int          m_knifeVariant = 0;   // 0=white 1=yellow 2=blue 3=red
    sf::Vector2f m_playerPos;

    // ── Throw pose hold 
    float        m_throwTimer = 0.f;
    bool         m_inThrowPose = false;
};