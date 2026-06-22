#pragma once


#include "Enemy.h"
#include "DropItem.h"
#include "Mogerachild.h"   // MogeraChild, AnimClip, ChildState all live here

class Player;

// Boss render size on screen (bust portion for Idle frames)
static constexpr int BOSS_FRAME_W = 490;
static constexpr int BOSS_FRAME_H = 300;

// ── Boss state machine ─────────────────────────────────────
enum class BossState
{
    Idle,          // breathing idle loop  (bust + legs composite)
    HitRecoil,     // brief pause + flash after taking a hit  (crouched, self-contained)
    SpawnWindUp,   // wind-up before releasing a child        (alternates idle/crouched)
    SpawnRelease,  // one-frame "pop" when child exits        (lying flat, self-contained)
    Dead
};

// =========================================================
//  Mogera (Boss)
// =========================================================
class Mogera : public Enemy
{
public:
    Mogera(float x, float y);
    virtual ~Mogera() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void drawDebug(sf::RenderWindow& window) override;

    bool takeHit() override;
    void onChainKill() override;

    void setPlayerRef(Player** players, int count)
    {
        m_players = players;
        m_playerCount = count;
    }

    bool         hasChildToSpawn() const { return m_pendingChild; }
    MogeraChild* popChild();              // caller owns the pointer

    bool     hasDrop()    const { return m_hasDrop; }
    void     clearDrop() { m_hasDrop = false; }
    DropType getDropType() const { return DropType::FaceRed; }

    // Give access to texture so spawned children can share it
    sf::Texture* getTexture() { return m_hasTexture ? &m_texture : nullptr; }

private:
    // ── Appearance ──
    sf::Texture m_texture;
    bool        m_hasTexture = false;

    // Second sprite used only for the leg portion of Idle frames
    sf::Sprite  m_legSprite;

    // ── State machine ──
    BossState m_state = BossState::Idle;
    float     m_stateTimer = 0.f;

    // ── Animation ──
    AnimClip m_clips[5];     // indexed by BossState (Dead has no clip)
    int      m_clipFrame = 0;
    float    m_frameTimer = 0.f;

    // ── Spawn logic ──
    float m_spawnAccum = 0.f;
    float m_spawnInterval = 3.5f;
    bool  m_pendingChild = false;
    float m_spawnDirX = 1.f;

    // ── Hit flash ──
    float m_hitFlashTimer = 0.f;

    bool m_hasDrop = false;

    Player** m_players = nullptr;
    int      m_playerCount = 0;

    // ── Helpers ──
    void transitionTo(BossState next);
    void advanceAnim(float dt);

    // Returns the primary (bust/body) texture rect for the current frame.
    // For Idle, this is the upper-body rect only.
    sf::IntRect currentTexRect() const;

    // Returns the leg rect for the current Idle frame (only valid in Idle state).
    // Returns a zero-sized rect for all other states.
    sf::IntRect currentLegRect() const;

    void drawHealthBar(sf::RenderWindow& window) const;
    float spawnIntervalForHealth() const;
};