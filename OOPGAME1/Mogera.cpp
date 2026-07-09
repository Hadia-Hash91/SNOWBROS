
#include "Mogera.h"
#include "MogeraChild.h"
#include "Player.h"
#include "Constants.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>

// ── State-duration constants ───────────────────────────────
static constexpr float BOSS_HIT_RECOIL_DUR = 0.35f;
static constexpr float BOSS_SPAWN_WINDUP_DUR = 0.55f;
static constexpr float BOSS_SPAWN_RELEASE_DUR = 0.20f;

// ── tickClip helper ───────────────────────────────────────
static bool tickClip(const AnimClip& clip, int& clipFrame,
    float& frameTimer, float dt)
{
    frameTimer += dt;
    if (frameTimer >= clip.frameDur)
    {
        frameTimer -= clip.frameDur;
        clipFrame++;
        if (clipFrame >= clip.frameCount)
        {
            if (clip.loop)
                clipFrame = 0;
            else
            {
                clipFrame = clip.frameCount - 1;
                return true;
            }
        }
    }
    return false;
}

// =========================================================
//  Mogera (Boss)
// =========================================================

Mogera::Mogera(float x, float y)
    : Enemy(x, y, EnemyVariant::Red)
{
    m_health = 12;
    m_maxHealth = 12;
    m_hitsToEncase = 999;
    m_speed = 0.f;
    m_scoreValue = SCORE_MOGERA;
    m_gemDrop = GEMS_MOGERA_REWARD;

    // ── Load sprite ───────────────────────────────────────
    m_hasTexture = m_texture.loadFromFile("mogera.png");
    if (m_hasTexture)
    {
        sf::Image img = m_texture.copyToImage();
        for (unsigned iy = 0; iy < img.getSize().y; iy++)
            for (unsigned ix = 0; ix < img.getSize().x; ix++)
            {
                sf::Color c = img.getPixel(ix, iy);
                if (c.r < 15 && c.g < 15 && c.b < 15)
                    img.setPixel(ix, iy, sf::Color::Transparent);
            }
        m_texture.loadFromImage(img);
        m_sprite.setTexture(m_texture);
        m_sprite.setOrigin(0.f, 0.f);

        // Leg sprite shares the same texture — no extra load needed
        m_legSprite.setTexture(m_texture);
        m_legSprite.setOrigin(0.f, 0.f);
    }

    

    // Idle: 3-frame breathing loop (bust frames 0-1-2, legs follow automatically)
    {
        auto& c = m_clips[static_cast<int>(BossState::Idle)];
        c.frames[0] = 0; c.frames[1] = 1; c.frames[2] = 2;
        c.frameCount = 3;
        c.frameDur = 0.35f;
        c.loop = true;
    }

    // HitRecoil: hold the crouched full-body frame
    {
        auto& c = m_clips[static_cast<int>(BossState::HitRecoil)];
        c.frames[0] = 0;
        c.frameCount = 1;
        c.frameDur = 0.35f;
        c.loop = false;
    }

    // SpawnWindUp: alternate idle-pose bust (frame 0) and crouched (frame 1)
    {
        auto& c = m_clips[static_cast<int>(BossState::SpawnWindUp)];
        c.frames[0] = 0; c.frames[1] = 1;
        c.frameCount = 2;
        c.frameDur = 0.20f;
        c.loop = true;
    }

    // SpawnRelease: show the lying-flat frame once
    {
        auto& c = m_clips[static_cast<int>(BossState::SpawnRelease)];
        c.frames[0] = 0;
        c.frameCount = 1;
        c.frameDur = 0.20f;
        c.loop = false;
    }

    m_hitBox = new HitBox(128.f, 128.f, 0.f, 0.f);
    m_hitBox->update(m_position);

    transitionTo(BossState::Idle);
}

// ── State transition 

void Mogera::transitionTo(BossState next)
{
    m_state = next;
    m_stateTimer = 0.f;
    m_clipFrame = 0;
    m_frameTimer = 0.f;
}

// ── Advance animation 
void Mogera::advanceAnim(float dt)
{
    if (m_state == BossState::Dead) return;
    const AnimClip& clip = m_clips[static_cast<int>(m_state)];
    tickClip(clip, m_clipFrame, m_frameTimer, dt);
}


sf::IntRect Mogera::currentTexRect() const
{
    // Idle bust frames (upper body only — legs added in draw())
    static const sf::IntRect bustFrames[3] = {
        { 43,  6, 538, 461},
        {587,  6, 538, 461},
        {1167, 6, 537, 461},
    };

    switch (m_state)
    {
    case BossState::Idle:
    {
        int f = (m_clipFrame < 3) ? m_clipFrame : 2;
        return bustFrames[f];
    }

    case BossState::HitRecoil:
        // Full crouched body — includes legs
        return sf::IntRect(0, 528, 845, 584);

    case BossState::SpawnWindUp:
        // Even sub-frames: idle bust pose; odd: crouched full body
        if (m_clipFrame % 2 == 0)
            return bustFrames[0];
        else
            return sf::IntRect(0, 528, 845, 584);

    case BossState::SpawnRelease:
        // Lying flat — tight bounds within sheet
        return sf::IntRect(1009, 758, 781, 334);

    default:
        return bustFrames[0];
    }
}



sf::IntRect Mogera::currentLegRect() const
{
    static const sf::IntRect legFrames[3] = {
        {1983,   6, 473, 104},
        {1997, 214, 473, 225},
        {1975, 442, 495, 334},
    };

    if (m_state == BossState::Idle)
    {
        int f = (m_clipFrame < 3) ? m_clipFrame : 2;
        return legFrames[f];
    }

    // SpawnWindUp even sub-frame shows the idle bust — pair it with leg 0
    if (m_state == BossState::SpawnWindUp && m_clipFrame % 2 == 0)
        return legFrames[0];

    // All other states have legs built into the full-body frame
    return sf::IntRect(0, 0, 0, 0);
}

// ── Helper 

float Mogera::spawnIntervalForHealth() const
{
    float interval = 3.5f - (static_cast<float>(m_maxHealth - m_health) * 0.2f);
    return std::max(interval, 1.5f);
}

// ── update 

void Mogera::update(float dt)
{
    if (!m_active) return;

    m_stateTimer += dt;
    if (m_hitFlashTimer > 0.f) m_hitFlashTimer -= dt;

    switch (m_state)
    {
    case BossState::Idle:
        m_spawnAccum += dt;
        if (m_spawnAccum >= m_spawnInterval)
        {
            m_spawnAccum = 0.f;
            transitionTo(BossState::SpawnWindUp);
        }
        break;

    case BossState::HitRecoil:
        if (m_stateTimer >= BOSS_HIT_RECOIL_DUR)
            transitionTo(BossState::Idle);
        break;

    case BossState::SpawnWindUp:
        if (m_stateTimer >= BOSS_SPAWN_WINDUP_DUR)
        {
            m_pendingChild = true;
            m_spawnDirX *= -1.f;
            m_spawnInterval = spawnIntervalForHealth();
            transitionTo(BossState::SpawnRelease);
        }
        break;

    case BossState::SpawnRelease:
        if (m_stateTimer >= BOSS_SPAWN_RELEASE_DUR)
            transitionTo(BossState::Idle);
        break;

    case BossState::Dead:
        break;
    }

    advanceAnim(dt);
    m_hitBox->update(m_position);
}

// ── popChild

MogeraChild* Mogera::popChild()
{
    if (!m_pendingChild) return nullptr;
    m_pendingChild = false;

    float cx = m_position.x + (m_spawnDirX > 0 ? 100.f : -24.f);
    float cy = m_position.y + 20.f;

    return new MogeraChild(cx, cy, m_spawnDirX,
        m_hasTexture ? &m_texture : nullptr);
}

// ── takeHit

bool Mogera::takeHit()
{
    if (m_state == BossState::Dead) return false;

    m_health--;
    m_hitFlashTimer = 0.25f;

    if (m_health <= 0)
    {
        transitionTo(BossState::Dead);
        m_active = false;
        m_hasDrop = true;
        return false;
    }

    if (m_state != BossState::SpawnWindUp && m_state != BossState::SpawnRelease)
        transitionTo(BossState::HitRecoil);

    return false;
}

// ── onChainKill ───────────────────────────────────────────

void Mogera::onChainKill()
{
    if (m_state == BossState::Dead) return;

    m_health -= 2;
    m_hitFlashTimer = 0.35f;

    if (m_health <= 0)
    {
        transitionTo(BossState::Dead);
        m_active = false;
        m_hasDrop = true;
        return;
    }

    if (m_state != BossState::SpawnWindUp && m_state != BossState::SpawnRelease)
        transitionTo(BossState::HitRecoil);
}



void Mogera::draw(sf::RenderWindow& window)
{
    if (!m_active && m_state != BossState::Dead) return;
    if (m_state == BossState::Dead) return;

    const bool flashing = (m_hitFlashTimer > 0.f) &&
        (static_cast<int>(m_hitFlashTimer * 20) % 2 == 0);

    sf::Color tint = flashing ? sf::Color(255, 80, 80) : sf::Color::White;

    if (m_hasTexture)
    {
        // ── Fixed on-screen width for all states ──────────
        constexpr float SCREEN_W = 128.f;

        // Foot anchor: the bottom of the boss always sits here.
        // This must match the bottom of the hitbox so collision stays consistent.
        const float footY = m_position.y + 105.f;

        // Primary (bust / full-body) rect
        sf::IntRect bodyRect = currentTexRect();
        sf::IntRect legRect = currentLegRect();

        const bool hasLegs = (legRect.width > 0);

        // ── Compute scale for body rect ────────────────────
        float bodyScaleX = SCREEN_W / static_cast<float>(bodyRect.width);
        float bodyScaleY = bodyScaleX;
        float bodyScreenH = static_cast<float>(bodyRect.height) * bodyScaleY;

        // ── Compute scale for leg rect (same screen width) ──
        float legScreenH = 0.f;
        float legScaleX = 0.f;
        float legScaleY = 0.f;
        if (hasLegs)
        {
            legScaleX = SCREEN_W / static_cast<float>(legRect.width);
            legScaleY = legScaleX;
            legScreenH = static_cast<float>(legRect.height) * legScaleY;
        }

        bool flipX = (m_state == BossState::SpawnRelease && m_spawnDirX < 0);

        if (hasLegs)
        {
            m_legSprite.setTextureRect(legRect);

            float sx = flipX ? -legScaleX : legScaleX;
            m_legSprite.setScale(sx, legScaleY);

            m_legSprite.setOrigin(
                static_cast<float>(legRect.width) / 2.f,
                static_cast<float>(legRect.height));

            m_legSprite.setPosition(m_position.x + 64.f, footY);

            m_legSprite.setColor(tint);
            window.draw(m_legSprite);
        }

        // ── Draw body (bust or full-body) — sits above legs ──
        {
            m_sprite.setTextureRect(bodyRect);

            float sx = flipX ? -bodyScaleX : bodyScaleX;
            m_sprite.setScale(sx, bodyScaleY);

            // Origin at bottom-centre of source rect
            m_sprite.setOrigin(
                static_cast<float>(bodyRect.width) / 2.f,
                static_cast<float>(bodyRect.height));

            
            float bodyBottomY = hasLegs ? (footY - legScreenH) : footY;
            m_sprite.setPosition(m_position.x + 64.f, bodyBottomY);

            m_sprite.setColor(tint);
            window.draw(m_sprite);
        }
    }
    else
    {
        // ── Fallback shapes ───────────────────────────────
        sf::Color bodyCol;
        switch (m_state)
        {
        case BossState::HitRecoil:    bodyCol = sf::Color(255, 80, 80);  break;
        case BossState::SpawnWindUp:  bodyCol = sf::Color(30, 130, 60);  break;
        case BossState::SpawnRelease: bodyCol = sf::Color(80, 200, 80);  break;
        default:
            bodyCol = flashing
                ? sf::Color(255, 80, 80)
                : (m_clipFrame == 0 ? sf::Color(20, 100, 40) : sf::Color(15, 80, 30));
            break;
        }

        sf::RectangleShape body(sf::Vector2f(128.f, 128.f));
        body.setPosition(m_position);
        body.setFillColor(bodyCol);
        body.setOutlineColor(sf::Color(0, 60, 20));
        body.setOutlineThickness(3.f);
        window.draw(body);

        if (m_state != BossState::HitRecoil)
        {
            sf::CircleShape eye(10.f, 20);
            eye.setFillColor(sf::Color(255, 200, 0));
            eye.setOutlineColor(sf::Color::Black);
            eye.setOutlineThickness(1.f);
            eye.setPosition(m_position.x + 14.f, m_position.y + 22.f);
            window.draw(eye);
            eye.setPosition(m_position.x + 60.f, m_position.y + 22.f);
            window.draw(eye);
        }

        if (m_state == BossState::SpawnWindUp)
        {
            sf::RectangleShape exclaim(sf::Vector2f(8.f, 22.f));
            exclaim.setFillColor(sf::Color(255, 230, 0));
            exclaim.setPosition(m_position.x + 44.f, m_position.y - 32.f);
            window.draw(exclaim);
            sf::CircleShape dot(4.f);
            dot.setFillColor(sf::Color(255, 230, 0));
            dot.setPosition(m_position.x + 44.f, m_position.y - 6.f);
            window.draw(dot);
        }
    }

    drawHealthBar(window);
}

// ── drawHealthBar ─────────────────────────────────────────

void Mogera::drawHealthBar(sf::RenderWindow& window) const
{
    const float barW = 200.f;
    const float barH = 14.f;
    const float barX = (WINDOW_WIDTH - barW) / 2.f;
    const float barY = 12.f;

    sf::RectangleShape bg(sf::Vector2f(barW, barH));
    bg.setPosition(barX, barY);
    bg.setFillColor(sf::Color(40, 10, 10));
    bg.setOutlineColor(sf::Color(120, 40, 40));
    bg.setOutlineThickness(1.5f);
    window.draw(bg);

    float ratio = static_cast<float>(m_health) /
        static_cast<float>(m_maxHealth);
    ratio = std::max(0.f, std::min(ratio, 1.f));

    sf::RectangleShape fill(sf::Vector2f(barW * ratio, barH));
    fill.setPosition(barX, barY);
    sf::Uint8 r = static_cast<sf::Uint8>(255.f * (1.f - ratio));
    sf::Uint8 g = static_cast<sf::Uint8>(255.f * ratio);
    fill.setFillColor(sf::Color(r, g, 30));
    window.draw(fill);

    float segW = barW / static_cast<float>(m_maxHealth);
    sf::RectangleShape notch(sf::Vector2f(1.f, barH));
    notch.setFillColor(sf::Color(0, 0, 0, 120));
    for (int i = 1; i < m_maxHealth; i++)
    {
        notch.setPosition(barX + i * segW, barY);
        window.draw(notch);
    }
}

// ── drawDebug
//
void Mogera::drawDebug(sf::RenderWindow& window)
{
    if (m_hitBox) m_hitBox->drawDebug(window, sf::Color::Magenta);
}