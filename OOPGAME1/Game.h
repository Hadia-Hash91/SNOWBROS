#pragma once
// =========================================================
// Game.h  —  with IntroVideo / OutroVideo states
// =========================================================

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <ctime>

#include "Player.h"
#include "LevelManager.h"
#include "HUD.h"
#include "CharacterSelect.h"
#include "Shop.h"
#include "Settings.h"

// =========================================================
// GameState
// =========================================================
enum class GameState
{
    Splash,
    AuthSelect,
    MainMenu,
    Login,
    SignUp,
    LevelSelect,
    DailyReward,
    AboutUs,
    CharacterSelect,
    IntroVideo,     // plays after character select 
    OutroVideo,     // plays after level 10 cleared
    Playing,
    Paused,
    Shop,
    Settings,
    SettingsPause,
    LevelComplete,
    BossDefeated,
    GameOver,
    Victory,
    Leaderboard
};

// Leaderboard entry
static const int MAX_LEADERBOARD_ENTRIES = 20;
struct LeaderboardEntry
{
    char  name[32];
    int   score;
    int   gems;
    int   level;
    char  date[32];

    LeaderboardEntry() : score(0), gems(0), level(1)
    {
        name[0] = '\0';
        date[0] = '\0';
    }
};


struct GameSaveSlot
{
    bool  valid = false;
    int   level = 1;
    int   playerCount = 0;
    Player::SaveData playerData[MAX_PLAYERS] = {};
    int   charIndex[MAX_PLAYERS] = { 0, 1 };
};


struct VideoPlayer
{
    std::vector<sf::Texture> frames;
    sf::Sprite               sprite;
    int                      current = 0;
    float                    fps = 24.f;
    float                    timer = 0.f;
    bool                     finished = false;

    bool load(const std::string& folder, int frameCount)
    {
        frames.clear();
        current = 0;
        timer = 0.f;
        finished = false;

        frames.reserve(frameCount);
        for (int i = 1; i <= frameCount; ++i)
        {
            char buf[256];
            snprintf(buf, sizeof(buf), "%s/frame_%04d.png", folder.c_str(), i);
            sf::Texture tex;
            if (!tex.loadFromFile(buf)) { frames.clear(); return false; }
            frames.push_back(std::move(tex));
        }
        if (!frames.empty())
            sprite.setTexture(frames[0]);
        return !frames.empty();
    }

    void update(float dt, float winW, float winH)
    {
        if (finished || frames.empty()) return;
        timer += dt;
        while (timer >= 1.f / fps)
        {
            timer -= 1.f / fps;
            ++current;
            if (current >= static_cast<int>(frames.size()))
            {
                current = static_cast<int>(frames.size()) - 1;
                finished = true;
                break;
            }
        }
        sprite.setTexture(frames[current]);
        auto sz = frames[current].getSize();
        if (sz.x > 0 && sz.y > 0)
            sprite.setScale(winW / sz.x, winH / sz.y);
    }

    void reset()
    {
        current = 0;
        timer = 0.f;
        finished = false;
        if (!frames.empty())
            sprite.setTexture(frames[0]);
    }

    bool isFinished() const { return finished; }
};

class Game
{
public:
    Game();
    ~Game();
    void run();

private:
    
    sf::RenderWindow m_window;
    sf::Clock        m_clock;
    sf::Font         m_font;
    sf::Music        m_music;
    sf::Music        m_sfx;
    std::string      m_currentBgmPath;

    
    // Start-click transition
    float            m_startClickTimer = 0.f;
    bool             m_startClickPlaying = false;

    // Audio state tracker
    GameState        m_prevAudioState = GameState::Splash;
    bool             m_endSoundPlayed = false;

    // Menu background
    sf::Texture      m_menuBgTexture;
    sf::Sprite       m_menuBgSprite;
    sf::Text         m_menuTitle;
    float            m_splashTimer = 0.f;
    sf::Texture      m_splashTexture;
    sf::Sprite       m_splashSprite;
    bool             m_splashButtonHovered = false;
    float            m_splashButtonScale = 1.f;

    // ----------------------------------------------------------
    // Auth-select screen
    // ----------------------------------------------------------
    sf::Texture      m_authSelectBgTexture;
    sf::Sprite       m_authSelectBgSprite;
    sf::Texture      m_authSelectCardTexture;
    sf::Sprite       m_authSelectCardSprite;
    int              m_authSelectSelection = 0;
    bool             m_authSelectHovered[3] = { false, false, false };

    // ----------------------------------------------------------
    // Login / Signup / misc background images
    // ----------------------------------------------------------
    sf::Texture      m_loginBgTexture;
    sf::Sprite       m_loginBgSprite;
    sf::Texture      m_loginCardTexture;
    sf::Sprite       m_loginCardSprite;
    sf::Texture      m_signupBgTexture;
    sf::Sprite       m_signupBgSprite;
    sf::Texture      m_levelSelectBgTexture;
    sf::Sprite       m_levelSelectBgSprite;
    sf::Texture      m_leaderboardBgTexture;
    sf::Sprite       m_leaderboardBgSprite;

    // ----------------------------------------------------------
    // Game Over / Victory backgrounds
    // ----------------------------------------------------------
    sf::Texture      m_gameOverBgTexture;
    sf::Sprite       m_gameOverBgSprite;
    sf::Texture      m_victoryBgTexture;
    sf::Sprite       m_victoryBgSprite;

    // Video players  
    VideoPlayer      m_introVideo;          // after character select
    VideoPlayer      m_outroVideo;          // after level 10
    sf::Music        m_videoMusic;          // optional audio track for videos
    GameState        m_videoReturnState = GameState::Playing;
    float            m_videoTimer = 0.f;   // max duration guard
    static const int VIDEO_MAX_SECONDS = 35;    // hard cap

    // Main menu hit-zones
    sf::RectangleShape m_centreButtons[3];

    static const int   MAIN_MENU_ITEMS = 7;
    sf::Text           m_menuItems[MAIN_MENU_ITEMS];
    sf::RectangleShape m_selectorBox;

    // State
    int       m_menuSelection = 0;
    GameState m_state = GameState::Splash;
    bool      m_debugMode = false;

    // ----------------------------------------------------------
    // Sub-systems
    // ----------------------------------------------------------
    LevelManager    m_levelManager;
    HUD             m_hud;
    CharacterSelect m_charSelect;
    Shop            m_shop;
    Settings        m_settings;

    // ----------------------------------------------------------
    // Players
    // ----------------------------------------------------------
    Player* m_players[MAX_PLAYERS];
    int     m_playerCount = 0;
    int     m_pendingPlayerCount = 1;
    int     m_selectedCharIdx[MAX_PLAYERS] = { 0, 1 };

    // ----------------------------------------------------------
    // Return-state helpers
    // ----------------------------------------------------------
    GameState m_shopReturnState = GameState::MainMenu;
    GameState m_settingsReturnState = GameState::MainMenu;

    // ----------------------------------------------------------
    // Pause menu
    // ----------------------------------------------------------
    static const int PAUSE_MENU_ITEMS = 6;
    sf::Text         m_pauseItems[PAUSE_MENU_ITEMS];
    int              m_pauseSelection = 0;

    // ----------------------------------------------------------
    // Save slot
    // ----------------------------------------------------------
    GameSaveSlot m_saveSlot;
    char         m_saveMsg[64] = "";
    float        m_saveMsgTimer = 0.f;

    // ----------------------------------------------------------
    // Level selection
    // ----------------------------------------------------------
    int  m_levelSelectChoice = 1;

    // ----------------------------------------------------------
    // How To Play overlay
    // ----------------------------------------------------------
    bool m_howToPlayOpen = false;
    int  m_howToPlayPage = 0;

    // ----------------------------------------------------------
    // Persistent leaderboard
    // ----------------------------------------------------------
    LeaderboardEntry m_leaderboard[MAX_LEADERBOARD_ENTRIES];
    int              m_leaderboardCount = 0;

    // ----------------------------------------------------------
    // Login / Sign-Up
    // ----------------------------------------------------------
    std::string m_authField[4];
    int         m_authActiveField = 0;
    std::string m_authError;
    bool        m_loggedIn = false;
    std::string m_loggedInUser;
    bool        m_enteredAsGuest = false;
    bool        m_authShowPassword[2] = { false, false };
    float       m_authCursorTimer = 0.f;
    bool        m_authCursorVisible = true;

    // ----------------------------------------------------------
    // Daily reward
    // ----------------------------------------------------------
    bool m_dailyRewardClaimed = false;

    // ----------------------------------------------------------
    // Core loop
    // ----------------------------------------------------------
    void processEvents();
    void update(float dt);
    void render();

    // ----------------------------------------------------------
    // Update helpers
    // ----------------------------------------------------------
    void updatePlaying(float dt);
    void updateLevelComplete(float dt);
    void updateBgmForState();

    // ----------------------------------------------------------
    // Video helpers  ← NEW
    // ----------------------------------------------------------
    void playIntroVideo();
    void playOutroVideo();
    void stopVideo();

    // ----------------------------------------------------------
    // Audio helpers
    // ----------------------------------------------------------
    void playBgm(const std::string& path, bool loop = true);
    void playSfx(const std::string& path);
    void stopBgm();

    // ----------------------------------------------------------
    // Draw functions
    // ----------------------------------------------------------
    void drawSplash();
    void drawAuthSelect();
    void drawAuthScreen(bool isSignUp);
    void drawMainMenu();
    void drawPlaying();
    void drawStatsPanel();
    void drawPaused();
    void drawLevelComplete();
    void drawGameOver();
    void drawVictory();
    void drawLeaderboard();
    void drawDailyReward();
    void drawAboutUs();
    void drawLevelSelect();
    void drawHowToPlay();
    void drawVideo(VideoPlayer& vp, const std::string& skipHint); // ← NEW
    void handleStarChoiceInput(const sf::Event& event);

    // ----------------------------------------------------------
    // Menu / screen event handlers
    // ----------------------------------------------------------
    void handleMainMenuEvents(const sf::Event& event, float sx, float sy);
    void handleNewMenuConfirm(int idx);
    void handlePauseMenuConfirm();
    void handleLevelSelectEvents(const sf::Event& event);
    void handleAuthSelectEvents(const sf::Event& event);
    void handleAuthEvents(const sf::Event& event, bool isSignUp);

    // ----------------------------------------------------------
    // Auth helpers
    // ----------------------------------------------------------
    void attemptLogin();
    void attemptSignUp();

    // ----------------------------------------------------------
    // Password hashing
    // ----------------------------------------------------------
    static unsigned int hashPassword(const std::string& pass);
    static std::string  hashToHex(unsigned int h);

    // ----------------------------------------------------------
    // Leaderboard file helpers
    // ----------------------------------------------------------
    void loadLeaderboard();
    void saveLeaderboard();
    void submitScore(const char* name, int score, int gems, int level);

    // ----------------------------------------------------------
    // Daily reward helpers
    // ----------------------------------------------------------
    void loadDailyRewardState();
    void saveDailyRewardState();

    // ----------------------------------------------------------
    // Game flow
    // ----------------------------------------------------------
    void startNewGame(int numPlayers);
    void startFromCharacterSelect();
    void advanceToNextLevel();

    // ----------------------------------------------------------
    // Init
    // ----------------------------------------------------------
    void initMainMenu();
    void initPauseMenu();

    // ----------------------------------------------------------
    // Music helpers
    // ----------------------------------------------------------
    void loadMusic(const char* path);
    void loadLevelMusic(int levelNumber);

    // ----------------------------------------------------------
    // Misc helpers
    // ----------------------------------------------------------
    void clearPlayers();
    void saveGame();
    bool loadGame();
    void saveGameToFile();
    bool loadGameFromFile();
    void applySettingsBindings();
};