#include "Gamakichi.h"
#include "Player.h"
#include "Constants.h"
#include <cstdlib>
#include <cmath>



static const sf::IntRect HEAD_IDLE(133, 9, 1081, 620);
static const sf::IntRect HEAD_ATTACK(1445, 18, 1081, 621);

static const float TUBE_OFFSETS_X[4] = { 58.f, 210.f, 845.f, 990.f };
static const float TUBE_TOP_Y_OFFSET = -50.f;

// ── Muzzle flash  
static const int FLASH_COUNT = 2;
static const sf::IntRect FLASH_FRAMES[FLASH_COUNT] = {
    { 64, 620, 167, 318},
    {231, 620, 169, 327},
};


static const int EYE_COUNT = 6;
static const sf::IntRect EYE_FRAMES[EYE_COUNT] = {
    { 904, 731, 159, 313},   // 0 – fully open
    {1232, 731, 159, 313},   // 1 – slightly open
    {1560, 731, 159, 313},   // 2 – half closed
    {1888, 731, 159, 313},   // 3 – squinting
    {2216, 731, 159, 313},   // 4 – almost shut
    {2544, 731, 159, 313},   // 5 – shut
};

// ── Smoke  (4 frames) 
static const int SMOKE_COUNT = 4;
static const sf::IntRect SMOKE_FRAMES[SMOKE_COUNT] = {
    {2544, 731, 158, 313},
    {2756, 712, 313, 186},
    {3084, 683, 312, 215},
    {3412, 630, 312, 268},
};

// ── Artillery projectile  (10 fly frames)
static const int PROJ_FLY_COUNT = 10;
static const sf::IntRect PROJ_FLY_FRAMES[PROJ_FLY_COUNT] = {
    {  48, 1145, 253, 314},
    { 347, 1215, 290, 243},
    { 615, 1145, 265, 313},
    { 914, 1206, 290, 254},
    {1156, 1244, 290, 215},
    {1407, 1184, 290, 276},
    {1678, 1253, 290, 206},
    {1917, 1184, 266, 275},
    {2207, 1244, 290, 216},
    {2901, 1174, 266, 276},
};

// ── Draw sizes ────────────────────────────────────────────
static const float HEAD_DRAW_W = 230.f;
static const float PROJ_DRAW_W = 48.f;
static const float FLASH_DRAW_W = 42.f;
static const float EYE_DRAW_W = 30.f;   // rendered size of each eye


// =========================================================

void ArtilleryShell::update(float dt)
{
    if (!active) return;

    if (!exploded)
    {
        flyTimer += dt;
        if (flyTimer >= 0.050f)
        {
            flyTimer = 0.f;
            flyFrame = (flyFrame + 1) % PROJ_FLY_COUNT;
        }

        vel.y += 580.f * dt;
        pos += vel * dt;

        if (pos.y >= WINDOW_HEIGHT - 28.f)
        {
            pos.y = WINDOW_HEIGHT - 28.f;
            exploded = true;
            explTimer = 0.35f;
        }

        if (pos.x < -80.f || pos.x > WINDOW_WIDTH + 80.f)
            active = false;

        return;
    }

    explTimer -= dt;
    if (explTimer <= 0.f) active = false;
}

void ArtilleryShell::draw(sf::RenderWindow& window,
    const sf::Texture& tex) const
{
    if (!active || exploded) return;

    const sf::IntRect& fr = PROJ_FLY_FRAMES[flyFrame];
    sf::Sprite s(tex);
    s.setTextureRect(fr);
    float sc = PROJ_DRAW_W / static_cast<float>(fr.width);
    s.setScale(sc, sc);
    s.setOrigin(fr.width * 0.5f, fr.height * 0.5f);
    s.setPosition(pos);
    window.draw(s);
}

//  Snow
void Gamakichi::initSnow()
{
    for (int i = 0; i < MAX_SNOW; ++i)
    {
        m_snow[i].pos.x = static_cast<float>(std::rand() % WINDOW_WIDTH);
        m_snow[i].pos.y = static_cast<float>(std::rand() % WINDOW_HEIGHT);
        m_snow[i].speed = 28.f + static_cast<float>(std::rand() % 55);
        m_snow[i].drift = 10.f + static_cast<float>(std::rand() % 22);
        m_snow[i].driftPhase = static_cast<float>(std::rand() % 628) / 100.f;
        m_snow[i].radius = 1.5f + static_cast<float>(std::rand() % 3);
        m_snow[i].alpha = 130 + static_cast<sf::Uint8>(std::rand() % 125);
    }
}

void Gamakichi::updateSnow(float dt)
{
    for (int i = 0; i < MAX_SNOW; ++i)
    {
        m_snow[i].driftPhase += dt * 1.3f;
        m_snow[i].pos.y += m_snow[i].speed * dt;
        m_snow[i].pos.x += std::sin(m_snow[i].driftPhase) *
            m_snow[i].drift * dt;

        if (m_snow[i].pos.y > WINDOW_HEIGHT + 10.f)
        {
            m_snow[i].pos.y = -5.f;
            m_snow[i].pos.x = static_cast<float>(std::rand() % WINDOW_WIDTH);
        }
        if (m_snow[i].pos.x < -10.f)               m_snow[i].pos.x = WINDOW_WIDTH + 5.f;
        if (m_snow[i].pos.x > WINDOW_WIDTH + 10.f)  m_snow[i].pos.x = -5.f;
    }
}

void Gamakichi::drawSnow(sf::RenderWindow& window) const
{
    for (int i = 0; i < MAX_SNOW; ++i)
    {
        sf::CircleShape flake(m_snow[i].radius, 8);
        flake.setOrigin(m_snow[i].radius, m_snow[i].radius);
        flake.setPosition(m_snow[i].pos);
        flake.setFillColor(sf::Color(220, 235, 255, m_snow[i].alpha));
        window.draw(flake);
    }
}



Gamakichi::Gamakichi(float x, float y)
    : Enemy(x, y, EnemyVariant::Red)
{
    m_health = 20;
    m_maxHealth = 20;
    m_hitsToEncase = 999;
    m_speed = 0.f;
    m_scoreValue = SCORE_GAMAKICHI;
    m_gemDrop = GEMS_GAMAKICHI_REWARD;

    m_introBattleY = y;
    m_introStartY = static_cast<float>(WINDOW_HEIGHT) + 60.f;
    m_position.y = m_introStartY;
    m_introPauseTimer = 0.f;
    m_introBombTimer = 0.f;
    m_phase = Phase::IntroPause;

    for (int i = 0; i < MAX_SHELLS; ++i)
    {
        m_smokeActive[i] = false;
        m_smokeFrame[i] = 0;
        m_smokeTimer[i] = 0.f;
        m_smokePos[i] = { 0.f, 0.f };
    }
    for (int t = 0; t < TUBE_COUNT; ++t)
    {
        m_tubeFlashActive[t] = false;
        m_tubeFlashFrame[t] = 0;
        m_tubeFlashTimer[t] = 0.f;
    }
    for (int i = 0; i < MAX_CHILDREN; ++i)
        m_pendingChildren[i] = nullptr;

    initSnow();

    m_hasTexture = m_texture.loadFromFile("Gamakichi.png");
    if (m_hasTexture)
    {
        // Make pure-black pixels transparent (sprite background)
        sf::Image img = m_texture.copyToImage();
        for (unsigned iy = 0; iy < img.getSize().y; ++iy)
            for (unsigned ix = 0; ix < img.getSize().x; ++ix)
            {
                sf::Color c = img.getPixel(ix, iy);
                if (c.r < 12 && c.g < 12 && c.b < 12)
                    img.setPixel(ix, iy, sf::Color::Transparent);
            }
        m_texture.loadFromImage(img);
        m_sprite.setTexture(m_texture);
    }

    m_hitBox = new HitBox(HEAD_DRAW_W, 130.f, 0.f, 0.f);
    m_hitBox->update(m_position);
}

//  Phase helpers

Gamakichi::Phase Gamakichi::currentPhase() const
{
    float r = static_cast<float>(m_health) /
        static_cast<float>(m_maxHealth);
    if (r > 0.6f) return Phase::Slow;
    if (r > 0.3f) return Phase::Aggressive;
    return Phase::Frenzy;
}

void Gamakichi::updatePhase()
{
    if (m_phase == Phase::Intro || m_phase == Phase::IntroPause) return;

    Phase p = currentPhase();
    if (p == m_phase) return;
    m_phase = p;

    switch (m_phase)
    {
    case Phase::Slow:
        m_fireInterval = 3.0f;
        m_shotsPerVolley = 2;
        m_childThrowInterval = 7.0f;
        break;
    case Phase::Aggressive:
        m_fireInterval = 1.8f;
        m_shotsPerVolley = 3;
        m_childThrowInterval = 5.0f;
        break;
    case Phase::Frenzy:
        m_fireInterval = 1.0f;
        m_shotsPerVolley = 5;
        m_childThrowInterval = 3.0f;
        break;
    default: break;
    }
}

//  Smoke

void Gamakichi::updateSmoke(float dt)
{
    for (int i = 0; i < MAX_SHELLS; ++i)
    {
        if (!m_smokeActive[i]) continue;
        m_smokeTimer[i] += dt;
        if (m_smokeTimer[i] >= 0.10f)
        {
            m_smokeTimer[i] = 0.f;
            if (++m_smokeFrame[i] >= SMOKE_COUNT)
                m_smokeActive[i] = false;
        }
    }
}

void Gamakichi::drawSmoke(sf::RenderWindow& window) const
{
    if (!m_hasTexture) return;
    for (int i = 0; i < MAX_SHELLS; ++i)
    {
        if (!m_smokeActive[i]) continue;
        const sf::IntRect& fr = SMOKE_FRAMES[m_smokeFrame[i]];
        sf::Sprite s(m_texture);
        s.setTextureRect(fr);
        float sc = 90.f / static_cast<float>(fr.width);
        s.setScale(sc, sc);
        s.setOrigin(fr.width * 0.5f, fr.height * 0.5f);
        s.setPosition(m_smokePos[i]);
        sf::Uint8 a = static_cast<sf::Uint8>(255 - m_smokeFrame[i] * 255 / SMOKE_COUNT);
        s.setColor(sf::Color(255, 255, 255, a));
        window.draw(s);
    }
}

// =========================================================
//  Tube flash
// =========================================================

void Gamakichi::updateTubeFlashes(float dt)
{
    for (int t = 0; t < TUBE_COUNT; ++t)
    {
        if (!m_tubeFlashActive[t]) continue;
        m_tubeFlashTimer[t] += dt;
        if (m_tubeFlashTimer[t] >= 0.07f)
        {
            m_tubeFlashTimer[t] = 0.f;
            if (++m_tubeFlashFrame[t] >= FLASH_COUNT)
                m_tubeFlashActive[t] = false;
        }
    }
}

sf::Vector2f Gamakichi::getTubeTipPos(int t) const
{
    const sf::IntRect& hr = (m_headFrame == 0) ? HEAD_IDLE : HEAD_ATTACK;
    float sc = HEAD_DRAW_W / static_cast<float>(hr.width);

    float ox = m_position.x + HEAD_DRAW_W * 0.5f - hr.width * 0.5f * sc;
    float oy = m_position.y + 60.f - hr.height * 0.5f * sc;

    return { ox + TUBE_OFFSETS_X[t] * sc,
             oy + TUBE_TOP_Y_OFFSET * sc };
}

// =========================================================
//  throwIntroBomb
// =========================================================

void Gamakichi::throwIntroBomb()
{
    if (m_pendingChildCount >= MAX_CHILDREN) return;

    float spawnX = m_position.x + HEAD_DRAW_W * 0.5f;
    float spawnY = m_position.y + 20.f;

    float targetX = 60.f + static_cast<float>(std::rand() %
        static_cast<int>(WINDOW_WIDTH - 120.f));

    float arcTime = 1.4f + static_cast<float>(std::rand() % 70) / 100.f;
    float vx = (targetX - spawnX) / arcTime;
    float vy = -390.f - static_cast<float>(std::rand() % 80);

    auto* bomb = new GamakichiChild(spawnX, spawnY, vx, vy);
    m_pendingChildren[m_pendingChildCount++] = bomb;
}

// =========================================================
//  throwBombs
// =========================================================

void Gamakichi::throwBombs(int count)
{
    for (int i = 0; i < count && m_pendingChildCount < MAX_CHILDREN; ++i)
    {
        float spawnX = m_position.x + HEAD_DRAW_W * 0.5f;
        float spawnY = m_position.y + 20.f;

        float targetX = 60.f + static_cast<float>(std::rand() %
            static_cast<int>(WINDOW_WIDTH - 120.f));

        float arcTime = 1.2f + static_cast<float>(std::rand() % 80) / 100.f;
        float vx = (targetX - spawnX) / arcTime;
        float vy = -370.f - static_cast<float>(std::rand() % 100);

        auto* bomb = new GamakichiChild(spawnX, spawnY, vx, vy);
        m_pendingChildren[m_pendingChildCount++] = bomb;
    }
}

// =========================================================
//  fireVolley
// =========================================================

void Gamakichi::fireVolley()
{
    // Target nearest player, fall back to screen centre
    sf::Vector2f target(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT - 60.f);
    if (m_players && m_playerCount > 0)
    {
        target = m_players[0]->getPosition();
        if (m_playerCount > 1)
        {
            sf::Vector2f p1 = m_players[1]->getPosition();
            sf::Vector2f me = m_position;
            float d0 = (target.x - me.x) * (target.x - me.x) + (target.y - me.y) * (target.y - me.y);
            float d1 = (p1.x - me.x) * (p1.x - me.x) + (p1.y - me.y) * (p1.y - me.y);
            if (d1 < d0) target = p1;
        }
    }

    const int   tubeStart = (m_phase == Phase::Slow) ? 1 : 0;
    const int   tubeEnd = (m_phase == Phase::Slow) ? 3 : TUBE_COUNT;
    const float speed = 300.f;
    const float spreadStep = 0.15f;
    float       startOff = -spreadStep * ((m_shotsPerVolley - 1) / 2.f);

    for (int t = tubeStart; t < tubeEnd; ++t)
    {
        m_tubeFlashActive[t] = true;
        m_tubeFlashFrame[t] = 0;
        m_tubeFlashTimer[t] = 0.f;

        sf::Vector2f origin = getTubeTipPos(t);
        sf::Vector2f toTgt = target - origin;
        float len = std::sqrt(toTgt.x * toTgt.x + toTgt.y * toTgt.y);
        if (len < 1.f) len = 1.f;
        toTgt /= len;

        for (int s = 0; s < m_shotsPerVolley; ++s)
        {
            float angle = startOff + spreadStep * s;
            float cosA = std::cos(angle), sinA = std::sin(angle);

            sf::Vector2f dir;
            dir.x = toTgt.x * cosA - toTgt.y * sinA;
            dir.y = toTgt.x * sinA + toTgt.y * cosA;

            for (int i = 0; i < MAX_SHELLS; ++i)
            {
                if (!m_shells[i].active)
                {
                    m_shells[i] = ArtilleryShell{};
                    m_shells[i].pos = origin;
                    m_shells[i].vel = dir * speed;
                    m_shells[i].active = true;
                    break;
                }
            }
        }
    }
}

// =========================================================
//  update
// =========================================================

void Gamakichi::update(float dt)
{
    if (!m_active) return;

    if (m_hitFlashTimer > 0.f) m_hitFlashTimer -= dt;

    updateSnow(dt);

    // ── IntroPause ────────────────────────────────────────
    if (m_phase == Phase::IntroPause)
    {
        m_headTimer += dt;
        if (m_headTimer >= 0.40f) { m_headTimer = 0.f; m_headFrame = 1 - m_headFrame; }

        m_introBombTimer += dt;
        if (m_introBombTimer >= INTRO_BOMB_INTV)
        {
            m_introBombTimer = 0.f;
            throwIntroBomb();
        }

        m_introPauseTimer += dt;
        if (m_introPauseTimer >= INTRO_PAUSE_DUR)
            m_phase = Phase::Intro;

        m_hitBox->update(m_position);
        return;
    }

    // ── Intro rise ────────────────────────────────────────
    if (m_phase == Phase::Intro)
    {
        m_position.y -= m_introSpeed * dt;

        if (m_position.y <= m_introBattleY)
        {
            m_position.y = m_introBattleY;
            m_phase = Phase::Slow;
            m_fireInterval = 3.0f;
            m_shotsPerVolley = 2;
            m_childThrowInterval = 7.0f;
            m_childThrowTimer = 2.0f;
        }

        m_headTimer += dt;
        if (m_headTimer >= 0.28f) { m_headTimer = 0.f; m_headFrame = 1 - m_headFrame; }

        m_hitBox->update(m_position);
        return;
    }

    // ── Battle ────────────────────────────────────────────
    updatePhase();

    // Head bob speed varies by phase
    float headDur = (m_phase == Phase::Frenzy) ? 0.14f :
        (m_phase == Phase::Aggressive) ? 0.22f : 0.38f;
    m_headTimer += dt;
    if (m_headTimer >= headDur)
    {
        m_headTimer = 0.f;
        m_headFrame = (m_headFrame + 1) % 2;
    }

    // ── Eye blink: ping-pong  0 → 5 → 0 → … ─────────────
    //   Total cycle length = EYE_COUNT*2 - 2  (= 10 for 6 frames)
    //   Frame index mapping:
    //     eyeFrame 0..5  → use directly
    //     eyeFrame 6..9  → mirror back: frame = (10 - eyeFrame) = 4,3,2,1
    static constexpr int EYE_CYCLE = EYE_COUNT * 2 - 2;   // 10
    m_eyeTimer += dt;
    if (m_eyeTimer >= 0.13f)
    {
        m_eyeTimer = 0.f;
        m_eyeFrame = (m_eyeFrame + 1) % EYE_CYCLE;
    }

    updateTubeFlashes(dt);

    // Artillery
    m_fireTimer += dt;
    if (m_fireTimer >= m_fireInterval) { m_fireTimer = 0.f; fireVolley(); }

    // Dark bombs
    m_childThrowTimer -= dt;
    if (m_childThrowTimer <= 0.f)
    {
        m_childThrowTimer = m_childThrowInterval;
        int cnt = (m_phase == Phase::Frenzy) ? 4 :
            (m_phase == Phase::Aggressive) ? 3 : 2;
        throwBombs(cnt);
    }

    // Shells + smoke
    for (int i = 0; i < MAX_SHELLS; ++i)
    {
        bool wasEx = m_shells[i].exploded;
        m_shells[i].update(dt);
        if (m_shells[i].active && m_shells[i].exploded && !wasEx)
        {
            m_smokeActive[i] = true;
            m_smokeFrame[i] = 0;
            m_smokeTimer[i] = 0.f;
            m_smokePos[i] = m_shells[i].pos;
        }
    }

    updateSmoke(dt);
    m_hitBox->update(m_position);
}

// =========================================================
//  takeHit / onChainKill
// =========================================================

bool Gamakichi::takeHit()
{
    --m_health;
    m_hitFlashTimer = 0.25f;
    if (m_health <= 0) { m_active = false; m_hasDrop = true; }
    return false;
}

void Gamakichi::onChainKill()
{
    m_health -= 3;
    m_hitFlashTimer = 0.40f;
    if (m_health <= 0) { m_active = false; m_hasDrop = true; }
}

// =========================================================
//  drawTubeFlashes
// =========================================================

void Gamakichi::drawTubeFlashes(sf::RenderWindow& window) const
{
    if (!m_hasTexture) return;
    for (int t = 0; t < TUBE_COUNT; ++t)
    {
        if (!m_tubeFlashActive[t]) continue;
        sf::Vector2f       tip = getTubeTipPos(t);
        const sf::IntRect& fr = FLASH_FRAMES[m_tubeFlashFrame[t]];
        sf::Sprite         fl(m_texture);
        fl.setTextureRect(fr);
        float sc = FLASH_DRAW_W / static_cast<float>(fr.width);
        fl.setScale(sc, sc);
        fl.setOrigin(fr.width * 0.5f, static_cast<float>(fr.height));
        fl.setRotation(180.f);
        fl.setPosition(tip);
        fl.setColor(sf::Color(255, 230, 0, 230));
        window.draw(fl);
    }
}

// =========================================================
//  draw
// =========================================================

void Gamakichi::draw(sf::RenderWindow& window)
{
    if (!m_active) return;

    drawSnow(window);

    for (int i = 0; i < MAX_SHELLS; ++i)
        m_shells[i].draw(window, m_texture);
    drawSmoke(window);

    bool flashing = (m_hitFlashTimer > 0.f) &&
        (static_cast<int>(m_hitFlashTimer * 20) % 2 == 0);

    if (m_hasTexture)
    {
        // ── Resolve ping-pong eye frame ───────────────────
        //   eyeFrame 0..5  → direct index
        //   eyeFrame 6..9  → mirror: (EYE_CYCLE - eyeFrame) = 4,3,2,1
        int eyeIdx = m_eyeFrame;
        if (eyeIdx >= EYE_COUNT)
            eyeIdx = (EYE_COUNT * 2 - 2) - eyeIdx;
        // eyeIdx is now always in [0, EYE_COUNT-1]

        // ── Head sprite ───────────────────────────────────
        const sf::IntRect& hr = (m_headFrame == 0) ? HEAD_IDLE : HEAD_ATTACK;
        m_sprite.setTextureRect(hr);
        float sc = HEAD_DRAW_W / static_cast<float>(hr.width);
        m_sprite.setScale(sc, sc);
        m_sprite.setOrigin(hr.width * 0.5f, hr.height * 0.5f);
        m_sprite.setPosition(m_position.x + HEAD_DRAW_W * 0.5f,
            m_position.y + 60.f);
        m_sprite.setColor(flashing ? sf::Color(255, 80, 80) : sf::Color::White);
        window.draw(m_sprite);

        // ── Two eyes (left socket, right socket) ──────────
        //   The eye frames are close-up crops.  We scale them
        //   to EYE_DRAW_W and position them in the correct
        //   sockets on the head.
        const sf::IntRect& er = EYE_FRAMES[eyeIdx];
        float              esc = EYE_DRAW_W / static_cast<float>(er.width);

        sf::Sprite es(m_texture);
        es.setTextureRect(er);
        es.setScale(esc, esc);
        es.setOrigin(er.width * 0.5f, er.height * 0.5f);

        // Left eye socket
        es.setPosition(m_position.x + 60.f,
            m_position.y + 35.f);
        window.draw(es);

        // Right eye socket (mirror horizontally so both look inward)
        es.setScale(-esc, esc);
        es.setPosition(m_position.x + HEAD_DRAW_W - 60.f,
            m_position.y + 35.f);
        window.draw(es);

        drawTubeFlashes(window);
    }
    else
    {
        // ── Procedural fallback ───────────────────────────
        sf::RectangleShape body({ HEAD_DRAW_W, 130.f });
        body.setPosition(m_position);
        sf::Color bc = flashing ? sf::Color(255, 60, 60) :
            (m_phase == Phase::Frenzy) ? sf::Color(160, 20, 20) :
            (m_phase == Phase::Aggressive) ? sf::Color(140, 40, 30) :
            sf::Color(120, 30, 30);
        body.setFillColor(bc);
        body.setOutlineColor({ 60, 10, 10 });
        body.setOutlineThickness(3.f);
        window.draw(body);

        for (int t = 0; t < TUBE_COUNT; ++t)
        {
            float tsc = HEAD_DRAW_W / static_cast<float>(HEAD_IDLE.width);
            sf::RectangleShape tube({ 14.f, 30.f });
            tube.setPosition(m_position.x + TUBE_OFFSETS_X[t] * tsc - 7.f,
                m_position.y - 25.f);
            tube.setFillColor({ 160, 160, 170 });
            tube.setOutlineColor({ 80, 80, 90 });
            tube.setOutlineThickness(2.f);
            window.draw(tube);
        }

        sf::CircleShape eye(14.f, 20);
        eye.setFillColor({ 255, 200, 0 });
        eye.setOutlineColor(sf::Color::Black);
        eye.setOutlineThickness(2.f);

        eye.setOrigin(14.f, 14.f);
        eye.setPosition(m_position.x + 60.f, m_position.y + 35.f);
        window.draw(eye);
        eye.setPosition(m_position.x + HEAD_DRAW_W - 60.f, m_position.y + 35.f);
        window.draw(eye);

        if (m_headFrame == 1)
        {
            sf::RectangleShape mouth({ 90.f, 22.f });
            mouth.setPosition(m_position.x + 65.f, m_position.y + 78.f);
            mouth.setFillColor({ 20, 5, 5 });
            mouth.setOutlineColor(sf::Color::Black);
            mouth.setOutlineThickness(1.f);
            window.draw(mouth);

            for (int ti = 0; ti < 5; ++ti)
            {
                sf::RectangleShape tooth({ 6.f, 10.f });
                tooth.setPosition(m_position.x + 50.f + ti * 22.f,
                    m_position.y + 82.f);
                tooth.setFillColor({ 240, 230, 210 });
                window.draw(tooth);
            }
        }

        for (int t = 0; t < TUBE_COUNT; ++t)
        {
            if (!m_tubeFlashActive[t]) continue;
            float tsc = HEAD_DRAW_W / static_cast<float>(HEAD_IDLE.width);
            sf::CircleShape dot(8.f, 6);
            dot.setFillColor({ 255, 220, 0, 220 });
            dot.setOrigin(8.f, 8.f);
            dot.setPosition(m_position.x + TUBE_OFFSETS_X[t] * tsc,
                m_position.y - 40.f);
            window.draw(dot);
        }
    }

    if (m_phase != Phase::Intro && m_phase != Phase::IntroPause)
        drawHealthBar(window);
}

// =========================================================
//  drawHealthBar
// =========================================================

void Gamakichi::drawHealthBar(sf::RenderWindow& window) const
{
    const float barW = 280.f, barH = 18.f;
    const float barX = (WINDOW_WIDTH - barW) * 0.5f;
    const float barY = 8.f;

    sf::RectangleShape bg({ barW, barH });
    bg.setPosition(barX, barY);
    bg.setFillColor({ 30, 5, 5 });
    bg.setOutlineColor({ 160, 20, 20 });
    bg.setOutlineThickness(2.f);
    window.draw(bg);

    float ratio = static_cast<float>(m_health) /
        static_cast<float>(m_maxHealth);
    if (ratio < 0.f) ratio = 0.f;

    sf::RectangleShape fill({ barW * ratio, barH });
    fill.setPosition(barX, barY);
    sf::Color bc = (m_phase == Phase::Frenzy) ? sf::Color(255, 30, 10) :
        (m_phase == Phase::Aggressive) ? sf::Color(255, 120, 20) :
        sf::Color(220, 40, 20);
    fill.setFillColor(bc);
    window.draw(fill);

    for (float pct : {0.6f, 0.3f})
    {
        sf::RectangleShape pip({ 2.f, barH });
        pip.setPosition(barX + barW * pct, barY);
        pip.setFillColor({ 255, 255, 255, 160 });
        window.draw(pip);
    }
}

// =========================================================
//  drawDebug
// =========================================================

void Gamakichi::drawDebug(sf::RenderWindow& window)
{
    if (m_hitBox) m_hitBox->drawDebug(window, sf::Color::Red);

    for (int t = 0; t < TUBE_COUNT; ++t)
    {
        sf::Vector2f  tip = getTubeTipPos(t);
        sf::CircleShape dot(4.f);
        dot.setFillColor(sf::Color::Cyan);
        dot.setOrigin(4.f, 4.f);
        dot.setPosition(tip);
        window.draw(dot);
    }

    for (int i = 0; i < MAX_SHELLS; ++i)
    {
        if (!m_shells[i].active || m_shells[i].exploded) continue;
        sf::CircleShape dot(5.f);
        dot.setFillColor({ 255, 100, 0, 120 });
        dot.setOutlineColor({ 255, 100, 0 });
        dot.setOutlineThickness(1.f);
        dot.setOrigin(5.f, 5.f);
        dot.setPosition(m_shells[i].pos);
        window.draw(dot);
    }
}