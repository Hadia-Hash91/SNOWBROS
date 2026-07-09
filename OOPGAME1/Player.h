#pragma once


#include <SFML/Graphics.hpp>
#include "Entity.h"
#include "HitBox.h"
#include "Snowball.h"
#include "Constants.h"
#include "CharacterData.h"

class Platform;

// Forward declaration — full definition is in Player.cpp
struct CharacterSheet;

enum AnimState
{
    ANIM_IDLE = 0,
    ANIM_WALK = 1,
    ANIM_FLY = 2,
    ANIM_DASH = 3,
    ANIM_THROW = 4,
    ANIM_HIT = 5,
    ANIM_JUMP = 6,
    ANIM_BALLOON = 7
};

// =========================================================float getVelocityY() const { return m_velocity.y; }
class Player : public Entity
{
private:
    sf::Texture m_texture;
    sf::Sprite  m_sprite;
    bool        m_hasTexture = false;
    float       m_prevY = 0.f;
    bool        m_throwKeyHeld = false;
    static constexpr float DRAW_WIDTH = 36.f;
    static constexpr float DRAW_HEIGHT = 80.f;

    // Active spritesheet config (set in constructor via findSheet)
    const CharacterSheet* m_pSheet = nullptr;

    // Animation state machine
    AnimState m_animState = ANIM_IDLE;
    int       m_animRow = 0;
    int       m_animFrame = 0;
    float     m_animTimer = 0.f;

    float m_throwAnimTimer = 0.f;
    float m_hitAnimTimer = 0.f;


    // Balloon visuals
    float m_balloonPulse = 0.f;
    float m_fatScale = 1.0f;

    void updateAnimation(float dt);

public:
    Player(float x, float y, int playerIndex, const char* name);
    Player(float x, float y, int playerIndex, const CharacterData* charData);
    virtual ~Player() = default;

    void handleInput(float dt);
    void update(float dt)                    override;
    void draw(sf::RenderWindow& window)      override;
    void drawDebug(sf::RenderWindow& window) override;

    void resolvePlatformCollision(const Platform* platforms, int count);
    void takeDamage();
    void resetForNewLevel();

    Snowball* tryThrow();
    void applySpeedBoost(float duration = -1.f);
    void applySnowballPower();
    void applyDistanceIncrease();
    void applyBalloonMode(float duration = -1.f);
    void addLife();
    void updatePowerUps(float dt);
    void resetLevelPowerUps();

    void applyKeyBindings(sf::Keyboard::Key left,
        sf::Keyboard::Key right,
        sf::Keyboard::Key jump,
        sf::Keyboard::Key throwKey)
    {
        m_keyLeft = left;
        m_keyRight = right;
        m_keyJump = jump;
        m_keyThrow = throwKey;
    }
    void bounce() { 
        m_velocity.y = JUMP_FORCE * 0.8f;
    m_onGround = false; 
    }

    float getVelocityY() const { return m_velocity.y; }

    // ── Getters ──────────────────────────────────────────
    int         getLives()  const { return m_lives; }
    int         getScore()  const { return m_score; }
    int         getGems()   const { return m_gems; }
    const char* getName()   const { return m_name; }
    bool        isAlive()   const { return m_lives > 0; }
    void restoreLives(int count) { m_lives = count; }
    void resetVelocity() { m_velocity = sf::Vector2f(0.f, 0.f); }

    bool isInvincible()  const { return m_invincibleTimer > 0.f; }
    bool isPowered()     const { return m_snowballPowered; }
    bool isMaxRange()    const { return m_maxRange; }
    bool isFacingRight() const { return m_facingRight; }
    bool isOnGround()    const { return m_onGround; }
    bool isBalloon()     const { return m_balloonTimer > 0.f; }

    void addScore(int points) { m_score += points; }
    void addGems(int count) { m_gems += count; }
    bool spendGems(int cost);

    // ── Save / restore ───────────────────────────────────
    struct SaveData
    {
        int   lives, score, gems;
        float posX, posY;
        int   charIdx;
        char  name[32];
    };
    SaveData getSaveData(int charIdx) const
    {
        SaveData d;
        d.lives = m_lives;
        d.score = m_score;
        d.gems = m_gems;
        d.posX = m_position.x;
        d.posY = m_position.y;
        d.charIdx = charIdx;
        return d;
    }
    void applyRestoreData(const SaveData& d)
    {
        m_lives = d.lives;
        m_score = d.score;
        m_gems = d.gems;
        m_position = sf::Vector2f(d.posX, d.posY);
    }
    HitBox& getHitBox() const { return *m_hitBox; }

private:
    const char* m_name;
    int         m_playerIndex;

    int   m_lives = PLAYER_START_LIVES;
    int   m_score = 0;
    int   m_gems = 0;

    sf::Vector2f m_velocity = sf::Vector2f(0.f, 0.f);
    bool         m_onGround = false;

    float m_invincibleTimer = 0.f;
    float m_throwCooldown = 0.f;
    bool  m_throwPressed = false;

    bool  m_snowballPowered = false;
    bool  m_maxRange = false;
    float m_speedBoostTimer = 0.f;
    float m_balloonTimer = 0.f;

    sf::Keyboard::Key m_keyLeft;
    sf::Keyboard::Key m_keyRight;
    sf::Keyboard::Key m_keyJump;
    sf::Keyboard::Key m_keyThrow;

    float     m_speedMult = 1.0f;
    float     m_throwRate = 1.0f;
    sf::Color m_bodyColor;
    sf::Color m_accentColor;
    bool      m_nativeMaxRange = false;

    void initKeyBindings();
    void applyCharacterData(const CharacterData& cd);
};
