#pragma once
#include "Botom.h"

enum class FlightState { Grounded, Flying, Returning };

enum class FlyAnim
{
    Idle,
    Walking,
    Flying_,    // wing-flap (ascending/level)
    Falling,    // descending
    FreeRoam,   // NEW: full 8-dir cycle — uses ALL flight frames
    Trapped,
    Dying,
    Count
};

// 8 compass directions for free-roam flight
enum class FlyDir { N, NE, E, SE, S, SW, W, NW, None };

class FlyingEnemy : public Botom
{
public:
    FlyingEnemy(float x, float y, EnemyVariant variant = EnemyVariant::Red);
    virtual ~FlyingEnemy() = default;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void setGroundDuration(float s) { m_groundDuration = s; }
    void setFlightDuration(float s) { m_flightDuration = s; }

protected:
    FlightState  m_flightState = FlightState::Grounded;
    float        m_groundTimer = 0.f;
    float        m_groundDuration = 3.f;
    float        m_flightTimer = 0.f;
    float        m_flightDuration = 2.f;
    sf::Vector2f m_flightDir;

    void enterFlight();
    void exitFlight();

private:
    // ── direction helpers 
    FlyDir       m_flyDir = FlyDir::None;
    float        m_dirChangeTimer = 0.f;

    // ── sprite 
    sf::Texture  m_texture;
    sf::Sprite   m_sprite;
    bool         m_hasTexture = false;

    static const int MAX_FRAMES = 8;
    struct AnimClip {
        sf::IntRect frames[MAX_FRAMES];
        int   frameCount = 0;
        float frameTime = 0.12f;
        bool  loop = true;
    };

    AnimClip m_clips[(int)FlyAnim::Count];
    FlyAnim  m_currentAnim = FlyAnim::Idle;
    int      m_currentFrame = 0;
    float    m_animTimer = 0.f;
    bool     m_animFinished = false;

    void setupClips();
    void setAnimation(FlyAnim anim, bool restart = false);
    void updateAnimation(float dt);
    void applyFrameRect();
    void pickNewDirection();
    sf::Vector2f dirToVector(FlyDir d) const;
};