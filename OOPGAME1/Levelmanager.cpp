#include "Levelmanager.h"
#include "Constants.h"
#include "MogeraChild.h"
#include "GamakichiChild.h"   // explicit include — don't rely on Gamakichi.h chain
#include <cstdlib>
#include <cstring>
#include <cmath>

// =========================================================
LevelManager::LevelManager()
    : m_currentLevel(1), m_levelCount(0),
    m_platformCount(0), m_enemyCount(0),
    m_snowballCount(0), m_dropCount(0),
    m_knifeCount(0)
{
    for (int i = 0; i < MAX_ENEMIES; i++)   m_enemies[i] = nullptr;
    for (int i = 0; i < MAX_SNOWBALLS; i++) m_snowballs[i] = nullptr;
    for (int i = 0; i < MAX_PLATFORMS; i++) m_platformPtrs[i] = nullptr;
}

LevelManager::~LevelManager()
{
    clearEnemies();
    clearSnowballs();
}

// =========================================================
static bool streq(const char* a, const char* b)
{
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return a[i] == b[i];
}

// =========================================================
void LevelManager::clearEnemies()
{
    for (int i = 0; i < m_enemyCount; i++)
    {
        delete m_enemies[i];
        m_enemies[i] = nullptr;
    }
    m_enemyCount = 0;
}

void LevelManager::clearSnowballs()
{
    for (int i = 0; i < m_snowballCount; i++)
    {
        delete m_snowballs[i];
        m_snowballs[i] = nullptr;
    }
    m_snowballCount = 0;
}

// =========================================================
void LevelManager::buildLevelRegistry()
{
    m_levelCount = 0;

    // ---- Level 1 ----
    {
        LevelConfig& c = m_levelRegistry[m_levelCount++];
        c = LevelConfig();
        c.levelNumber = 1;
        strcpy_s(c.backgroundPath, "assets/images/l2.png");
        strcpy_s(c.platformImagePath, "assets/images/p22.png");
        strcpy_s(c.musicPath, "assets/music/ami_je_tom.ogg");
        c.backgroundColor = sf::Color::White;
        c.addPlatform(0, 550, 800, 35);
        c.addPlatform(0, 400, 200, 35);
        c.addPlatform(620, 400, 200, 35);
        c.addPlatform(300, 400, 200, 35);
        c.addPlatform(200, 250, 400, 35);
        c.addPlatform(620, 100, 200, 35);
        c.addPlatform(0, 100, 200, 35);
        c.addEnemy("Botom", 150, 100, EnemyVariant::Red);
        c.addEnemy("Botom", 500, 100, EnemyVariant::Red);
        c.addEnemy("Botom", 300, 100, EnemyVariant::Red);
        c.addEnemy("Botom", 200, 100, EnemyVariant::Red);
        c.addEnemy("Botom", 350, 100, EnemyVariant::Red);
    }
    // ---- Level 2 ----
    {
        LevelConfig& c = m_levelRegistry[m_levelCount++];
        c = LevelConfig();
        c.levelNumber = 2;
        strcpy_s(c.backgroundPath, "assets/images/l2.png");
        strcpy_s(c.platformImagePath, "assets/images/p22.png");
        strcpy_s(c.musicPath, "assets/music/ami_je_tomar.ogg");
        c.backgroundColor = sf::Color(10, 10, 30);
        c.addPlatform(0, 550, 800, 60);
        c.addPlatform(240, 460, 220, 35);
        c.addPlatform(160, 300, 220, 35);
        c.addPlatform(380, 200, 220, 35);
        c.addPlatform(440, 100, 220, 35);
        c.addPlatform(200, 50, 220, 35);
        c.addEnemy("Botom", 280, 70, EnemyVariant::Red);
        c.addEnemy("Botom", 430, 70, EnemyVariant::Red);
        c.addEnemy("FlyingEnemy", 540, 190, EnemyVariant::Red);
        c.addEnemy("Botom", 470, 400, EnemyVariant::Red);
        c.addEnemy("Botom", 320, 480, EnemyVariant::Red);
    }
    // ---- Level 3 ----
    {
        LevelConfig& c = m_levelRegistry[m_levelCount++];
        c = LevelConfig();
        c.levelNumber = 3;
        strcpy_s(c.backgroundPath, "assets/images/l3.png");
        strcpy_s(c.platformImagePath, "assets/images/p33.png");
        strcpy_s(c.musicPath, "assets/music/ami_je_tomar.ogg");
        c.backgroundColor = sf::Color(20, 5, 15);
        c.addPlatform(0, 550, 800, 25);
        c.addPlatform(220, 100, 360, 22);
        c.addPlatform(120, 190, 200, 22);
        c.addPlatform(480, 190, 200, 22);
        c.addPlatform(300, 280, 110, 22);
        c.addPlatform(420, 280, 110, 22);
        c.addPlatform(200, 370, 180, 22);
        c.addPlatform(420, 370, 180, 22);
        c.addPlatform(80, 460, 160, 22);
        c.addPlatform(300, 460, 200, 22);
        c.addPlatform(560, 460, 160, 22);
        c.addEnemy("Botom", 100, 430, EnemyVariant::Red);
        c.addEnemy("Botom", 600, 430, EnemyVariant::Red);
        c.addEnemy("FlyingEnemy", 250, 320, EnemyVariant::Red);
        c.addEnemy("Botom", 500, 320, EnemyVariant::Red);
        c.addEnemy("Tornado", 350, 150, EnemyVariant::Red);
    }
    // ---- Level 4 — Collectables only (no enemies) ----
    {
        LevelConfig& c = m_levelRegistry[m_levelCount++];
        c = LevelConfig();
        c.levelNumber = 4;
        strcpy_s(c.backgroundPath, "assets/images/l4.png");
        strcpy_s(c.platformImagePath, "assets/images/p33.png");
        strcpy_s(c.musicPath, "assets/music/ami_je_tomar.ogg");
        c.backgroundColor = sf::Color(5, 5, 5);
        c.addPlatform(0, 550, 800, 25);
        c.addPlatform(80, 460, 640, 22);
        c.addPlatform(80, 370, 260, 22);
        c.addPlatform(420, 370, 280, 22);
        c.addPlatform(320, 415, 120, 22);
        c.addPlatform(80, 280, 280, 22);
        c.addPlatform(400, 280, 280, 22);
        c.addPlatform(200, 190, 400, 22);
        c.addPlatform(80, 325, 120, 22);
    }
    // ---- Level 5 — MOGERA BOSS ----
    {
        LevelConfig& c = m_levelRegistry[m_levelCount++];
        c = LevelConfig();
        c.levelNumber = 5;
        c.isBossLevel = true;
        strcpy_s(c.backgroundPath, "assets/images/i55.png");
        strcpy_s(c.platformImagePath, "assets/images/p52.png");
        strcpy_s(c.musicPath, "assets/music/boss_theme.ogg");
        c.backgroundColor = sf::Color(30, 0, 0);
        c.addPlatform(0, 550, 800, 45);
        c.addPlatform(0, 400, 300, 45);   //highest
        c.addPlatform(0, 150, 250, 60);
        c.addPlatform(550, 150, 250, 60);
        c.addPlatform(200, 280, 400, 45);
        c.addPlatform(550,400 , 300, 45);
        c.addEnemy("Mogera", 650,80 );
    }
    // ---- Level 6 ----
    {
        LevelConfig& c = m_levelRegistry[m_levelCount++];
        c = LevelConfig();
        c.levelNumber = 6;
        strcpy_s(c.backgroundPath, "assets/images/l6.png");
        strcpy_s(c.platformImagePath, "assets/images/plat3.png");
        strcpy_s(c.musicPath, "assets/music/ami_je_tomar.ogg");
        c.backgroundColor = sf::Color(20, 20, 40);
        c.addPlatform(0, 550, 800, 25);
        c.addPlatform(0, 450, 160, 22);
        c.addPlatform(300, 450, 200, 22);
        c.addPlatform(640, 450, 160, 22);
        c.addPlatform(0, 340, 220, 22);
        c.addPlatform(580, 340, 220, 22);
        c.addPlatform(280, 220, 240, 22);
        c.addPlatform(0, 220, 140, 22);
        c.addPlatform(660, 220, 140, 22);
        c.addEnemy("Botom", 300, 185, EnemyVariant::Blue);
        c.addEnemy("Botom", 450, 185, EnemyVariant::Blue);
        c.addEnemy("Tornado", 30, 185, EnemyVariant::Blue);
        c.addEnemy("FlyingEnemy", 720, 185, EnemyVariant::Blue);
    }
    // ---- Level 7 ----
    {
        LevelConfig& c = m_levelRegistry[m_levelCount++];
        c = LevelConfig();
        c.levelNumber = 7;
        strcpy_s(c.backgroundPath, "assets/images/l7.png");
        strcpy_s(c.platformImagePath, "assets/images/p6.png");
        strcpy_s(c.musicPath, "assets/music/ami_je_tomar.ogg");
        c.backgroundColor = sf::Color(40, 40, 60);
        c.addPlatform(0, 550, 800, 25);
        c.addPlatform(0, 440, 420, 22);
        c.addPlatform(380, 330, 420, 22);
        c.addPlatform(0, 220, 420, 22);
        c.addPlatform(380, 110, 420, 22);
        c.addEnemy("Botom", 80, 410, EnemyVariant::Blue);
        c.addEnemy("FlyingEnemy", 320, 410, EnemyVariant::Blue);
        c.addEnemy("Botom", 520, 300, EnemyVariant::Blue);
        c.addEnemy("Tornado", 700, 300, EnemyVariant::Blue);
        c.addEnemy("Botom", 80, 190, EnemyVariant::Blue);
        c.addEnemy("FlyingEnemy", 300, 190, EnemyVariant::Blue);
        c.addEnemy("Botom", 550, 80, EnemyVariant::Blue);
        c.addEnemy("Tornado", 720, 80, EnemyVariant::Blue);
    }
    // ---- Level 8 ----
    {
        LevelConfig& c = m_levelRegistry[m_levelCount++];
        c = LevelConfig();
        c.levelNumber = 8;
        strcpy_s(c.backgroundPath, "assets/images/l8.png");
        strcpy_s(c.platformImagePath, "assets/images/p6.png");
        strcpy_s(c.musicPath, "assets/music/ami_je_tomar.ogg");
        c.backgroundColor = sf::Color(30, 30, 50);
        c.addPlatform(0, 550, 800, 25);
        c.addPlatform(160, 440, 480, 22);
        c.addPlatform(0, 330, 320, 22);
        c.addPlatform(480, 330, 320, 22);
        c.addPlatform(180, 220, 440, 22);
        c.addPlatform(300, 110, 200, 22);
        c.addEnemy("Botom", 220, 190, EnemyVariant::Blue);
        c.addEnemy("Botom", 560, 190, EnemyVariant::Blue);
        c.addEnemy("FlyingEnemy", 400, 190, EnemyVariant::Blue);
        c.addEnemy("Botom", 100, 300, EnemyVariant::Blue);
        c.addEnemy("FlyingEnemy", 650, 300, EnemyVariant::Blue);
        c.addEnemy("Tornado", 400, 410, EnemyVariant::Blue);
    }
    // ---- Level 9 — Collectables only (no enemies) ----
    {
        LevelConfig& c = m_levelRegistry[m_levelCount++];
        c = LevelConfig();
        c.levelNumber = 9;
        strcpy_s(c.backgroundPath, "assets/images/l9.png");
        strcpy_s(c.platformImagePath, "assets/images/p4.png");
        strcpy_s(c.musicPath, "assets/music/ami_je_tomar.ogg");
        c.backgroundColor = sf::Color(10, 10, 20);
        c.addPlatform(0, 550, 800, 25);
        c.addPlatform(0, 440, 260, 22);
        c.addPlatform(540, 440, 260, 22);
        c.addPlatform(160, 330, 480, 22);
        c.addPlatform(0, 220, 300, 22);
        c.addPlatform(500, 220, 300, 22);
        c.addPlatform(200, 110, 400, 22);
    }
    // ---- Level 10 — GAMAKICHI FINAL BOSS ----
    {
        LevelConfig& c = m_levelRegistry[m_levelCount++];
        c = LevelConfig();
        c.levelNumber = 10;
        c.isBossLevel = true;
        strcpy_s(c.backgroundPath, "assets/images/l10.png");
        strcpy_s(c.platformImagePath, "assets/images/p4.png");
        strcpy_s(c.musicPath, "assets/music/final_boss.ogg");
        c.backgroundColor = sf::Color(40, 0, 10);
        c.addPlatform(0, 550, 800, 25);
        c.addPlatform(500, 440, 430, 22);
        c.addPlatform(0, 440, 300, 22);
        c.addPlatform(0, 330, 370, 22);
        c.addPlatform(500, 330, 430, 22);
        c.addPlatform(200, 200, 470, 22);
        c.addEnemy("Gamakichi", 300, 90);
    }

    // Mark 2 random non-boss levels as bonus
    int nonBoss[MAX_LEVELS];
    int nonBossCount = 0;
    for (int i = 0; i < m_levelCount; i++)
        if (!m_levelRegistry[i].isBossLevel)
            nonBoss[nonBossCount++] = i;

    for (int b = 0; b < BONUS_LEVEL_COUNT && nonBossCount > 0; b++)
    {
        int pick = rand() % nonBossCount;
        m_levelRegistry[nonBoss[pick]].isBonusLevel = true;
        nonBoss[pick] = nonBoss[--nonBossCount];
    }

    nonBossCount = 0;
    for (int i = 0; i < m_levelCount; i++)
        if (!m_levelRegistry[i].isBossLevel)
            nonBoss[nonBossCount++] = i;

    if (nonBossCount > 0)
    {
        int starIdx = rand() % nonBossCount;
        m_starLevelNumber = m_levelRegistry[nonBoss[starIdx]].levelNumber;
    }
}

// =========================================================
void LevelManager::loadLevel(int levelNumber)
{
    m_currentLevel = levelNumber;
    clearEnemies();
    clearSnowballs();
    m_platformCount = 0;
    m_dropCount = 0;
    m_knifeCount = 0;
    m_bonusRainActive = false;

    m_starSpawned = false;
    m_starCollected = false;
    m_starActive = false;
    m_starChoiceOpen = false;
    m_starBobTimer = 0.f;

    if (levelNumber < 1 || levelNumber > m_levelCount) return;
    const LevelConfig& cfg = m_levelRegistry[levelNumber - 1];

    for (int i = 0; i < cfg.platformCount; i++)
    {
        const PlatformData& pd = cfg.platforms[i];
        m_platforms[m_platformCount] = Platform(pd.x, pd.y, pd.width, pd.height);
        m_platforms[m_platformCount].load(cfg.platformImagePath);
        m_platformCount++;
    }

    buildPlatformRectCache();

    for (int i = 0; i < cfg.enemyCount; i++)
    {
        Enemy* e = spawnEnemy(cfg.enemies[i]);
        if (e && m_enemyCount < MAX_ENEMIES)
            m_enemies[m_enemyCount++] = e;
    }

    // ---- Collectable-goods levels (4 and 9) ----
    if (levelNumber == 4 || levelNumber == 9)
    {
        struct SpawnPos { float x; float y; DropType type; };
        SpawnPos spawns[] = {
            { 120.f, 510.f, DropType::Gem },
            { 200.f, 510.f, DropType::FaceBlue },
            { 300.f, 510.f, DropType::BottleRed },
            { 420.f, 510.f, DropType::Gem },
            { 540.f, 510.f, DropType::FaceRed },
            { 660.f, 510.f, DropType::Gem },
            { 150.f, 340.f, DropType::BottleBlue },
            { 300.f, 340.f, DropType::Gem },
            { 450.f, 340.f, DropType::BottleYellow },
            { 600.f, 340.f, DropType::Gem },
            { 200.f, 230.f, DropType::FaceBlue },
            { 400.f, 230.f, DropType::FaceRed },
            { 600.f, 230.f, DropType::Gem },
            { 300.f, 120.f, DropType::BottleGreen },
            { 500.f, 120.f, DropType::Gem },
        };
        int spawnCount = (int)(sizeof(spawns) / sizeof(spawns[0]));
        for (int i = 0; i < spawnCount && m_dropCount < MAX_DROPS; i++)
        {
            m_dropItems[m_dropCount] = DropItem(spawns[i].x, spawns[i].y, spawns[i].type);
            m_dropItems[m_dropCount].setLifetime(60.f);
            m_dropCount++;
        }
    }

    if (levelNumber == m_starLevelNumber && !m_starCollected)
    {
        m_starX = 400.f;
        m_starY = 480.f;
        m_starActive = true;
        m_starSpawned = true;
    }

    m_backgroundLoaded = m_backgroundTex.loadFromFile(cfg.backgroundPath);
    if (m_backgroundLoaded)
    {
        m_backgroundSprite.setTexture(m_backgroundTex);
        m_backgroundSprite.setScale(
            WINDOW_WIDTH / static_cast<float>(m_backgroundTex.getSize().x),
            WINDOW_HEIGHT / static_cast<float>(m_backgroundTex.getSize().y));
    }
}

void LevelManager::buildPlatformRectCache()
{
    for (int i = 0; i < m_platformCount; i++)
    {
        m_platRects[i] = m_platforms[i].getRect();
        m_platformPtrs[i] = &m_platforms[i];
    }
}

Enemy* LevelManager::spawnEnemy(const EnemySpawn& es)
{
    if (streq(es.enemyType, "FlyingEnemy"))
        return new FlyingEnemy(es.x, es.y, es.variant);
    if (streq(es.enemyType, "Tornado"))
        return new Tornado(es.x, es.y, es.variant);
    if (streq(es.enemyType, "Mogera"))
        return new Mogera(es.x, es.y);
    if (streq(es.enemyType, "Gamakichi"))
        return new Gamakichi(es.x, es.y);
    return new Botom(es.x, es.y, es.variant);
}

// =========================================================
void LevelManager::update(float dt, Player** players, int playerCount)
{
    for (int i = 0; i < m_enemyCount; i++)
    {
        // ── Tornado ───────────────────────────────────────
        if (auto* t = dynamic_cast<Tornado*>(m_enemies[i]))
        {
            if (playerCount > 0)
            {
                sf::Vector2f best = players[0]->getPosition();
                if (playerCount > 1)
                {
                    sf::Vector2f p0 = players[0]->getPosition();
                    sf::Vector2f p1 = players[1]->getPosition();
                    sf::Vector2f tp = t->getPosition();
                    float d0 = (p0.x - tp.x) * (p0.x - tp.x) + (p0.y - tp.y) * (p0.y - tp.y);
                    float d1 = (p1.x - tp.x) * (p1.x - tp.x) + (p1.y - tp.y) * (p1.y - tp.y);
                    best = (d1 < d0) ? p1 : p0;
                }
                t->setPlayerPosition(best);
            }
            m_enemies[i]->update(dt);

            if (t->shouldFireKnife() && m_knifeCount < MAX_KNIVES)
            {
                sf::Vector2f pos = t->getPosition();
                sf::Vector2f dir = t->getKnifeDirection();
                m_knives[m_knifeCount++] = KnifeProjectile(pos.x, pos.y, dir);
            }
        }
        // ── Mogera ────────────────────────────────────────
        else if (auto* mg = dynamic_cast<Mogera*>(m_enemies[i]))
        {
            mg->setPlayerRef(players, playerCount);
            mg->update(dt);

            while (mg->hasChildToSpawn() && m_enemyCount < MAX_ENEMIES)
            {
                MogeraChild* child = mg->popChild();
                if (child) m_enemies[m_enemyCount++] = child;
            }
        }
        // ── Gamakichi boss ────────────────────────────────
        // NOTE: GamakichiChild is checked FIRST (before Gamakichi)
        // because GamakichiChild is also in the enemy array and we
        // must not double-dispatch it through this boss branch.
        else if (auto* gc = dynamic_cast<GamakichiChild*>(m_enemies[i]))
        {
            // GamakichiChild manages its own Flying→Walking state.
            // resolvePlatformCollision and tryJump are called INTERNALLY
            // inside updateWalking() — do NOT call them here again.
            gc->update(dt);
        }
        else if (auto* gk = dynamic_cast<Gamakichi*>(m_enemies[i]))
        {
            gk->setPlayerRef(players, playerCount);
            gk->update(dt);

            // Shells have no blast — shellHitsPlayer always false.
            // Kept here in case you later re-enable shell damage.
            for (int p = 0; p < playerCount; p++)
            {
                if (!players[p]->isInvincible() &&
                    players[p]->isAlive() &&
                    gk->shellHitsPlayer(*players[p]))
                {
                    players[p]->takeDamage();
                }
            }

            // Pop newly thrown dark bombs from the boss queue and
            // insert them into the main enemy array so they get
            // updated, drawn, and collided with like any enemy.
            while (gk->hasPendingChild() && m_enemyCount < MAX_ENEMIES)
            {
                GamakichiChild* child = gk->popPendingChild();
                if (child)
                {
                    // Give each bomb the platform list so it can
                    // detect landing and do platform collision.
                    child->setPlatforms(m_platformPtrs, m_platformCount);
                    child->setPlayerRef(players, playerCount);
                    m_enemies[m_enemyCount++] = child;
                }
            }
        }
        // ── All other enemies (Botom, MogeraChild, FlyingEnemy…) ──
        else
        {
            m_enemies[i]->update(dt);
        }
    }

    // ---- Update snowballs ----
    for (int i = 0; i < m_snowballCount; i++)
        m_snowballs[i]->update(dt);

    // ---- Update knives ----
    for (int i = 0; i < m_knifeCount; i++)
        m_knives[i].update(dt);

    // ---- Update drops ----
    for (int i = 0; i < m_dropCount; i++)
        m_dropItems[i].update(dt);

    spawnDropsFromDeadEnemies();

    // ---- Remove inactive enemies ----
    for (int i = m_enemyCount - 1; i >= 0; i--)
    {
        if (!m_enemies[i]->isActive())
        {
            delete m_enemies[i];
            m_enemies[i] = m_enemies[--m_enemyCount];
            m_enemies[m_enemyCount] = nullptr;
        }
    }

    // ---- Remove inactive snowballs ----
    for (int i = m_snowballCount - 1; i >= 0; i--)
    {
        if (!m_snowballs[i]->isActive())
        {
            delete m_snowballs[i];
            m_snowballs[i] = m_snowballs[--m_snowballCount];
            m_snowballs[m_snowballCount] = nullptr;
        }
    }

    // ---- Remove inactive knives ----
    for (int i = m_knifeCount - 1; i >= 0; i--)
    {
        if (!m_knives[i].isActive())
            m_knives[i] = m_knives[--m_knifeCount];
    }

    // ---- Remove inactive drops ----
    for (int i = m_dropCount - 1; i >= 0; i--)
    {
        if (!m_dropItems[i].isActive())
            m_dropItems[i] = m_dropItems[--m_dropCount];
    }

    if (m_bonusRainActive)
        updateBonusRain(dt, players, playerCount);
    else if (isLevelComplete() &&
        m_currentLevel <= m_levelCount &&
        m_levelRegistry[m_currentLevel - 1].isBonusLevel)
        m_bonusRainActive = true;

    if (m_starActive && !m_starCollected && !m_starChoiceOpen)
    {
        m_starBobTimer += dt;
        checkStarCollision(players, playerCount);
    }
}

// =========================================================
void LevelManager::resolveCollisions(Player** players, int playerCount)
{
    // ---- Platform collision for enemies ----
    for (int i = 0; i < m_enemyCount; i++)
    {
        // ── MogeraChild — MUST come before Botom check ───
        if (auto* mc = dynamic_cast<MogeraChild*>(m_enemies[i]))
        {
            mc->resolvePlatformCollision(m_platRects, m_platformCount);
            mc->tryJump();
        }
        
        else if (auto* gc = dynamic_cast<GamakichiChild*>(m_enemies[i]))
        {
            if (!gc->isActive()) continue;

            // Player stomp on dark bomb (jump on head)
            for (int p = 0; p < playerCount; p++)
            {
                if (!players[p] || players[p]->isInvincible()) continue;

                sf::FloatRect pr = players[p]->getHitBox().getRect();
                sf::FloatRect gcr = gc->getHitBox().getRect();

                float playerBottom = pr.top + pr.height;
                float bombTop = gcr.top;

                // Player feet crossing into top third of bomb + player falling
                bool stompX = (pr.left + pr.width > gcr.left) &&
                    (pr.left < gcr.left + gcr.width);
                bool stompY = (playerBottom >= bombTop) &&
                    (playerBottom <= bombTop + gcr.height * 0.38f);
                bool falling = (players[p]->getVelocityY() > 0.f);

                if (stompX && stompY && falling)
                {
                    gc->takeHit();
                    players[p]->bounce();   // give player a small upward bounce
                }
            }
        }
        // ── Botom ─────────────────────────────────────────
        else if (auto* b = dynamic_cast<Botom*>(m_enemies[i]))
        {
            b->resolvePlatformCollision(m_platRects, m_platformCount);
            b->tryJump();
        }
        // Mogera, Gamakichi, FlyingEnemy — no platform collision needed
    }

    // ---- Platform collision for players ----
    for (int p = 0; p < playerCount; p++)
        players[p]->resolvePlatformCollision(m_platforms, m_platformCount);

    // ---- Snowball off-screen + platform ----
    for (int i = 0; i < m_snowballCount; i++)
    {
        m_snowballs[i]->resolvePlatformCollision(m_platRects, m_platformCount);
        sf::FloatRect r = m_snowballs[i]->getHitBox().getRect();
        if (r.left + r.width < 0.f || r.left > WINDOW_WIDTH)
            m_snowballs[i]->setActive(false);
    }

    handleSnowballEnemyCollision();

    for (int p = 0; p < playerCount; p++)
    {
        handlePlayerSnowballKick(*players[p]);
        handlePlayerEnemyCollision(*players[p]);
        handlePlayerPowerUpCollision(*players[p]);
        handlePlayerDropCollision(*players[p]);
    }

    handleKnifePlayerCollision(players, playerCount);
    handleGamakichiChildSpitCollision(players, playerCount);
}

// =========================================================
void LevelManager::handleKnifePlayerCollision(Player** players, int playerCount)
{
    for (int k = 0; k < m_knifeCount; k++)
    {
        if (!m_knives[k].isActive()) continue;
        sf::FloatRect kr = m_knives[k].getRect();

        for (int p = 0; p < playerCount; p++)
        {
            if (players[p]->isInvincible()) continue;
            if (players[p]->getHitBox().getRect().intersects(kr))
            {
                players[p]->takeDamage();
                m_knives[k].setActive(false);
                break;
            }
        }
    }
}


void LevelManager::handleGamakichiChildSpitCollision(Player** players, int playerCount)
{
    for (int e = 0; e < m_enemyCount; e++)
    {
        auto* child = dynamic_cast<GamakichiChild*>(m_enemies[e]);
        if (!child || !child->isActive()) continue;

        // Only walking bombs fire spits
        if (!child->isWalking()) continue;

        for (int p = 0; p < playerCount; p++)
        {
            if (!players[p]->isAlive() || players[p]->isInvincible()) continue;
            if (child->spitHitsPlayer(*players[p]))
                players[p]->takeDamage();
        }
    }
}

// =========================================================
void LevelManager::handleSnowballEnemyCollision()
{
    for (int s = 0; s < m_snowballCount; s++)
    {
        if (!m_snowballs[s]->isActive()) continue;

        for (int e = 0; e < m_enemyCount; e++)
        {
            if (!m_enemies[e]->isActive()) continue;

            if (m_snowballs[s]->getHitBox().intersects(m_enemies[e]->getHitBox()))
            {
                if (m_snowballs[s]->isRolling())
                {
                    m_enemies[e]->onChainKill();
                    m_snowballs[s]->addChainKill();
                }
                else
                {
                    bool encased = m_enemies[e]->takeHit();
                    if (encased)
                        m_snowballs[s]->setEncased();
                    else
                        m_snowballs[s]->onHit();
                }
                break;
            }
        }
    }
}

// =========================================================
void LevelManager::handlePlayerSnowballKick(Player& player)
{
    for (int s = 0; s < m_snowballCount; s++)
    {
        if (!m_snowballs[s]->isActive()) continue;
        if (m_snowballs[s]->getState() != SnowballState::Encased) continue;

        if (player.getHitBox().intersects(m_snowballs[s]->getHitBox()))
        {
            sf::Vector2f dir = player.isFacingRight()
                ? sf::Vector2f(1.f, 0.f)
                : sf::Vector2f(-1.f, 0.f);
            m_snowballs[s]->startRolling(dir);
        }
    }
}

// =========================================================
void LevelManager::handlePlayerEnemyCollision(Player& player)
{
    if (player.isInvincible()) return;
    for (int i = 0; i < m_enemyCount; i++)
    {
        if (!m_enemies[i]->isActive() || m_enemies[i]->isEncased()) continue;

        // Gamakichi boss body contact does no damage — shells only
        if (dynamic_cast<Gamakichi*>(m_enemies[i])) continue;

       

        if (player.getHitBox().intersects(m_enemies[i]->getHitBox()))
        {
            player.takeDamage();
            break;
        }
    }
}

// =========================================================
void LevelManager::handlePlayerPowerUpCollision(Player& /*player*/) {}

// =========================================================
void LevelManager::handlePlayerDropCollision(Player& player)
{
    sf::FloatRect pRect = player.getHitBox().getRect();
    for (int i = 0; i < m_dropCount; i++)
    {
        if (!m_dropItems[i].isActive()) continue;
        if (!m_dropItems[i].getRect().intersects(pRect)) continue;

        int score = m_dropItems[i].collect();
        player.addScore(score);

        switch (m_dropItems[i].getType())
        {
        case DropType::BottleRed:    player.addLife();               break;
        case DropType::BottleBlue:   player.applySpeedBoost();       break;
        case DropType::BottleYellow: player.applySnowballPower();    break;
        case DropType::BottleGreen:  player.applyDistanceIncrease(); break;
        case DropType::Gem:          player.addGems(5);              break;
        case DropType::FaceBlue:     player.addGems(20);             break;
        case DropType::FaceRed:      player.addGems(50);             break;
        default: break;
        }
    }
}

// =========================================================
void LevelManager::spawnDropsFromDeadEnemies()
{
    for (int i = 0; i < m_enemyCount; i++)
    {
        if (m_enemies[i]->isActive()) continue;

        // ── GamakichiChild gem drop ───────────────────────
        // Must be checked BEFORE Botom (GamakichiChild is not a Botom).
        if (auto* gc = dynamic_cast<GamakichiChild*>(m_enemies[i]))
        {
            if (gc->hasDrop() && m_dropCount < MAX_DROPS)
            {
                m_dropItems[m_dropCount++] = DropItem(
                    gc->getPosition().x + 18.f,
                    gc->getPosition().y + 18.f,
                    gc->getDropType());
                gc->clearDrop();
            }
        }
        // ── MogeraChild — MUST come before Botom check ───
        else if (auto* mc = dynamic_cast<MogeraChild*>(m_enemies[i]))
        {
            if (mc->hasDrop() && m_dropCount < MAX_DROPS)
            {
                m_dropItems[m_dropCount++] = DropItem(
                    mc->getPosition().x,
                    mc->getPosition().y,
                    mc->getRandomDrop());
                mc->clearDrop();
            }
        }
        // ── Regular Botom ─────────────────────────────────
        else if (auto* b = dynamic_cast<Botom*>(m_enemies[i]))
        {
            if (b->hasDrop() && m_dropCount < MAX_DROPS)
            {
                m_dropItems[m_dropCount++] = DropItem(
                    b->getPosition().x,
                    b->getPosition().y,
                    b->getRandomDrop());
                b->clearDrop();
            }
        }
        // ── Mogera boss reward ────────────────────────────
        else if (auto* mg = dynamic_cast<Mogera*>(m_enemies[i]))
        {
            if (mg->hasDrop())
            {
                if (m_dropCount < MAX_DROPS)
                    m_dropItems[m_dropCount++] = DropItem(
                        mg->getPosition().x + 32.f,
                        mg->getPosition().y + 32.f,
                        mg->getDropType());

                for (int g = 0; g < 40 && m_dropCount < MAX_DROPS; g++)
                {
                    float ox = static_cast<float>(rand() % 160) - 80.f;
                    float oy = static_cast<float>(rand() % 60);
                    m_dropItems[m_dropCount++] = DropItem(
                        mg->getPosition().x + ox,
                        mg->getPosition().y + 20.f + oy,
                        DropType::Gem);
                }
                mg->clearDrop();
            }
        }
        else if (auto* gk = dynamic_cast<Gamakichi*>(m_enemies[i]))
        {
            if (gk->hasDrop())
            {
                if (m_dropCount < MAX_DROPS)
                    m_dropItems[m_dropCount++] = DropItem(
                        gk->getPosition().x + 48.f,
                        gk->getPosition().y + 48.f,
                        gk->getDropType());

                for (int g = 0; g < 100 && m_dropCount < MAX_DROPS; g++)
                {
                    float ox = static_cast<float>(rand() % 200) - 100.f;
                    float oy = static_cast<float>(rand() % 80);
                    m_dropItems[m_dropCount++] = DropItem(
                        gk->getPosition().x + ox,
                        gk->getPosition().y + 20.f + oy,
                        DropType::Gem);
                }
                gk->clearDrop();
            }
        }
    }
}

// =========================================================
void LevelManager::addSnowball(Snowball* sb)
{
    if (m_snowballCount < MAX_SNOWBALLS)
        m_snowballs[m_snowballCount++] = sb;
    else
        delete sb;
}

// =========================================================
void LevelManager::draw(sf::RenderWindow& window, bool debugMode,
    Player** players, int playerCount)
{
    if (m_backgroundLoaded)
        window.draw(m_backgroundSprite);
    else
    {
        sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        bg.setFillColor(m_levelRegistry[m_currentLevel - 1].backgroundColor);
        window.draw(bg);
    }

    for (int i = 0; i < m_platformCount; i++)
    {
        m_platforms[i].draw(window);
        if (debugMode) m_platforms[i].drawDebug(window);
    }

    for (int i = 0; i < m_enemyCount; i++)
    {
        m_enemies[i]->draw(window);
        if (debugMode) m_enemies[i]->drawDebug(window);
    }

    for (int i = 0; i < m_snowballCount; i++)
        m_snowballs[i]->draw(window);

    for (int i = 0; i < m_knifeCount; i++)
        m_knives[i].draw(window);

    for (int i = 0; i < m_dropCount; i++)
        m_dropItems[i].draw(window);

    if (m_starActive && !m_starCollected && !m_starChoiceOpen)
        drawStarIcon(window);
}

// =========================================================
bool LevelManager::isLevelComplete() const
{
    if (m_starChoiceOpen) return false;

    if (m_currentLevel == 4 || m_currentLevel == 9)
        return (m_dropCount == 0);

    return m_enemyCount == 0;
}

// =========================================================
void LevelManager::resetLevelPowerUps(Player** players, int count)
{
    for (int i = 0; i < count; i++)
        players[i]->resetLevelPowerUps();
}

// =========================================================
void LevelManager::updateBonusRain(float dt, Player** players, int playerCount)
{
    m_bonusRainTimer += dt;
    if (m_bonusRainTimer < 5.f &&
        static_cast<int>(m_bonusRainTimer * 10) % 3 == 0)
    {
        for (int p = 0; p < playerCount; p++)
        {
            players[p]->addScore(1000);
            players[p]->addGems(10);
        }
    }
    if (m_bonusRainTimer >= 5.f) m_bonusRainActive = false;
}

// =========================================================
// Star Bonus helpers
// =========================================================

void LevelManager::checkStarCollision(Player** players, int playerCount)
{
    float starSize = 28.f;
    sf::FloatRect starRect(
        m_starX - starSize * 0.5f,
        m_starY - starSize * 0.5f + sinf(m_starBobTimer * 3.f) * 8.f,
        starSize, starSize);

    for (int p = 0; p < playerCount; p++)
    {
        if (!players[p]->isAlive()) continue;
        if (players[p]->getHitBox().getRect().intersects(starRect))
        {
            m_starCollected = true;
            m_starActive = false;
            pickStarChoices();
            m_starChoiceOpen = true;
            return;
        }
    }
}

void LevelManager::pickStarChoices()
{
    PowerUpType all[] = {
        PowerUpType::SpeedBoost,
        PowerUpType::SnowballPower,
        PowerUpType::DistanceIncrease,
        PowerUpType::BalloonMode,
        PowerUpType::ExtraLife
    };
    int order[5] = { 0, 1, 2, 3, 4 };
    for (int i = 4; i > 0; i--)
    {
        int j = rand() % (i + 1);
        int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
    }
    for (int i = 0; i < 3; i++)
        m_starChoices[i] = all[order[i]];
}

static const char* powerUpName(PowerUpType t)
{
    switch (t)
    {
    case PowerUpType::SpeedBoost:       return "Speed Boost";
    case PowerUpType::SnowballPower:    return "Snowball Power";
    case PowerUpType::DistanceIncrease: return "Distance Boost";
    case PowerUpType::BalloonMode:      return "Balloon Mode";
    case PowerUpType::ExtraLife:        return "Extra Life";
    default:                            return "Power-Up";
    }
}

static const char* powerUpDesc(PowerUpType t)
{
    switch (t)
    {
    case PowerUpType::SpeedBoost:       return "Move 50% faster!";
    case PowerUpType::SnowballPower:    return "Bigger snowballs!";
    case PowerUpType::DistanceIncrease: return "Throw farther!";
    case PowerUpType::BalloonMode:      return "Float in the air!";
    case PowerUpType::ExtraLife:        return "+1 Life!";
    default:                            return "";
    }
}

void LevelManager::drawStarIcon(sf::RenderWindow& window)
{
    float bobY = m_starY + sinf(m_starBobTimer * 3.f) * 8.f;

    sf::CircleShape glow(20.f);
    glow.setOrigin(20.f, 20.f);
    glow.setPosition(m_starX, bobY);
    glow.setFillColor(sf::Color(255, 255, 0, 60));
    glow.setOutlineColor(sf::Color(255, 220, 0, 180));
    glow.setOutlineThickness(3.f);
    window.draw(glow);

    sf::CircleShape star(14.f, 5);
    star.setOrigin(14.f, 14.f);
    star.setPosition(m_starX, bobY);
    star.setFillColor(sf::Color(255, 230, 0));
    star.setOutlineColor(sf::Color(255, 150, 0));
    star.setOutlineThickness(2.f);
    star.setRotation(-18.f);
    window.draw(star);
}

void LevelManager::drawStarChoiceUI(sf::RenderWindow& window, const sf::Font& font)
{
    if (!m_starChoiceOpen) return;

    sf::RectangleShape overlay(sf::Vector2f(800.f, 600.f));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    float panW = 680.f, panH = 300.f;
    float panX = (800.f - panW) * 0.5f;
    float panY = (600.f - panH) * 0.5f;

    sf::RectangleShape panel(sf::Vector2f(panW, panH));
    panel.setPosition(panX, panY);
    panel.setFillColor(sf::Color(20, 10, 50, 240));
    panel.setOutlineColor(sf::Color(255, 220, 0));
    panel.setOutlineThickness(3.f);
    window.draw(panel);

    sf::Text title;
    title.setFont(font);
    title.setString("★  STAR BONUS  ★  Choose a Power-Up!");
    title.setCharacterSize(22);
    title.setFillColor(sf::Color(255, 230, 0));
    sf::FloatRect tb = title.getLocalBounds();
    title.setOrigin(tb.width * 0.5f, tb.height * 0.5f);
    title.setPosition(400.f, panY + 28.f);
    window.draw(title);

    float cardW = 180.f, cardH = 160.f, spacing = 20.f;
    float totalW = 3.f * cardW + 2.f * spacing;
    float startX = (800.f - totalW) * 0.5f;
    float cardY = panY + 70.f;

    static const sf::Color cardColors[3] = {
        sf::Color(255, 200, 0),
        sf::Color(100, 200, 255),
        sf::Color(100, 255, 150)
    };

    for (int i = 0; i < 3; i++)
    {
        float cx = startX + i * (cardW + spacing);

        sf::RectangleShape card(sf::Vector2f(cardW, cardH));
        card.setPosition(cx, cardY);
        card.setFillColor(sf::Color(30, 20, 70, 230));
        card.setOutlineColor(cardColors[i]);
        card.setOutlineThickness(2.f);
        window.draw(card);

        sf::CircleShape icon(24.f);
        icon.setOrigin(24.f, 24.f);
        icon.setPosition(cx + cardW * 0.5f, cardY + 40.f);
        icon.setFillColor(cardColors[i]);
        icon.setOutlineColor(sf::Color::White);
        icon.setOutlineThickness(2.f);
        window.draw(icon);

        sf::Text nameText;
        nameText.setFont(font);
        nameText.setString(powerUpName(m_starChoices[i]));
        nameText.setCharacterSize(14);
        nameText.setFillColor(sf::Color::White);
        sf::FloatRect nb = nameText.getLocalBounds();
        nameText.setOrigin(nb.width * 0.5f, nb.height * 0.5f);
        nameText.setPosition(cx + cardW * 0.5f, cardY + 84.f);
        window.draw(nameText);

        sf::Text desc;
        desc.setFont(font);
        desc.setString(powerUpDesc(m_starChoices[i]));
        desc.setCharacterSize(12);
        desc.setFillColor(sf::Color(200, 200, 200));
        sf::FloatRect db = desc.getLocalBounds();
        desc.setOrigin(db.width * 0.5f, db.height * 0.5f);
        desc.setPosition(cx + cardW * 0.5f, cardY + 106.f);
        window.draw(desc);

        char keyHint[4] = { '[', (char)('1' + i), ']', '\0' };
        sf::Text key;
        key.setFont(font);
        key.setString(keyHint);
        key.setCharacterSize(16);
        key.setFillColor(cardColors[i]);
        sf::FloatRect kb = key.getLocalBounds();
        key.setOrigin(kb.width * 0.5f, kb.height * 0.5f);
        key.setPosition(cx + cardW * 0.5f, cardY + cardH - 22.f);
        window.draw(key);
    }

    sf::Text hint;
    hint.setFont(font);
    hint.setString("Press 1, 2 or 3  —  or click a card");
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(180, 180, 180));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.width * 0.5f, hb.height * 0.5f);
    hint.setPosition(400.f, panY + panH - 18.f);
    window.draw(hint);
}

void LevelManager::handleStarChoice(int idx, Player** players, int playerCount)
{
    if (!m_starChoiceOpen || idx < 0 || idx > 2) return;
    m_starChoiceOpen = false;

    PowerUpType chosen = m_starChoices[idx];
    for (int p = 0; p < playerCount; p++)
    {
        if (!players[p]->isAlive()) continue;
        switch (chosen)
        {
        case PowerUpType::SpeedBoost:       players[p]->applySpeedBoost();       break;
        case PowerUpType::SnowballPower:    players[p]->applySnowballPower();    break;
        case PowerUpType::DistanceIncrease: players[p]->applyDistanceIncrease(); break;
        case PowerUpType::BalloonMode:      players[p]->applyBalloonMode();      break;
        case PowerUpType::ExtraLife:        players[p]->addLife();               break;
        default: break;
        }
    }
}