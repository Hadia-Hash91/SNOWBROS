// =========================================================
// Snowball.cpp  —  Pink energy-ball projectile (Blossom)
//
// Visual design matches Blossom's spritesheet rows 8-12:
//   Flying  → small glowing pink orb with sparkle aura
//   Encased → large icy pink bubble (frozen enemy inside)
//   Rolling → oversized pulsing pink energy ball
// =========================================================

#include "Snowball.h"
#include "Constants.h"
#include "levelmanager.h"
#include "Projectile.h"
#include <cmath>

// ── Constructor ───────────────────────────────────────────

Snowball::Snowball(float x, float y, sf::Vector2f direction,
    bool powered, bool maxRange)
    : Projectile(x, y, direction, SNOWBALL_SPEED, 0)
    , m_powered(powered)
    , m_maxRange(maxRange)
{
    m_maxDistance = maxRange
        ? static_cast<float>(WINDOW_WIDTH)
        : static_cast<float>(WINDOW_WIDTH / 2);

    delete m_hitBox;
    m_hitBox = new HitBox(32.f, 32.f, 0.f, 0.f);
    m_hitBox->update(m_position);
}

// ── setEncased ────────────────────────────────────────────

void Snowball::setEncased()
{
    m_state = SnowballState::Encased;
    m_rollSize = 64.f;

    delete m_hitBox;
    m_hitBox = new HitBox(m_rollSize, m_rollSize, 0.f, 0.f);
    m_hitBox->update(m_position);

    m_direction = sf::Vector2f(0.f, 0.f);
    m_rollVelocityY = 0.f;
}

// ── update ────────────────────────────────────────────────

void Snowball::update(float dt)
{
    if (!m_active) return;

    m_glowTimer += dt;   // drives pulsing glow

    if (m_state == SnowballState::Flying)
    {
        float dx = m_direction.x * m_speed * dt;
        m_position.x += dx;
        m_distanceTravelled += fabsf(dx);

        if (m_position.x < 0.f || m_position.x > WINDOW_WIDTH)
            m_active = false;
        if (m_distanceTravelled >= m_maxDistance)
            m_active = false;
    }
    else if (m_state == SnowballState::Encased)
    {
        // stays in place
    }
    else if (m_state == SnowballState::Rolling)
    {
        m_position.x += m_direction.x * SNOWBALL_ROLL_SPEED * dt;

        m_rollVelocityY += GRAVITY * dt;
        m_position.y += m_rollVelocityY * dt;
    }

    m_hitBox->update(m_position);
}

// ── Platform collision ────────────────────────────────────

void Snowball::resolvePlatformCollision(const sf::FloatRect* platforms, int count)
{
    if (!m_active) return;

    sf::FloatRect sRect = m_hitBox->getRect();

    for (int i = 0; i < count; i++)
    {
        const sf::FloatRect& plat = platforms[i];
        if (!sRect.intersects(plat)) continue;

        float overlapTop = (sRect.top + sRect.height) - plat.top;
        float overlapBottom = (plat.top + plat.height) - sRect.top;
        float overlapLeft = (sRect.left + sRect.width) - plat.left;
        float overlapRight = (plat.left + plat.width) - sRect.left;

        float minX = (overlapLeft < overlapRight) ? overlapLeft : overlapRight;
        float minY = (overlapTop < overlapBottom) ? overlapTop : overlapBottom;

        if (m_state == SnowballState::Flying)
        {
            if (minX < minY) { m_active = false; return; }
            if (overlapTop < overlapBottom) { m_active = false; return; }
        }
        else if (m_state == SnowballState::Encased)
        {
            if (overlapTop < overlapBottom)
            {
                m_position.y = plat.top - sRect.height;
                m_rollVelocityY = 0.f;
            }
        }
        else if (m_state == SnowballState::Rolling)
        {
            if (overlapTop < overlapBottom && m_rollVelocityY >= 0.f)
            {
                m_position.y = plat.top - sRect.height;
                m_rollVelocityY = 0.f;
            }
            else if (minX < minY)
            {
                m_active = false;
                return;
            }
        }

        m_hitBox->update(m_position);
        sRect = m_hitBox->getRect();
    }
}

// ── draw ──────────────────────────────────────────────────
// All states rendered as stylised pink/magenta energy balls
// matching Blossom's visual palette (deep pink, hot pink, white core)

void Snowball::draw(sf::RenderWindow& window)
{
    if (!m_active) return;

    float size = m_hitBox->getRect().width;
    float radius = size / 2.f;

    // Centre of the hitbox on screen
    float cx = m_position.x + radius;
    float cy = m_position.y + radius;

    // Pulsing alpha for the outer glow (0.5–1.0 range)
    float pulse = 0.5f + 0.5f * sinf(m_glowTimer * 8.f);

    // ── State-specific visuals ────────────────────────────

    if (m_state == SnowballState::Flying)
    {
        // Outer glow ring — semi-transparent magenta
        float glowR = radius + 6.f + 3.f * pulse;
        sf::CircleShape glow(glowR);
        glow.setOrigin(glowR, glowR);
        glow.setPosition(cx, cy);
        glow.setFillColor(sf::Color(255, 20, 180,
            static_cast<sf::Uint8>(60 + 40 * pulse)));
        glow.setOutlineColor(sf::Color(255, 100, 220,
            static_cast<sf::Uint8>(120 * pulse)));
        glow.setOutlineThickness(2.f);
        window.draw(glow);

        // Main orb — hot pink
        sf::CircleShape ball(radius);
        ball.setOrigin(radius, radius);
        ball.setPosition(cx, cy);
        ball.setFillColor(sf::Color(240, 0, 150));
        ball.setOutlineColor(sf::Color(255, 80, 200));
        ball.setOutlineThickness(2.f);
        window.draw(ball);

        // Inner bright core
        float coreR = radius * 0.45f;
        sf::CircleShape core(coreR);
        core.setOrigin(coreR, coreR);
        core.setPosition(cx - radius * 0.15f, cy - radius * 0.15f);
        core.setFillColor(sf::Color(255, 200, 240,
            static_cast<sf::Uint8>(200 + 55 * pulse)));
        window.draw(core);

        // Powered — add a second outer ring
        if (m_powered)
        {
            float pr = radius + 12.f;
            sf::CircleShape ring(pr);
            ring.setOrigin(pr, pr);
            ring.setPosition(cx, cy);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineColor(sf::Color(255, 60, 200,
                static_cast<sf::Uint8>(150 * pulse)));
            ring.setOutlineThickness(2.5f);
            window.draw(ring);
        }
    }
    else if (m_state == SnowballState::Encased)
    {
        // Icy bubble containing enemy — pale pink / light blue tint
        // Outer icy shell
        sf::CircleShape shell(radius);
        shell.setOrigin(radius, radius);
        shell.setPosition(cx, cy);
        shell.setFillColor(sf::Color(210, 80, 200,
            static_cast<sf::Uint8>(120 + 30 * pulse)));
        shell.setOutlineColor(sf::Color(255, 180, 255));
        shell.setOutlineThickness(3.f);
        window.draw(shell);

        // Sparkle highlights (4 small dots)
        const float offsets[4][2] = { {-radius * 0.5f,-radius * 0.5f},
                                     { radius * 0.5f,-radius * 0.4f},
                                     {-radius * 0.3f, radius * 0.5f},
                                     { radius * 0.4f, radius * 0.4f} };
        for (int i = 0; i < 4; i++)
        {
            float sparkleR = 3.f + 2.f * pulse;
            sf::CircleShape sp(sparkleR);
            sp.setOrigin(sparkleR, sparkleR);
            sp.setPosition(cx + offsets[i][0], cy + offsets[i][1]);
            sp.setFillColor(sf::Color(255, 240, 255,
                static_cast<sf::Uint8>(180 * pulse)));
            window.draw(sp);
        }
    }
    else if (m_state == SnowballState::Rolling)
    {
        // Rolling energy ball — large, intense, pulsing
        // Outer energy field
        float fieldR = radius + 8.f + 6.f * pulse;
        sf::CircleShape field(fieldR);
        field.setOrigin(fieldR, fieldR);
        field.setPosition(cx, cy);
        field.setFillColor(sf::Color(255, 0, 200,
            static_cast<sf::Uint8>(40 + 30 * pulse)));
        field.setOutlineColor(sf::Color(255, 100, 230,
            static_cast<sf::Uint8>(160 * pulse)));
        field.setOutlineThickness(3.f);
        window.draw(field);

        // Main rolling ball
        sf::CircleShape ball(radius);
        ball.setOrigin(radius, radius);
        ball.setPosition(cx, cy);
        ball.setFillColor(sf::Color(220, 0, 130));
        ball.setOutlineColor(sf::Color(255, 60, 200));
        ball.setOutlineThickness(3.f);
        window.draw(ball);

        // Bright spinning core (offset to give spin illusion)
        float spinOffset = radius * 0.3f * sinf(m_glowTimer * 12.f);
        float coreR = radius * 0.5f;
        sf::CircleShape core(coreR);
        core.setOrigin(coreR, coreR);
        core.setPosition(cx + spinOffset, cy - spinOffset);
        core.setFillColor(sf::Color(255, 180, 255));
        window.draw(core);

        // Chain-kill size bonus indicator (extra ring per kill)
        for (int k = 0; k < m_chainCount && k < 4; k++)
        {
            float ringR = radius + 14.f + k * 8.f;
            sf::CircleShape ring(ringR);
            ring.setOrigin(ringR, ringR);
            ring.setPosition(cx, cy);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineColor(sf::Color(255, 150, 255,
                static_cast<sf::Uint8>(80 - k * 15)));
            ring.setOutlineThickness(1.5f);
            window.draw(ring);
        }
    }
}

// ── onHit ─────────────────────────────────────────────────

void Snowball::onHit()
{
    if (m_state == SnowballState::Flying)
        m_active = false;
}

// ── startRolling ─────────────────────────────────────────

void Snowball::startRolling(sf::Vector2f direction)
{
    m_state = SnowballState::Rolling;
    m_direction = direction;
    m_rollSize = 64.f;

    delete m_hitBox;
    m_hitBox = new HitBox(m_rollSize, m_rollSize, 0.f, 0.f);
    m_hitBox->update(m_position);

    m_rollVelocityY = 0.f;
}

// ── addChainKill ──────────────────────────────────────────

void Snowball::addChainKill()
{
    m_chainCount++;
    m_rollSize += 8.f;
    delete m_hitBox;
    m_hitBox = new HitBox(m_rollSize, m_rollSize, 0.f, 0.f);
    m_hitBox->update(m_position);
}