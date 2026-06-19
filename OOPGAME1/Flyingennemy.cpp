#include "FlyingEnemy.h"
#include "Enemy.h"
#include <cstdlib>
#include <cmath>

// ─────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────
FlyingEnemy::FlyingEnemy(float x, float y, EnemyVariant variant)
    : Botom(x, y, variant)
{
    m_texture.setSmooth(false);
    m_hasTexture = false;

    switch (variant)
    {
    case EnemyVariant::Blue:
        m_hasTexture = loadAndRecolor(m_texture,
            "assets/images/FlyingFoogaFoog_Blue.png",
            EnemyVariant::Blue,
            /*skipRecolor=*/true);
        break;

    case EnemyVariant::Orange:
        m_hasTexture = loadAndRecolor(m_texture,
            "assets/images/FlyingFoogaFoog_Orange.png",
            EnemyVariant::Orange,
            /*skipRecolor=*/true);
        break;

    case EnemyVariant::Red:
        m_hasTexture = loadAndRecolor(m_texture,
            "assets/images/FlyingFoogaFoog_Red.png",
            EnemyVariant::Red,
            /*skipRecolor=*/true);
        break;

    case EnemyVariant::Green:
        m_hasTexture = loadAndRecolor(m_texture,
            "assets/images/FlyingFoogaFoog_Blue.png",
            EnemyVariant::Green);
        break;

    case EnemyVariant::Purple:
        m_hasTexture = loadAndRecolor(m_texture,
            "assets/images/FlyingFoogaFoog_Blue.png",
            EnemyVariant::Purple);
        break;
    }

    if (m_hasTexture)
        m_sprite.setTexture(m_texture);

    setupClips();
    setAnimation(FlyAnim::Idle, true);

    m_speed = 120.f;
    m_scoreValue = SCORE_FOOGA_MIN + (rand() % (SCORE_FOOGA_MAX - SCORE_FOOGA_MIN));
    applyVariantModifiers();

    m_groundDuration = 2.5f + static_cast<float>(rand() % 3);
    m_flightDuration = 1.5f + static_cast<float>(rand() % 2);

    delete m_hitBox;
    m_hitBox = new HitBox(42.f, 48.f, 5.f, 12.f);
    m_hitBox->update(m_position);
}


void FlyingEnemy::setupClips()
{
    using R = sf::IntRect;

    // IDLE — single landed pose (row 2, frame 0)
    {
        auto& c = m_clips[(int)FlyAnim::Idle];
        c.frames[0] = R(28, 431, 151, 127);
        c.frameCount = 1; c.frameTime = 0.20f; c.loop = true;
    }

    // WALKING — two grounded poses (row 2, both frames)
    {
        auto& c = m_clips[(int)FlyAnim::Walking];
        c.frames[0] = R(28, 431, 151, 127);   // row2-f0
        c.frames[1] = R(220, 431, 151, 122);   // row2-f1
        c.frameCount = 2; c.frameTime = 0.18f; c.loop = true;
    }

    // FLYING_ — 3-frame wing-flap (row 0, all three frames)
    {
        auto& c = m_clips[(int)FlyAnim::Flying_];
        c.frames[0] = R(57, 32, 93, 157);    // row0-f0  upright
        c.frames[1] = R(220, 23, 161, 161);    // row0-f1  wings spread
        c.frames[2] = R(431, 23, 137, 156);    // row0-f2  wings back
        c.frameCount = 3; c.frameTime = 0.10f; c.loop = true;
    }

    // FALLING — 3 look-down frames (row 1, all three frames)
    {
        auto& c = m_clips[(int)FlyAnim::Falling];
        c.frames[0] = R(42, 239, 132, 137);   // row1-f0
        c.frames[1] = R(234, 259, 132, 117);   // row1-f1
        c.frames[2] = R(411, 224, 147, 152);   // row1-f2  (reused from old Flying_[3])
        c.frameCount = 3; c.frameTime = 0.12f; c.loop = true;
    }

    // FREEROAM — all 6 airborne frames blended for smooth 8-dir flight
    //            row0 f0-f2 → ascending/level  |  row1 f0-f2 → descending
    {
        auto& c = m_clips[(int)FlyAnim::FreeRoam];
        c.frames[0] = R(57, 32, 93, 157);   // row0-f0
        c.frames[1] = R(220, 23, 161, 161);   // row0-f1
        c.frames[2] = R(431, 23, 137, 156);   // row0-f2
        c.frames[3] = R(42, 239, 132, 137);   // row1-f0
        c.frames[4] = R(234, 259, 132, 117);   // row1-f1
        c.frames[5] = R(411, 224, 147, 152);   // row1-f2
        c.frameCount = 6; c.frameTime = 0.09f; c.loop = true;
    }

    // TRAPPED — 3 vulnerable wide-eyed frames (row 3)
    {
        auto& c = m_clips[(int)FlyAnim::Trapped];
        c.frames[0] = R(23, 608, 161, 142);   // row3-f0
        c.frames[1] = R(224, 638, 152, 112);   // row3-f1
        c.frames[2] = R(416, 662, 142, 88);   // row3-f2  (squashed peek)
        c.frameCount = 3; c.frameTime = 0.22f; c.loop = true;
    }

    // DYING — loose tumble + fully squashed (row 4 both frames)
    {
        auto& c = m_clips[(int)FlyAnim::Dying];
        c.frames[0] = R(47, 810, 137, 156);    // row4-f0  loose tumble
        c.frames[1] = R(416, 662, 142, 88);   // row4-f1  squashed flat (reuse)
        c.frameCount = 2; c.frameTime = 0.20f; c.loop = false;
    }
}

// ─────────────────────────────────────────────────────────────
// Animation helpers
// ─────────────────────────────────────────────────────────────
void FlyingEnemy::setAnimation(FlyAnim anim, bool restart)
{
    if (m_currentAnim == anim && !restart) return;
    m_currentAnim = anim;
    m_currentFrame = 0;
    m_animTimer = 0.f;
    m_animFinished = false;
    applyFrameRect();
}

void FlyingEnemy::updateAnimation(float dt)
{
    const AnimClip& c = m_clips[(int)m_currentAnim];
    if (c.frameCount <= 1 || m_animFinished) return;
    m_animTimer += dt;
    if (m_animTimer >= c.frameTime)
    {
        m_animTimer = 0.f;
        ++m_currentFrame;
        if (m_currentFrame >= c.frameCount)
        {
            if (c.loop) m_currentFrame = 0;
            else { m_currentFrame = c.frameCount - 1; m_animFinished = true; }
        }
        applyFrameRect();
    }
}

void FlyingEnemy::applyFrameRect()
{
    const AnimClip& c = m_clips[(int)m_currentAnim];
    const sf::IntRect& r = c.frames[m_currentFrame];
    m_sprite.setTextureRect(r);
    m_sprite.setOrigin(r.width / 2.f, r.height / 2.f);
}

// ─────────────────────────────────────────────────────────────
// Direction helpers — true 8-way movement
// ─────────────────────────────────────────────────────────────
void FlyingEnemy::pickNewDirection()
{
    // Pick one of 8 compass directions at random
    m_flyDir = static_cast<FlyDir>(rand() % 8);
}

sf::Vector2f FlyingEnemy::dirToVector(FlyDir d) const
{
    // Pre-normalised diagonal = 1/√2 ≈ 0.707
    constexpr float D = 0.7071f;
    switch (d)
    {
    case FlyDir::N:  return { 0.f, -1.f };
    case FlyDir::NE: return { D,  -D };
    case FlyDir::E:  return { 1.f,  0.f };
    case FlyDir::SE: return { D,   D };
    case FlyDir::S:  return { 0.f,  1.f };
    case FlyDir::SW: return { -D,   D };
    case FlyDir::W:  return { -1.f,  0.f };
    case FlyDir::NW: return { -D,  -D };
    default:         return { 0.f,  0.f };
    }
}

// ─────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────
void FlyingEnemy::update(float dt)
{
    if (!m_active) return;

    // ── Snow-trapped: delegate to base, play Trapped anim ──
    if (m_snowState == SnowState::Full)
    {
        Botom::update(dt);
        setAnimation(FlyAnim::Trapped);
        updateAnimation(dt);
        return;
    }

    // ── Grounded phase ──────────────────────────────────────
    if (m_flightState == FlightState::Grounded)
    {
        Botom::update(dt);
        m_groundTimer += dt;
        if (m_groundTimer >= m_groundDuration)
        {
            enterFlight();
            m_groundTimer = 0.f;
        }
        setAnimation(std::abs(m_velocity.x) > 1.f ? FlyAnim::Walking : FlyAnim::Idle);
    }

    // ── Flying phase — true 8-directional FreeRoam ──────────
    else if (m_flightState == FlightState::Flying)
    {
        m_dirChangeTimer += dt;
        if (m_dirChangeTimer >= 0.35f)
        {
            m_dirChangeTimer = 0.f;
            pickNewDirection();
        }

        sf::Vector2f vel = dirToVector(m_flyDir) * m_speed;
        m_position += vel * dt;

        // Facing: flip sprite on horizontal component
        if (vel.x > 0.5f) m_facingRight = true;
        else if (vel.x < -0.5f) m_facingRight = false;

        // Animation: FreeRoam cycles all 6 airborne frames;
        // the frame subset that shows naturally suits ascent vs descent.
        setAnimation(FlyAnim::FreeRoam);

        // Clamp to screen
        m_position.x = std::max(0.f, std::min(m_position.x, (float)WINDOW_WIDTH - 32.f));
        m_position.y = std::max(0.f, std::min(m_position.y, (float)WINDOW_HEIGHT - 32.f));

        m_flightTimer += dt;
        if (m_flightTimer >= m_flightDuration)
        {
            m_flightState = FlightState::Returning;
            m_flightTimer = 0.f;
        }

        m_hitBox->update(m_position);
    }

    // ── Returning phase — descend to ground ─────────────────
    else
    {
        m_position.y += m_speed * dt;
        setAnimation(FlyAnim::Falling);

        if (m_position.y > WINDOW_HEIGHT - 32.f)
            m_position.y = WINDOW_HEIGHT - 32.f;

        Botom::update(dt);
        m_hitBox->update(m_position);

        if (std::abs(m_velocity.y) < 0.01f)
            exitFlight();
    }

    updateAnimation(dt);
}

// ─────────────────────────────────────────────────────────────
// Draw
// ─────────────────────────────────────────────────────────────
void FlyingEnemy::draw(sf::RenderWindow& window)
{
    if (!m_active) return;

    if (m_snowState == SnowState::Full)
    {
        sf::CircleShape ball(18.f);
        ball.setPosition(m_position.x - 2.f, m_position.y - 2.f);
        ball.setFillColor(sf::Color(240, 240, 255));
        ball.setOutlineColor(sf::Color(150, 200, 255));
        ball.setOutlineThickness(3.f);
        window.draw(ball);
        return;
    }

    if (m_hasTexture)
    {
        const float targetSize = 64.f;
        const sf::IntRect& r = m_sprite.getTextureRect();
        const float scale = (r.height > 0)
            ? targetSize / static_cast<float>(r.height) : 1.f;

        m_sprite.setScale(m_facingRight ? scale : -scale, scale);
        m_sprite.setPosition(m_position.x + 16.f, m_position.y + 16.f);
        window.draw(m_sprite);
    }
    else
    {
        sf::RectangleShape shape({ 32.f, 32.f });
        shape.setPosition(m_position);
        shape.setFillColor(m_flightState == FlightState::Flying
            ? sf::Color(100, 200, 255) : sf::Color(80, 100, 220));
        window.draw(shape);
    }

    if (m_snowState == SnowState::Partial)
    {
        sf::RectangleShape snow({ 32.f, 14.f });
        snow.setPosition(m_position);
        snow.setFillColor(sf::Color(255, 255, 255, 180));
        window.draw(snow);
    }
}

// ─────────────────────────────────────────────────────────────
void FlyingEnemy::enterFlight()
{
    m_flightState = FlightState::Flying;
    m_flightTimer = 0.f;
    pickNewDirection();
}

void FlyingEnemy::exitFlight()
{
    m_flightState = FlightState::Grounded;
    m_groundTimer = 0.f;
    m_velocity.y = 0.f;
}