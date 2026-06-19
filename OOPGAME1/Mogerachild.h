#pragma once


#include "Enemy.h"
#include "DropItem.h"

struct AnimClip
{
    int   frames[4];   // column indices on the sprite sheet (max 8)
    int   frameCount;
    float frameDur;    // seconds per frame
    bool  loop;       
};

// ── Child state machine 
enum class ChildState
{
    SpawnPop,   // tiny bounce on birth
    Walk,       // normal patrol
    Frozen      // encased in snow
};

class MogeraChild : public Enemy
{
public:
    MogeraChild(float x, float y, float dirX,
        sf::Texture* sharedTexture = nullptr);
    virtual ~MogeraChild() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawDebug(sf::RenderWindow& window) override;

    bool takeHit() override;
    void onChainKill() override;

    void tryJump();

    void resolvePlatformCollision(const sf::FloatRect* rects, int count);

    bool     hasDrop()       const { return m_hasDrop; }
    void     clearDrop() { m_hasDrop = false; }
    DropType getRandomDrop() const { return DropType::SushiSmall; }

private:
    // ── Texture ──
    sf::Texture* m_sharedTexture = nullptr;   // NOT owned; points to Mogera's texture
    sf::Sprite   m_childSprite;               // dedicated sprite for the child
    bool         m_hasTexture = false;

    // ── Movement ──
    float m_dirX;
    bool  m_onGround = false;

    // ── State ──
    ChildState m_state = ChildState::SpawnPop;
    float      m_stateTimer = 0.f;

    // ── Animation ──
    AnimClip m_clips[3];      
    int      m_clipFrame = 0;
    float    m_frameTimer = 0.f;

    // ── Misc ──
    float m_hitFlashTimer = 0.f;
    bool  m_hasDrop = false;

    void transitionTo(ChildState next);
    void advanceAnim(float dt);
    sf::IntRect currentTexRect() const;
};