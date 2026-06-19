#include "Tornado.h"
#include "Constants.h"
#include <cstdlib>
#include <cmath>
// ---------------------------------------------------------
Tornado::Tornado(float x, float y, EnemyVariant variant)
    : FlyingEnemy(x, y, variant)
{
    m_tornadoTexture.setSmooth(false);
    m_tornadoHasTexture = false;

    switch (variant)
    {
    case EnemyVariant::Red:
        m_tornadoHasTexture = loadAndRecolor(m_tornadoTexture,
            "assets/images/Tornado_Red.png",
            EnemyVariant::Red,
            /*skipRecolor=*/true);
        break;

    case EnemyVariant::Orange:
        m_tornadoHasTexture = loadAndRecolor(m_tornadoTexture,
            "assets/images/Tornado_Red.png",
            EnemyVariant::Orange);
        break;

    case EnemyVariant::Green:
        m_tornadoHasTexture = loadAndRecolor(m_tornadoTexture,
            "assets/images/Tornado_Blue.png",
            EnemyVariant::Green);
        break;

    case EnemyVariant::Blue:
        m_tornadoHasTexture = loadAndRecolor(m_tornadoTexture,
            "assets/images/Tornado_Blue.png",
            EnemyVariant::Blue,
            /*skipRecolor=*/true);
        break;

    case EnemyVariant::Purple:
        m_tornadoHasTexture = loadAndRecolor(m_tornadoTexture,
            "assets/images/Tornado_Blue.png",
            EnemyVariant::Purple);
        break;
    }

    if (m_tornadoHasTexture)
        m_tornadoSprite.setTexture(m_tornadoTexture);

    setupTornadoClips();
    setTornadoAnim(TornadoAnim::Idle, true);

    m_speed = 80.f + static_cast<float>(rand() % 160);
    m_scoreValue = SCORE_TORNADO_MIN + (rand() % (SCORE_TORNADO_MAX - SCORE_TORNADO_MIN));
    applyVariantModifiers();

    float knifeBase = 2.f + static_cast<float>(rand() % 2);
    switch (m_variant)
    {
    case EnemyVariant::Green:  m_knifeInterval = knifeBase * 0.85f; break;
    case EnemyVariant::Blue:   m_knifeInterval = knifeBase * 0.70f; break;
    case EnemyVariant::Purple: m_knifeInterval = knifeBase * 0.55f; break;
    default:                   m_knifeInterval = knifeBase;          break;
    }

    delete m_hitBox;
    m_hitBox = new HitBox(38.f, 58.f, 7.f, 4.f);
    m_hitBox->update(m_position);
}

// ---------------------------------------------------------
// setupTornadoClips — every frame measured from Tornado_Blue.png (1250×1346)
// ---------------------------------------------------------
void Tornado::setupTornadoClips()
{
    using R = sf::IntRect;

    // ── IDLE ─────────────────────────────────────────────
    // Row 0, frame 0 only (standing still)
    {
        TornadoClip& c = m_tornadoClips[(int)TornadoAnim::Idle];
        c.frames[0] = R(24, 40, 91, 113);
        c.frameCount = 1;
        c.frameTime = 0.20f;
        c.loop = true;
    }

    // ── WALKING ───────────────────────────────────────────
    // Row 0, all 4 frames
    {
        TornadoClip& c = m_tornadoClips[(int)TornadoAnim::Walking];
        c.frames[0] = R(24, 40, 91, 113);
        c.frames[1] = R(149, 40, 117, 113);
        c.frames[2] = R(288, 40, 112, 113);
        c.frames[3] = R(426, 40, 117, 113);
        c.frameCount = 4;
        c.frameTime = 0.12f;
        c.loop = true;
    }

    // ── FLYING ────────────────────────────────────────────
    // Row 1, first 3 character frames (wing flap)
    // NOTE: frames at x≥544 on this row are watermark text — not used
    {
        TornadoClip& c = m_tornadoClips[(int)TornadoAnim::Flying_];
        c.frames[0] = R(36, 203, 88, 172);
        c.frames[1] = R(162, 203, 99, 172);
        c.frames[2] = R(296, 203, 104, 172);
        c.frameCount = 3;
        c.frameTime = 0.10f;
        c.loop = true;
    }

    // ── HURT ─────────────────────────────────────────────
    // Row 2, 3 frames — plays once when hit
    {
        TornadoClip& c = m_tornadoClips[(int)TornadoAnim::Hurt];
        c.frames[0] = R(19, 380, 108, 108);
        c.frames[1] = R(174, 380, 92, 108);
        c.frames[2] = R(296, 380, 87, 108);
        c.frameCount = 3;
        c.frameTime = 0.09f;
        c.loop = false;
    }

    // ── CROUCH (snow-trapped) ─────────────────────────────
    // Row 3, 2 frames
    {
        TornadoClip& c = m_tornadoClips[(int)TornadoAnim::Crouch];
        c.frames[0] = R(11, 536, 125, 133);
        c.frames[1] = R(162, 536, 95, 133);
        c.frameCount = 2;
        c.frameTime = 0.22f;
        c.loop = true;
    }

    // ── ATTACK POSE ───────────────────────────────────────
    // Row 4, 2 frames — pre-throw idle pose / idle attack stance
    {
        TornadoClip& c = m_tornadoClips[(int)TornadoAnim::AttackPose];
        c.frames[0] = R(23, 714, 105, 110);
        c.frames[1] = R(153, 714, 113, 110);
        c.frameCount = 2;
        c.frameTime = 0.18f;
        c.loop = true;
    }

    // ── THROW ─────────────────────────────────────────────
    // Row 6, first 4 character frames - full body spin while throwing
    // (knife projectile frames on row 5 are handled separately)
    {
        TornadoClip& c = m_tornadoClips[(int)TornadoAnim::Throw];
        c.frames[0] = R(24, 1047, 90, 129);
        c.frames[1] = R(141, 1047, 137, 129);
        c.frames[2] = R(305, 1047, 91, 129);
        c.frames[3] = R(418, 1047, 137, 129);
        c.frameCount = 4;
        c.frameTime = 0.08f;
        c.loop = true;
    }

    // ── FIRE DEATH ────────────────────────────────────────
    // Row 6 — 4 character frames followed by 4 fire-burst effect frames
    {
        TornadoClip& c = m_tornadoClips[(int)TornadoAnim::FireDeath];
        // Character burning
        c.frames[0] = R(24, 1047, 90, 129);
        c.frames[1] = R(141, 1047, 137, 129);
        c.frames[2] = R(305, 1047, 91, 129);
        c.frames[3] = R(418, 1047, 137, 129);
        // Fire-burst aftermath
        c.frames[4] = R(558, 1047, 81, 129);
        c.frames[5] = R(720, 1047, 83, 129);
        c.frames[6] = R(888, 1047, 81, 129);
        c.frames[7] = R(994, 1047, 89, 129);
        c.frameCount = 8;
        c.frameTime = 0.09f;
        c.loop = false;
    }

    // ── DYING (full explosion) ────────────────────────────
    // Row 7 — 8 frames: tumble → crumple → explode
    {
        TornadoClip& c = m_tornadoClips[(int)TornadoAnim::Dying];
        c.frames[0] = R(7, 1206, 133, 137);   // tumbling
        c.frames[1] = R(150, 1206, 124, 137);   // tumbling
        c.frames[2] = R(301, 1206, 90, 137);   // curled
        c.frames[3] = R(418, 1206, 137, 137);   // curled + legs
        c.frames[4] = R(582, 1206, 90, 137);   // crumpled
        c.frames[5] = R(695, 1206, 137, 137);   // squashed, wings out
        c.frames[6] = R(834, 1206, 136, 137);   // sparks / early explosion
        c.frames[7] = R(984, 1206, 116, 137);   // full explosion burst
        c.frameCount = 8;
        c.frameTime = 0.10f;
        c.loop = false;
    }
}

// ---------------------------------------------------------
// Knife-projectile source rects (row 5, right side)
// Call Tornado::getKnifeRect() to pick the sprite for a fired knife.
// ---------------------------------------------------------
sf::IntRect Tornado::getKnifeRect(int variant) const
{
    // variant 0=white 1=yellow 2=blue 3=red  (cycle with rand()%4)
    static const sf::IntRect knifeRects[4] = {
        sf::IntRect(284, 883, 61, 109),   // white
        sf::IntRect(356, 883, 60, 109),   // yellow
        sf::IntRect(427, 883, 60, 109),   // blue
        sf::IntRect(498, 883, 61, 109),   // red
    };
    return knifeRects[variant & 3];
}

// ---------------------------------------------------------
void Tornado::setTornadoAnim(TornadoAnim anim, bool restart)
{
    if (m_currentAnim == anim && !restart) return;
    m_currentAnim = anim;
    m_currentFrame = 0;
    m_animTimer = 0.f;
    m_animDone = false;
    applyTornadoFrame();
}

void Tornado::updateTornadoAnim(float dt)
{
    const TornadoClip& c = m_tornadoClips[(int)m_currentAnim];
    if (c.frameCount <= 1 || m_animDone) return;

    m_animTimer += dt;
    if (m_animTimer >= c.frameTime)
    {
        m_animTimer = 0.f;
        m_currentFrame++;
        if (m_currentFrame >= c.frameCount)
        {
            if (c.loop)
                m_currentFrame = 0;
            else
            {
                m_currentFrame = c.frameCount - 1;
                m_animDone = true;
            }
        }
        applyTornadoFrame();
    }
}

void Tornado::applyTornadoFrame()
{
    const TornadoClip& c = m_tornadoClips[(int)m_currentAnim];
    const sf::IntRect& r = c.frames[m_currentFrame];
    m_tornadoSprite.setTextureRect(r);
    m_tornadoSprite.setOrigin(r.width / 2.f, r.height / 2.f);
}

// ---------------------------------------------------------
void Tornado::update(float dt)
{
    if (!m_active)
    {
        if (m_currentAnim != TornadoAnim::Dying)
            setTornadoAnim(TornadoAnim::Dying, true);

        updateTornadoAnim(dt);
        return;
    }

    // Snow-encased: crouch anim, physics from base
    if (m_snowState == SnowState::Full)
    {
        FlyingEnemy::update(dt);
        setTornadoAnim(TornadoAnim::Crouch);
        updateTornadoAnim(dt);
        return;
    }

    // Throw-pose hold: freeze movement briefly
    if (m_inThrowPose)
    {
        m_throwTimer -= dt;
        updateTornadoAnim(dt);
        if (m_throwTimer <= 0.f)
        {
            m_inThrowPose = false;
            setTornadoAnim(
                m_flightState == FlightState::Flying
                ? TornadoAnim::Flying_
                : TornadoAnim::Idle,
                true);
        }
        return;
    }

    // Normal flight logic from FlyingEnemy
    FlyingEnemy::update(dt);

    // Pick animation based on flight state
    if (m_flightState == FlightState::Grounded)
    {
        if (std::abs(m_velocity.x) > 1.f)
            setTornadoAnim(TornadoAnim::Walking);
        else
            setTornadoAnim(TornadoAnim::Idle);
    }
    else if (m_flightState == FlightState::Flying)
    {
        m_speed = 60.f + static_cast<float>(rand() % 200);
        setTornadoAnim(TornadoAnim::Flying_);
    }
    else // Returning
    {
        setTornadoAnim(TornadoAnim::Flying_);
    }

    // Knife cooldown
    if (m_snowState != SnowState::Full)
    {
        m_knifeTimer += dt;
        if (m_knifeTimer >= m_knifeInterval)
        {
            m_fireKnife = true;
            m_knifeTimer = 0.f;
            // Reset interval — keep variant scaling so harder colours stay aggressive
            float knifeBase = 2.f + static_cast<float>(rand() % 2);
            switch (m_variant)
            {
            case EnemyVariant::Green:  m_knifeInterval = knifeBase * 0.85f; break;
            case EnemyVariant::Blue:   m_knifeInterval = knifeBase * 0.70f; break;
            case EnemyVariant::Purple: m_knifeInterval = knifeBase * 0.55f; break;
            default:                   m_knifeInterval = knifeBase;          break;
            }
            m_knifeVariant = rand() % 4;   // pick random knife colour

            setTornadoAnim(TornadoAnim::Throw, true);
            m_inThrowPose = true;
            m_throwTimer = 0.42f;
        }
    }

    updateTornadoAnim(dt);
}

// ---------------------------------------------------------
void Tornado::draw(sf::RenderWindow & window)
{
    if (!m_active && (m_currentAnim != TornadoAnim::Dying || m_animDone)) return;


    if (m_currentAnim == TornadoAnim::Dying || m_currentAnim == TornadoAnim::FireDeath)
    {
        const float t = static_cast<float>(m_currentFrame) / 7.f;
        const float radius = 28.f + t * 10.f;
        const float thickness = 9.f - t * 3.f;
        const bool redFrame = (m_currentFrame & 1) != 0;

        sf::CircleShape ring(radius);
        ring.setOrigin(radius, radius);
        ring.setPosition(m_position.x + 32.f, m_position.y + 32.f);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(thickness);
        ring.setOutlineColor(redFrame
            ? sf::Color(230, 55, 35, 230)
            : sf::Color(185, 55, 235, 230));
        window.draw(ring);

        sf::CircleShape glow(radius - thickness * 0.45f);
        glow.setOrigin(radius - thickness * 0.45f, radius - thickness * 0.45f);
        glow.setPosition(m_position.x + 32.f, m_position.y + 32.f);
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineThickness(2.f);
        glow.setOutlineColor(redFrame
            ? sf::Color(255, 180, 40, 220)
            : sf::Color(255, 190, 65, 220));
        window.draw(glow);
        return;
    }
    // Fully encased — white snowball
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

    if (m_tornadoHasTexture)
    {
        const float targetH = 64.f;
        const sf::IntRect& r = m_tornadoSprite.getTextureRect();
        const float scale = (r.height > 0)
            ? targetH / static_cast<float>(r.height)
            : 1.f;

        const float renderedW = r.width * scale;

        // Always draw with top-left at m_position so the hitbox (offset from
        // m_position) lines up correctly regardless of facing direction.
        // origin stays at (0, 0); we shift the draw position by renderedW when
        // flipping so the sprite still starts at m_position.x on screen.
        m_tornadoSprite.setOrigin(0.f, 0.f);
        if (m_facingRight)
        {
            // Flip horizontally: scale X negative, shift right by renderedW
            m_tornadoSprite.setScale(-scale, scale);
            m_tornadoSprite.setPosition(m_position.x + renderedW, m_position.y);
        }
        else
        {
            // Normal direction: no flip needed
            m_tornadoSprite.setScale(scale, scale);
            m_tornadoSprite.setPosition(m_position.x, m_position.y);
        }
        window.draw(m_tornadoSprite);
    }
    else
    {
        sf::RectangleShape shape(sf::Vector2f(32.f, 32.f));
        shape.setPosition(m_position);
        shape.setFillColor(sf::Color(80, 0, 120));
        window.draw(shape);
    }

    // Partial snow overlay
    if (m_snowState == SnowState::Partial)
    {
        sf::RectangleShape snow(sf::Vector2f(32.f, 14.f));
        snow.setPosition(m_position);
        snow.setFillColor(sf::Color(255, 255, 255, 180));
        window.draw(snow);
    }
}

// ---------------------------------------------------------
bool Tornado::shouldFireKnife()
{
    if (m_fireKnife) { m_fireKnife = false; return true; }
    return false;
}

void Tornado::setPlayerPosition(const sf::Vector2f & playerPos)
{
    m_playerPos = playerPos;
}

sf::Vector2f Tornado::getKnifeDirection() const
{
    sf::Vector2f dir = m_playerPos - m_position;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len == 0.f) return sf::Vector2f(-1.f, 0.f);
    return dir / len;
}
