#include <iostream>
#include "Platform.h"
#include "Player.h"
#include "Constants.h"
#include <cmath>
#include <cstring>

// ─── CharacterSheet 
struct CharacterSheet
{
    const char* characterName;
    const char* texturePath;

    int frameW;    //  per frame horizontally
    int frameH;    //  per frame vertically
    int footY;     // pixel row inside frame where feet sit 

    // Row indices
    int rowIdle;
    int rowWalk;
    int rowFly;
    int rowDash;
    int rowThrow;
    int rowHit;
    int rowJump;
    int rowBalloon;

    // Frame counts 
    int fcIdle, fcWalk, fcFly, fcDash, fcThrow, fcHit, fcJump, fcBalloon;

    // Seconds per 
    float frIdle, frWalk, frFly, frDash, frThrow, frHit, frJump, frBalloon;
};


static constexpr CharacterSheet SHEET_BLOSSOM =
{
     "BLOSSOM",
     "assets/images/BLOSSOMFINAL.png",

     /* frameW frameH footY */
        47,    64,    56,

        // rowIdle  rowWalk  rowFly  rowDash  rowThrow  rowHit  rowJump  rowBalloon
           0,       1,       2,      2,       3,        4,      6,       5,

           // fcIdle  fcWalk  fcFly  fcDash  fcThrow  fcHit  fcJump  fcBalloon
              8,      8,      8,     8,      8,       8,     6,      7,

              // frIdle   frWalk   frFly    frDash   frThrow  frHit    frJump   frBalloon
                 0.13f,   0.09f,   0.11f,   0.07f,   0.06f,   0.08f,   0.11f,   0.14f
};

static constexpr CharacterSheet SHEET_BUBBLES =
{
     "BUBBLE",
   "assets/images/BUBBLEFINAL5.png",

   /* frameW frameH footY */
      47,    64,    56,

      // rowIdle  rowWalk  rowFly  rowDash  rowThrow  rowHit  rowJump  rowBalloon
         0,       1,       2,      2,       3,        4,      6,       5,

         // fcIdle  fcWalk  fcFly  fcDash  fcThrow  fcHit  fcJump  fcBalloon
            8,      8,      8,     8,      8,       8,     6,      7,

            // frIdle   frWalk   frFly    frDash   frThrow  frHit    frJump   frBalloon
               0.15f,   0.10f,   0.12f,   0.08f,   0.07f,   0.09f,   0.12f,   0.15f
};
static constexpr CharacterSheet SHEET_BUTTERCUP =
{
    /* name   */ "BUTTERCUP",
    /* path   */ "assets/images/BUTTERCUP.png",

    /* frameW frameH footY */
       47,    64,    56,

       // rowIdle  rowWalk  rowFly  rowDash  rowThrow  rowHit  rowJump  rowBalloon
          0,       1,       2,      2,       3,        4,      6,       5,

          // fcIdle  fcWalk  fcFly  fcDash  fcThrow  fcHit  fcJump  fcBalloon
             8,      8,      8,     8,      8,       8,     6,      7,

             // frIdle   frWalk   frFly    frDash   frThrow  frHit    frJump   frBalloon
                0.14f,   0.08f,   0.10f,   0.06f,   0.06f,   0.07f,   0.10f,   0.13f
};



static const CharacterSheet* CHARACTER_SHEET_REGISTRY[] =
{
    &SHEET_BLOSSOM,      // index 0
    &SHEET_BUTTERCUP,    // index 1
    &SHEET_BUBBLES,      // index 2
    
    nullptr
};

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers
// ─────────────────────────────────────────────────────────────────────────────

static const CharacterSheet* findSheet(const char* name)
{
    if (!name) return nullptr;
    for (int i = 0; CHARACTER_SHEET_REGISTRY[i] != nullptr; i++)
        if (strcmp(CHARACTER_SHEET_REGISTRY[i]->characterName, name) == 0)
            return CHARACTER_SHEET_REGISTRY[i];
    return nullptr;
}

static sf::IntRect frameRect(const CharacterSheet& cs, int col, int row)
{
    return sf::IntRect(
        col * cs.frameW,
        row * cs.frameH,
        cs.frameW,
        cs.frameH
    );
}

//  Constructors

Player::Player(float x, float y, int playerIndex, const char* name)
    : m_playerIndex(playerIndex), m_name(name)
{
    m_position = sf::Vector2f(x, y);
    m_velocity = sf::Vector2f(0.f, 0.f);
    m_onGround = false;
    m_lives = PLAYER_START_LIVES;
    m_bodyColor = (playerIndex == 0)
        ? sf::Color(50, 100, 255)
        : sf::Color(255, 100, 50);
    m_accentColor = sf::Color::White;
    m_hitBox = new HitBox(56.f, 80.f, 4.f, 3.f);
    m_hitBox->update(m_position);
    initKeyBindings();
}

Player::Player(float x, float y, int playerIndex, const CharacterData* cd)
    : m_playerIndex(playerIndex), m_name(cd ? cd->name : "?")
{
    m_position = sf::Vector2f(x, y);
    m_velocity = sf::Vector2f(0.f, 0.f);
    m_onGround = false;
    m_hitBox = new HitBox(56.f, 80.f, 4.f, 3.f);
    m_hitBox->update(m_position);
    initKeyBindings();

    if (cd)
    {
        applyCharacterData(*cd);
        m_pSheet = findSheet(cd->name);

        if (m_pSheet)
        {
            m_texture.setSmooth(false);
            if (m_texture.loadFromFile(m_pSheet->texturePath))
            {
                m_sprite.setTexture(m_texture);
                m_sprite.setTextureRect(
                    frameRect(*m_pSheet, 0, m_pSheet->rowIdle));
                m_sprite.setOrigin(0.f, 0.f);
                m_hasTexture = true;
            }
        }
    }
}

//  Character data

void Player::applyCharacterData(const CharacterData& cd)
{
    m_speedMult = cd.speedMult;
    m_throwRate = cd.throwRate;
    m_bodyColor = cd.bodyColor;
    m_accentColor = cd.accentColor;
    m_lives = cd.startLives;

    m_nativeMaxRange = false;
    for (int i = 0; cd.name[i]; i++)
        if (cd.name[i] == 'M' && cd.name[i + 1] == 'i' && cd.name[i + 2] == 'r')
        {
            m_nativeMaxRange = true;
            break;
        }

    if (m_nativeMaxRange) m_maxRange = true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Key bindings
// ─────────────────────────────────────────────────────────────────────────────

void Player::initKeyBindings()
{
    if (m_playerIndex == 0)
    {
        m_keyLeft = sf::Keyboard::A;
        m_keyRight = sf::Keyboard::D;
        m_keyJump = sf::Keyboard::W;
        m_keyThrow = sf::Keyboard::Space;
    }
    else
    {
        m_keyLeft = sf::Keyboard::Numpad4;
        m_keyRight = sf::Keyboard::Numpad6;
        m_keyJump = sf::Keyboard::Numpad8;
        m_keyThrow = sf::Keyboard::Numpad0;
    }
}



void Player::updateAnimation(float dt)
{
    if (!m_pSheet) return;
    const CharacterSheet& cs = *m_pSheet;

    m_animTimer += dt;

    // Balloon pulse for visual effect
    if (m_balloonTimer > 0.f)
        m_balloonPulse += dt * 3.5f;

    // Fat scale lerp
    float targetFat = (m_balloonTimer > 0.f) ? 1.45f : 1.0f;
    float lerpSpeed = (m_balloonTimer > 0.f) ? 4.0f : 6.0f;
    m_fatScale += (targetFat - m_fatScale) * lerpSpeed * dt;
    if (m_fatScale < 1.00f) m_fatScale = 1.00f;
    if (m_fatScale > 1.45f) m_fatScale = 1.45f;

    // ── Determine new state ───────────────────────────────
    AnimState newState = ANIM_IDLE;
    int       sheetRow = cs.rowIdle;
    int       frameCount = cs.fcIdle;
    float     frameDur = cs.frIdle;

    if (m_throwAnimTimer > 0.f)
    {
        // THROW — plays full 8-frame sequence during throw window
        newState = ANIM_THROW;
        sheetRow = cs.rowThrow;
        frameCount = cs.fcThrow;   // 8 frames
        frameDur = cs.frThrow;
        m_throwAnimTimer -= dt;
    }
    else if (m_hitAnimTimer > 0.f)
    {
        // HIT — damage flash sequence
        newState = ANIM_HIT;
        sheetRow = cs.rowHit;
        frameCount = cs.fcHit;     // 8 frames
        frameDur = cs.frHit;
        m_hitAnimTimer -= dt;
    }
    else if (m_balloonTimer > 0.f)
    {
        // BALLOON / SPIN hover — 7 frames 
        newState = ANIM_BALLOON;
        sheetRow = cs.rowBalloon;
        frameCount = cs.fcBalloon; // 7
        frameDur = cs.frBalloon;
    }
    else if (!m_onGround)
    {
        if (m_velocity.y < 0.f)
        {
            // Rising — JUMP row, 6 frames (col 0-5)
            newState = ANIM_JUMP;
            sheetRow = cs.rowJump;
            frameCount = cs.fcJump;   // 6
            frameDur = cs.frJump;
        }
        else
        {
            // Falling — FLY row, 8 frames
            newState = ANIM_FLY;
            sheetRow = cs.rowFly;
            frameCount = cs.fcFly;    // 8
            frameDur = cs.frFly;
        }
    }
    else if (fabsf(m_velocity.x) > 200.f && cs.rowDash >= 0)
    {
        // DASH — shares FLY row (row 2) at speed burst
        newState = ANIM_DASH;
        sheetRow = cs.rowDash;
        frameCount = cs.fcDash;       // 8
        frameDur = cs.frDash;
    }
    else if (fabsf(m_velocity.x) > 10.f)
    {
        // WALK — row 1, 8 frames
        newState = ANIM_WALK;
        sheetRow = cs.rowWalk;
        frameCount = cs.fcWalk;       // 8
        frameDur = cs.frWalk;
    }

    // ── State change: reset frame counter ────────────────
    if (newState != m_animState)
    {
        m_animState = newState;
        m_animRow = sheetRow;
        m_animFrame = 0;
        m_animTimer = 0.f;
    }

    if (m_animTimer >= frameDur)
    {
        m_animTimer = 0.f;
        m_animFrame = (m_animFrame + 1) % frameCount;
    }
}



void Player::handleInput(float dt)
{
    if (!m_active) return;

    float baseSpeed = PLAYER_SPEED * m_speedMult;
    if (m_speedBoostTimer > 0.f) baseSpeed *= POWERUP_SPEED_MULTIPLIER;
    if (m_balloonTimer > 0.f) baseSpeed *= 0.6f;

    float accel = m_onGround ? 2000.f : 800.f;
    float friction = 1500.f;

    bool pressingLeft = sf::Keyboard::isKeyPressed(m_keyLeft);
    bool pressingRight = sf::Keyboard::isKeyPressed(m_keyRight);

    if (pressingLeft)
    {
        m_velocity.x -= accel * dt;
        m_facingRight = false;
    }
    else if (pressingRight)
    {
        m_velocity.x += accel * dt;
        m_facingRight = true;
    }
    else
    {
        // Friction
        if (m_velocity.x > 0.f)
        {
            m_velocity.x -= friction * dt;
            if (m_velocity.x < 0.f) m_velocity.x = 0.f;
        }
        else if (m_velocity.x < 0.f)
        {
            m_velocity.x += friction * dt;
            if (m_velocity.x > 0.f) m_velocity.x = 0.f;
        }
    }

    if (m_velocity.x > baseSpeed) m_velocity.x = baseSpeed;
    if (m_velocity.x < -baseSpeed) m_velocity.x = -baseSpeed;

    // Jump
    if (sf::Keyboard::isKeyPressed(m_keyJump) && m_onGround)
    {
        m_velocity.y = JUMP_FORCE * 1.6f;
        m_onGround = false;
    }

    // Throw — edge-triggered (one throw per key press)
    bool isPressed = sf::Keyboard::isKeyPressed(m_keyThrow);
    if (isPressed && !m_throwKeyHeld && m_throwCooldown <= 0.f)
    {
        m_throwPressed = true;
        m_throwCooldown = 0.5f / m_throwRate;
        m_throwAnimTimer = 0.40f;   // play full throw animation for 0.40 s
    }
    m_throwKeyHeld = isPressed;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Update
// ─────────────────────────────────────────────────────────────────────────────

void Player::update(float dt)
{
    m_prevY = m_position.y;
    if (!m_active) return;

    handleInput(dt);

    if (m_balloonTimer > 0.f)
    {
        // Balloon hover: gentle sine bob
        float bob = sinf(m_balloonPulse) * 15.f;
        m_velocity.y = -80.f + bob;
    }
    else
    {
        m_velocity.y += GRAVITY * dt;
    }

    if (m_velocity.y > 800.f) m_velocity.y = 800.f;

    m_position.x += m_velocity.x * dt;
    m_position.y += m_velocity.y * dt;

    // Screen bounds
    if (m_position.x < 0.f)
        m_position.x = 0.f;
    if (m_position.x > WINDOW_WIDTH - 32)
        m_position.x = static_cast<float>(WINDOW_WIDTH - 32);

    m_onGround = false;
    if (m_position.y > WINDOW_HEIGHT)
    {
        m_position.y = static_cast<float>(WINDOW_HEIGHT);
        m_velocity.y = 0.f;
    }
    if (m_position.y < 0.f)
    {
        m_position.y = 0.f;
        m_velocity.y = 0.f;
    }

    if (m_invincibleTimer > 0.f) m_invincibleTimer -= dt;
    if (m_throwCooldown > 0.f) m_throwCooldown -= dt;

    updateAnimation(dt);
    updatePowerUps(dt);
    m_hitBox->update(m_position);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Platform collision
// ─────────────────────────────────────────────────────────────────────────────

void Player::resolvePlatformCollision(const Platform* platforms, int count)
{
    if (m_balloonTimer > 0.f) return;

    sf::FloatRect pRect = m_hitBox->getRect();
    for (int i = 0; i < count; i++)
    {
        const sf::FloatRect& plat = platforms[i].getRect();
        if (!pRect.intersects(plat)) continue;

        if (m_velocity.y > 0)
        {
            float playerBottom = pRect.top + pRect.height;
            if (playerBottom - m_velocity.y <= plat.top)
            {
                m_position.y = plat.top - pRect.height;
                m_velocity.y = 0.f;
                m_onGround = true;
                m_hitBox->update(m_position);
                return;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Draw
// ─────────────────────────────────────────────────────────────────────────────

void Player::draw(sf::RenderWindow& window)
{
    if (!m_active) return;

    if (m_invincibleTimer > 0.f &&
        static_cast<int>(m_invincibleTimer * 10) % 2 == 1)
        return;

    if (m_hasTexture && m_pSheet)
    {
        const CharacterSheet& cs = *m_pSheet;


        constexpr float HITBOX_W = 56.f;
        constexpr float HITBOX_H = 80.f;

        float scaleX = (DRAW_WIDTH / static_cast<float>(cs.frameW)) * m_fatScale * 1.15f;
        float scaleY = (DRAW_HEIGHT / static_cast<float>(cs.frameH)) * m_fatScale * 1.15f;

        // ── Set frame ────────────────────────────────────
        m_sprite.setTextureRect(frameRect(cs, m_animFrame, m_animRow));


        m_sprite.setOrigin(
            static_cast<float>(cs.frameW) * 0.5f,
            static_cast<float>(cs.footY)
        );

        float feetX = m_position.x + HITBOX_W * 0.5f;
        float feetY = m_position.y + HITBOX_H;

        m_sprite.setScale(m_facingRight ? scaleX : -scaleX, scaleY);
        m_sprite.setPosition(feetX, feetY);

        // ── Tint / colour ─────────────────────────────────
        if (m_balloonTimer > 0.f)
        {
            // Soft pink-blue pulse during balloon mode
            float     pulse = (sinf(m_balloonPulse * 2.f) + 1.f) * 0.5f;
            sf::Uint8 g = static_cast<sf::Uint8>(200 + static_cast<int>(30.f * pulse));
            sf::Uint8 b = static_cast<sf::Uint8>(210 + static_cast<int>(45.f * pulse));
            m_sprite.setColor(sf::Color(255, g, b, 230));
        }
        else if (m_hitAnimTimer > 0.f &&
            static_cast<int>(m_hitAnimTimer * 20) % 2 == 0)
        {
            // Red flash on hit
            m_sprite.setColor(sf::Color(255, 80, 80, 255));
        }
        else
        {
            m_sprite.setColor(sf::Color(255, 255, 255, 255));
        }

        window.draw(m_sprite);

        if (m_balloonTimer > 0.f)
        {
            float bobY = sinf(m_balloonPulse) * 4.f;
            float cx = m_position.x + HITBOX_W * 0.5f;
            for (int sp = 0; sp < 3; sp++)
            {
                float r = 5.f - static_cast<float>(sp);
                sf::CircleShape sparkle(r);
                sparkle.setFillColor(sf::Color(255, 210, 230,
                    static_cast<sf::Uint8>(130 - sp * 35)));
                sparkle.setOrigin(r, r);
                sparkle.setPosition(
                    cx + (sp - 1) * 10.f,
                    m_position.y - 4.f + bobY - sp * 5.f
                );
                window.draw(sparkle);
            }
        }
    }
    else
    {
        float w = 56.f * m_fatScale;
        float h = 80.f * m_fatScale;
        sf::RectangleShape shape(sf::Vector2f(w, h));
        shape.setPosition(m_position);
        shape.setFillColor(m_balloonTimer > 0.f
            ? sf::Color(255, 180, 210) : m_bodyColor);
        shape.setOutlineColor(m_accentColor);
        shape.setOutlineThickness(2.f);
        window.draw(shape);
    }
}

void Player::drawDebug(sf::RenderWindow& window)
{
    if (m_hitBox) m_hitBox->drawDebug(window, sf::Color::Green);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Damage / lives
// ─────────────────────────────────────────────────────────────────────────────

void Player::takeDamage()
{
    if (m_invincibleTimer > 0.f) return;
    if (m_balloonTimer > 0.f) return;   // balloon absorbs hit

    m_lives--;
    m_invincibleTimer = PLAYER_INVINCIBLE_TIME;
    m_hitAnimTimer = 0.32f;

    if (m_lives <= 0) m_active = false;
}

//  Throw

Snowball* Player::tryThrow()
{
    if (!m_throwPressed) return nullptr;
    m_throwPressed = false;

    sf::Vector2f dir = m_facingRight
        ? sf::Vector2f(1.f, 0.f)
        : sf::Vector2f(-1.f, 0.f);

    float centerX = m_position.x + 48.f;
    float centerY = m_position.y + 40.f;
    float spawnX = centerX + (m_facingRight ? 50.f : -50.f);

    return new Snowball(spawnX, centerY, dir, m_snowballPowered, m_maxRange);
}

//  Power-ups

void Player::applySpeedBoost(float duration)
{
    m_speedBoostTimer = (duration > 0.f) ? duration : POWERUP_SPEED_DURATION;
}

void Player::applySnowballPower() { m_snowballPowered = true; }
void Player::applyDistanceIncrease() { m_maxRange = true; }

void Player::applyBalloonMode(float duration)
{
    m_balloonTimer = (duration > 0.f) ? duration : POWERUP_BALLOON_DURATION;
    m_balloonPulse = 0.f;
}

void Player::addLife() { m_lives++; }

void Player::resetLevelPowerUps()
{
    m_snowballPowered = false;
    m_maxRange = m_nativeMaxRange;
    m_speedBoostTimer = 0.f;
    m_balloonTimer = 0.f;
    m_fatScale = 1.0f;
    m_balloonPulse = 0.f;
}

void Player::updatePowerUps(float dt)
{
    if (m_speedBoostTimer > 0.f) m_speedBoostTimer -= dt;
    if (m_balloonTimer > 0.f) m_balloonTimer -= dt;
}

bool Player::spendGems(int cost)
{
    if (m_gems < cost) return false;
    m_gems -= cost;
    return true;
}

void Player::resetForNewLevel()
{
    m_lives = PLAYER_START_LIVES;
    m_velocity = sf::Vector2f(0.f, 0.f);
    m_onGround = false;
    m_invincibleTimer = 0.f;
    m_throwCooldown = 0.f;
    m_animState = ANIM_IDLE;
    m_animRow = m_pSheet ? m_pSheet->rowIdle : 0;
    m_animFrame = 0;
    m_animTimer = 0.f;
    m_fatScale = 1.0f;
    m_balloonPulse = 0.f;
    resetLevelPowerUps();
    m_active = true;
}