#pragma once


#include "Enemy.h"
#include "DropItem.h"
#include "MogeraChild.h"  

class Player;

static constexpr int BOSS_FRAME_W = 490;
static constexpr int BOSS_FRAME_H = 300;

enum class BossState
{
    Idle,          
    HitRecoil,     
    SpawnWindUp,   
    SpawnRelease, 
    Dead
};


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
    MogeraChild* popChild();             

    bool     hasDrop()    const { return m_hasDrop; }
    void     clearDrop() { m_hasDrop = false; }
    DropType getDropType() const { return DropType::FaceRed; }

    sf::Texture* getTexture() { return m_hasTexture ? &m_texture : nullptr; }

private:
    sf::Texture m_texture;
    bool        m_hasTexture = false;


    sf::Sprite  m_legSprite;

    BossState m_state = BossState::Idle;
    float     m_stateTimer = 0.f;

    AnimClip m_clips[5];     
    int      m_clipFrame = 0;
    float    m_frameTimer = 0.f;

    float m_spawnAccum = 0.f;
    float m_spawnInterval = 3.5f;
    bool  m_pendingChild = false;
    float m_spawnDirX = 1.f;

    float m_hitFlashTimer = 0.f;

    bool m_hasDrop = false;

    Player** m_players = nullptr;
    int      m_playerCount = 0;

    void transitionTo(BossState next);
    void advanceAnim(float dt);

   
    sf::IntRect currentTexRect() const;

    sf::IntRect currentLegRect() const;

    void drawHealthBar(sf::RenderWindow& window) const;
    float spawnIntervalForHealth() const;
};