

#include "MogeraChild.h"
#include "Constants.h"
#include <cstdlib>
#include <cmath>
static constexpr int WALK_X[4] = { 1797, 1969, 2140, 2318 };
static constexpr int WALK_Y = 800;
static constexpr int WALK_W[4] = { 166,  166,  173,  152 };  // non-uniform widths
static constexpr int WALK_H = 190;

static constexpr int BALL_X[3] = { 1780, 1989, 2183 };
static constexpr int BALL_Y = 990;
static constexpr int BALL_W[3] = { 153,  131,  117 };        // non-uniform widths
static constexpr int BALL_H = 150;
// ─── Constructor ──────────────────────────────────────────

MogeraChild::MogeraChild(float x, float y, float dirX, sf::Texture* sharedTexture)
    : Enemy(x, y, EnemyVariant::Red)
    , m_dirX(dirX)
    , m_sharedTexture(sharedTexture)
{
    m_speed = 90.f;
    m_health = 1;
    m_scoreValue = 200;

    m_hasTexture = (m_sharedTexture != nullptr);
    if (m_hasTexture)
    {
        m_childSprite.setTexture(*m_sharedTexture);
    }

    // ── Clip definitions ──────────────────────────────────
    // SpawnPop: play walk frames 0,1 once as a birth-pop
    m_clips[static_cast<int>(ChildState::SpawnPop)] = {
     { 0, 1, 2, 3 }, 4, 0.07f, false
    };
    m_clips[static_cast<int>(ChildState::Walk)] = {
        { 0, 1, 2, 3 }, 4, 0.14f, true
    };
    m_clips[static_cast<int>(ChildState::Frozen)] = {
        { 0, 1, 2 }, 3, 0.35f, true
    };

    m_hitBox = new HitBox(30.f, 40.f, 0.f, 0.f);
    m_hitBox->update(m_position);

    transitionTo(ChildState::SpawnPop);

    // Birth-bounce
    m_velocity.y = -420.f;
    m_velocity.x = m_dirX * 200;
    m_facingRight = (m_dirX > 0.f);
}

// ─── State transitions ────────────────────────────────────

void MogeraChild::transitionTo(ChildState next)
{
    m_state = next;
    m_stateTimer = 0.f;
    m_clipFrame = 0;
    m_frameTimer = 0.f;
}

// ─── Animation ────────────────────────────────────────────

void MogeraChild::advanceAnim(float dt)
{
    const AnimClip& clip = m_clips[static_cast<int>(m_state)];
    m_frameTimer += dt;
    if (m_frameTimer >= clip.frameDur)
    {
        m_frameTimer = 0.f;
        m_clipFrame++;
        if (m_clipFrame >= clip.frameCount)
            m_clipFrame = clip.loop ? 0 : clip.frameCount - 1;
    }
}

sf::IntRect MogeraChild::currentTexRect() const
{
    const AnimClip& clip = m_clips[static_cast<int>(m_state)];
    int col = clip.frames[m_clipFrame];

    if (m_state == ChildState::Frozen)
        return sf::IntRect(BALL_X[col], BALL_Y, BALL_W[col], BALL_H);
    else
        return sf::IntRect(WALK_X[col], WALK_Y, WALK_W[col], WALK_H);
}

// ─── update ──────────────────────────────────────────────


void MogeraChild::update(float dt)
{
    if (!m_active) return;

    m_stateTimer += dt;
    if (m_hitFlashTimer > 0.f) m_hitFlashTimer -= dt;   // ← add this, it was missing

    if (m_state == ChildState::SpawnPop)
    {
        const AnimClip& clip = m_clips[static_cast<int>(ChildState::SpawnPop)];
        float popDur = clip.frameDur * clip.frameCount;
        if (m_stateTimer >= popDur)
            transitionTo(ChildState::Walk);
    }

    if (m_state == ChildState::Frozen)
    {
        if (m_stateTimer >= 5.f)
        {
            m_snowState = SnowState::None;
            m_snowHits = 0;
            m_velocity.x = m_speed * (m_facingRight ? 1.f : -1.f);
            transitionTo(ChildState::Walk);
        }
        m_hitBox->update(m_position);
        advanceAnim(dt);
        return;
    }

    // Gravity
    if (!m_onGround)
    {
        m_velocity.y += GRAVITY * dt;
        if (m_velocity.y > 1200.f) m_velocity.y = 1200.f;
    }
    else
    {
        m_velocity.y = 0.f;
    }

    // ── KEY FIX: only override horizontal velocity once grounded ──
    // During SpawnPop while airborne, preserve the launch X velocity.
    // Once Walk state begins OR already on ground, use patrol speed.
    if (m_state == ChildState::Walk || m_onGround)
    {
        m_velocity.x = m_speed * (m_facingRight ? 1.f : -1.f);
    }
    // else: SpawnPop while airborne — keep the launch velocity as-is

    m_position.x += m_velocity.x * dt;
    m_position.y += m_velocity.y * dt;

    // Screen bounds
    if (m_position.x < 0.f)
    {
        m_position.x = 0.f;
        m_facingRight = true;
    }
    if (m_position.x > WINDOW_WIDTH - 30.f)
    {
        m_position.x = static_cast<float>(WINDOW_WIDTH - 30);
        m_facingRight = false;
    }
    if (m_position.y >= WINDOW_HEIGHT - 30.f)
    {
        m_position.y = static_cast<float>(WINDOW_HEIGHT - 30);
        m_velocity.y = 0.f;
        m_onGround = true;
    }
    if (m_position.y < 0.f)
    {
        m_position.y = 0.f;
        m_velocity.y = 0.f;
    }

    m_hitBox->update(m_position);
    advanceAnim(dt);
}
// ─── Platform collision (identical logic to Botom) ────────

void MogeraChild::resolvePlatformCollision(const sf::FloatRect* rects, int count)
{
    m_onGround = false;

    for (int i = 0; i < count; i++)
    {
        m_hitBox->update(m_position);
        sf::FloatRect eRect = m_hitBox->getRect();
        const sf::FloatRect& plat = rects[i];

        if (!eRect.intersects(plat)) continue;

        float overlapTop = (eRect.top + eRect.height) - plat.top;
        float overlapBottom = (plat.top + plat.height) - eRect.top;
        float overlapLeft = (eRect.left + eRect.width) - plat.left;
        float overlapRight = (plat.left + plat.width) - eRect.left;

        float minX = (overlapLeft < overlapRight) ? overlapLeft : overlapRight;
        float minY = (overlapTop < overlapBottom) ? overlapTop : overlapBottom;

        if (minY < minX && overlapTop < overlapBottom && m_velocity.y > -200.f)
        {
            m_position.y -= overlapTop;
            m_velocity.y = 0.f;
            m_onGround = true;
            m_hitBox->update(m_position);
        }
        else if (minX < minY)
        {
            if (overlapLeft < overlapRight) m_position.x -= overlapLeft;
            else                            m_position.x += overlapRight;

            m_facingRight = !m_facingRight;
            m_velocity.x = m_speed * (m_facingRight ? 1.f : -1.f);
            m_hitBox->update(m_position);
        }
    }

    // Ground probe — prevents vibration when sitting flush on a platform
    if (!m_onGround && m_velocity.y >= 0.f)
    {
        sf::FloatRect probe = m_hitBox->getRect();
        probe.top += probe.height;
        probe.height = 2.f;

        for (int i = 0; i < count; i++)
        {
            if (probe.intersects(rects[i]))
            {
                m_onGround = true;
                m_velocity.y = 0.f;
                break;
            }
        }
    }
}

// ─── tryJump  (call AFTER resolvePlatformCollision) ───────

void MogeraChild::tryJump()
{
    if (!m_active) return;
    if (m_state == ChildState::Frozen) return;
    if (m_onGround && (rand() % 90 == 0))
        m_velocity.y = -550.f;
}

// ─── draw ────────────────────────────────────────────────

void MogeraChild::draw(sf::RenderWindow& window)
{
    if (!m_active) return;

    if (m_hasTexture)
    {
        m_childSprite.setTextureRect(currentTexRect());


        const float scale = (m_state == ChildState::Frozen) ? 0.27f : 0.21f;
        float sx = m_facingRight ? -scale : +scale;
        m_childSprite.setScale(sx, scale);

        sf::IntRect tr = currentTexRect();
        m_childSprite.setOrigin(tr.width / 2.f, 0.f);
        m_childSprite.setPosition(m_position.x + 15.f, m_position.y);

        if (m_hitFlashTimer > 0.f)
            m_childSprite.setColor(sf::Color(255, 80, 80, 255));
        else
            m_childSprite.setColor(sf::Color::White);

        window.draw(m_childSprite);
    }
    else
    {
        sf::RectangleShape shape(sf::Vector2f(30.f, 30.f));
        shape.setPosition(m_position);
        shape.setFillColor(sf::Color(180, 40, 40));
        window.draw(shape);
    }

    if (m_snowState == SnowState::Partial)
    {
        sf::RectangleShape snow(sf::Vector2f(30.f, 12.f));
        snow.setPosition(m_position);
        snow.setFillColor(sf::Color(255, 255, 255, 180));
        window.draw(snow);
    }
}

// ─── drawDebug ────────────────────────────────────────────

void MogeraChild::drawDebug(sf::RenderWindow& window)
{
    if (m_hitBox) m_hitBox->drawDebug(window, sf::Color(255, 128, 0));
}

// ─── takeHit ─────────────────────────────────────────────

bool MogeraChild::takeHit()
{
    if (m_state == ChildState::Frozen) return false;

    m_snowHits++;
    m_hitFlashTimer = 0.15f;   // assumes parent calls update() each frame

    if (m_snowHits >= 2)
    {
        m_snowState = SnowState::Full;
        m_velocity = sf::Vector2f(0.f, 0.f);
        transitionTo(ChildState::Frozen);
        return true;   // fully encased → parent can chain-kill
    }

    m_snowState = SnowState::Partial;
    m_velocity.x *= 0.4f;
    return false;
}

// ─── onChainKill ─────────────────────────────────────────

void MogeraChild::onChainKill()
{
    m_active = false;
    m_hasDrop = true;
}