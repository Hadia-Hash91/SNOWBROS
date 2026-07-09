#pragma once
// =========================================================
// LevelManager.h — No STL libraries, no memory library
// =========================================================

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

#include "LevelConfig.h"
#include "Platform.h"
#include "Botom.h"
#include "FlyingEnemy.h"
#include "Tornado.h"
#include "Mogera.h"
#include "Gamakichi.h"
#include "Gamakichichild.h"
#include "Snowball.h"
#include "PowerUp.h"
#include "Player.h"
#include "DropItem.h"
#include "KnifeProjectile.h"

static const int MAX_LEVELS = 12;
static const int MAX_PLATFORMS = 20;
static const int MAX_ENEMIES = 40;
static const int MAX_SNOWBALLS = 20;
static const int MAX_POWERUPS = 10;
static const int MAX_DROPS = 40;
static const int MAX_PLAYERS = 2;
static const int MAX_KNIVES = 20;

class LevelManager
{
public:
    LevelManager();
    ~LevelManager();

    void buildLevelRegistry();
    void loadLevel(int levelNumber);

    void update(float dt, Player** players, int playerCount);
    void draw(sf::RenderWindow& window, bool debugMode,
        Player** players, int playerCount);
    void resolveCollisions(Player** players, int playerCount);

    void addSnowball(Snowball* sb);

    bool isLevelComplete() const;
    int  getCurrentLevel() const { return m_currentLevel; }
    int  getTotalLevels()  const { return m_levelCount; }
    int  getStarLevelNumber() const { return m_starLevelNumber; }

    void resetLevelPowerUps(Player** players, int playerCount);

    // --- Star Bonus UI ---
    bool isStarChoiceOpen()   const { return m_starChoiceOpen; }
    void handleStarChoice(int idx, Player** players, int playerCount);
    void drawStarChoiceUI(sf::RenderWindow& window, const sf::Font& font);

private:
    int m_currentLevel = 1;
    int m_levelCount = 0;

    LevelConfig     m_levelRegistry[MAX_LEVELS];

    Platform        m_platforms[MAX_PLATFORMS];
    int             m_platformCount = 0;

    Enemy* m_enemies[MAX_ENEMIES];
    int             m_enemyCount = 0;

    Snowball* m_snowballs[MAX_SNOWBALLS];
    int             m_snowballCount = 0;

    DropItem        m_dropItems[MAX_DROPS];
    int             m_dropCount = 0;

    KnifeProjectile m_knives[MAX_KNIVES];
    int             m_knifeCount = 0;

    sf::Texture     m_backgroundTex;
    sf::Sprite      m_backgroundSprite;
    bool            m_backgroundLoaded = false;

    bool            m_bonusRainActive = false;
    float           m_bonusRainTimer = 0.f;

    sf::FloatRect   m_platRects[MAX_PLATFORMS];
    Platform* m_platformPtrs[MAX_PLATFORMS];   // for GamakichiChild::setPlatforms()

    void   buildPlatformRectCache();
    void   clearEnemies();
    void   clearSnowballs();
    Enemy* spawnEnemy(const EnemySpawn& spawn);

    void handleSnowballEnemyCollision();
    void handlePlayerSnowballKick(Player& player);
    void handlePlayerEnemyCollision(Player& player);
    void handlePlayerPowerUpCollision(Player& player);
    void handlePlayerDropCollision(Player& player);
    void handleKnifePlayerCollision(Player** players, int playerCount);
    void handleGamakichiChildSpitCollision(Player** players, int playerCount);
    void spawnDropsFromDeadEnemies();
    void updateBonusRain(float dt, Player** players, int playerCount);

    // --- Star Bonus internals ---
    int          m_starLevelNumber = -1;    // which level has the star (set at buildLevelRegistry)
    bool         m_starSpawned = false;     // has the star been placed this level?
    bool         m_starCollected = false;   // star already taken
    float        m_starX = 0.f;
    float        m_starY = 0.f;
    float        m_starBobTimer = 0.f;
    bool         m_starActive = false;

    // star-choice UI
    bool         m_starChoiceOpen = false;
    PowerUpType  m_starChoices[3];
    void         pickStarChoices();
    void         drawStarIcon(sf::RenderWindow& window);
    void         checkStarCollision(Player** players, int playerCount);
};