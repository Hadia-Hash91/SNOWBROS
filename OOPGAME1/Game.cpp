#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include "Game.h"
#include "Constants.h"
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdio>
#include <cmath>
using namespace std;

// hashPassword
// =========================================================
unsigned int Game::hashPassword(const std::string& pass)
{
    unsigned int hash = 5381u;
    for (char c : pass)
        hash = ((hash << 5u) + hash) ^ static_cast<unsigned char>(c);
    unsigned int salt = 0xDEADBEEFu;
    hash ^= salt;
    hash += (hash << 13u); hash ^= (hash >> 7u);
    hash += (hash << 3u);  hash ^= (hash >> 17u);
    hash += (hash << 5u);
    return hash;
}

std::string Game::hashToHex(unsigned int h)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%08X", h);
    return std::string(buf);
}


Game::Game()
    : m_window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE)
    , m_hud(m_font)
    , m_charSelect(m_font)
    , m_shop(m_font)
    , m_settings(m_font)
    , m_playerCount(0)
{
    m_window.setFramerateLimit(TARGET_FPS);

    for (int i = 0; i < MAX_PLAYERS; i++) m_players[i] = nullptr;

    if (!m_font.loadFromFile("assets/fonts/game_font.ttf"))
        m_font.loadFromFile("C:/Windows/Fonts/arial.ttf");

    m_levelManager.buildLevelRegistry();

    auto loadImg = [](sf::Texture& tex, sf::Sprite& spr,
        const std::string& filename,
        float winW, float winH) -> bool
        {
            const std::string roots[] = {
                "", "assets/images/", "assets/", "images/", "../", "../../"
            };
            for (const auto& root : roots)
            {
                if (tex.loadFromFile(root + filename))
                {
                    spr.setTexture(tex);
                    spr.setScale(winW / tex.getSize().x, winH / tex.getSize().y);
                    return true;
                }
            }
            cout << "Could not find image : " << filename << endl;
            return false;
        };

    const float W = static_cast<float>(WINDOW_WIDTH);
    const float H = static_cast<float>(WINDOW_HEIGHT);

    loadImg(m_splashTexture, m_splashSprite, "assets/images/POWERPUFF.png", W, H);
    loadImg(m_menuBgTexture, m_menuBgSprite, "assets/images/menu_bg.png", W, H);
    loadImg(m_loginBgTexture, m_loginBgSprite, "assets/images/MAINPAGE.png", W, H);
    loadImg(m_signupBgTexture, m_signupBgSprite, "assets/images/MAINPAGE.png", W, H);
    loadImg(m_levelSelectBgTexture, m_levelSelectBgSprite, "assets/images/levelselection.png", W, H);
    loadImg(m_authSelectBgTexture, m_authSelectBgSprite, "assets/images/MAINPAGE.png", W, H);
    loadImg(m_authSelectCardTexture, m_authSelectCardSprite, "assets/images/LOGINBOX.png", 440.f, 280.f);
    loadImg(m_loginCardTexture, m_loginCardSprite, "assets/images/box.png", 340.f, 270.f);
    m_settings.loadBackground("assets/images/SETTING.png");
    m_charSelect.loadBackground("assets/images/char.png");
    m_charSelect.loadCharacterImages();
    m_shop.loadBackground("assets/images/gem_shop_bg.png");
    loadImg(m_leaderboardBgTexture, m_leaderboardBgSprite, "assets/images/leaderboard_bg.png", W, H);
    loadImg(m_gameOverBgTexture, m_gameOverBgSprite, "gameloss.png", W, H);
    loadImg(m_victoryBgTexture, m_victoryBgSprite, "assets/images/victory.png", W, H);

    const int INTRO_FRAME_COUNT = 496;
    const int OUTRO_FRAME_COUNT = 720;

    if (!m_introVideo.load("assets/video/intro", INTRO_FRAME_COUNT))
        cout << "[VIDEO] Intro frames not found — will skip intro video." << endl;

    if (!m_outroVideo.load("assets/video/outro", OUTRO_FRAME_COUNT))
        cout << "Outro frames not found — will skip outro video." << endl;

    for (int i = 0; i < 4; i++) m_authField[i] = "";
    m_authActiveField = 0;
    m_authError = "";
    m_loggedIn = false;
    m_loggedInUser = "";
    m_enteredAsGuest = false;
    m_authShowPassword[0] = m_authShowPassword[1] = false;
    m_authCursorTimer = 0.f;
    m_authCursorVisible = true;

    m_splashButtonScale = 1.f;
    m_splashButtonHovered = false;

    m_dailyRewardClaimed = false;
    loadDailyRewardState();
    loadLeaderboard();
    loadGameFromFile();
    initMainMenu();
    initPauseMenu();

    playBgm("assets/music/MAINMENU.WAV", true);
}

Game::~Game() { clearPlayers(); }


void Game::playIntroVideo()
{
    if (m_introVideo.frames.empty())
    {
        m_state = GameState::Playing;
        return;
    }
    m_introVideo.reset();
    m_videoTimer = 0.f;
    m_videoReturnState = GameState::Playing;
    stopBgm();

    if (m_videoMusic.openFromFile("assets/music/intro_audio.wav"))
    {
        m_videoMusic.setLoop(false);
        m_videoMusic.setVolume(m_settings.getMusicVolume());
        m_videoMusic.play();
    }

    m_state = GameState::IntroVideo;
}

void Game::playOutroVideo()
{
    if (m_outroVideo.frames.empty())
    {
        m_state = GameState::Victory;
        return;
    }
    m_outroVideo.reset();
    m_videoTimer = 0.f;
    m_videoReturnState = GameState::Victory;
    stopBgm();

    if (m_videoMusic.openFromFile("assets/music/outro_video.WAV"))
    {
        m_videoMusic.setLoop(false);
        m_videoMusic.setVolume(m_settings.getMusicVolume());
        m_videoMusic.play();
    }

    m_state = GameState::OutroVideo;
}

void Game::stopVideo()
{
    m_videoMusic.stop();
}

// Audio helpers
void Game::playBgm(const std::string& path, bool loop)
{
    if (m_currentBgmPath == path && m_music.getStatus() == sf::Music::Playing)
        return;
    m_music.stop();
    if (m_music.openFromFile(path))
    {
        m_currentBgmPath = path;
        m_music.setLoop(loop);
        m_music.setVolume(m_settings.getMusicVolume());
        m_music.play();
    }
    else cout << " Could not open BGM: " << path << endl;
}

void Game::playSfx(const std::string& path)
{
    m_sfx.stop();
    if (m_sfx.openFromFile(path))
    {
        m_sfx.setLoop(false);
        m_sfx.setVolume(m_settings.getMusicVolume());
        m_sfx.play();
    }
    else cout << "[AUDIO] Could not open SFX: " << path << endl;
}

void Game::stopBgm()
{
    m_music.stop();
    m_currentBgmPath.clear();
}

void Game::updateBgmForState()
{
    if (m_state == GameState::Playing ||
        m_state == GameState::Paused ||
        m_state == GameState::LevelComplete)
    {
        m_prevAudioState = m_state;
        return;
    }

    if (m_state == GameState::IntroVideo ||
        m_state == GameState::OutroVideo)
    {
        m_prevAudioState = m_state;
        return;
    }

    if (m_startClickPlaying) return;

    if (m_state == m_prevAudioState &&
        m_music.getStatus() == sf::Music::Playing)
        return;

    const char* path = nullptr;
    switch (m_state)
    {
    case GameState::Splash:     path = "assets/music/guitar.WAV";    break;

    case GameState::AuthSelect:
    case GameState::MainMenu:
    case GameState::LevelSelect:
    case GameState::Leaderboard:
    case GameState::DailyReward:
    case GameState::AboutUs:    path = "assets/music/maintheme.WAV"; break;

    case GameState::Login:
    case GameState::SignUp:     path = "assets/music/guitar.WAV";    break;

    case GameState::CharacterSelect: path = "assets/music/charselect.WAV"; break;

    case GameState::Settings:
    case GameState::SettingsPause:   path = "assets/music/guitar.WAV";    break;

    case GameState::GameOver:   path = "assets/music/gamel.WAV"; break;
    case GameState::Victory:    path = "assets/music/victory.WAV";   break;

    case GameState::Shop:
        m_prevAudioState = m_state;
        return;

    default: return;
    }

    if (path) playBgm(path, true);
    m_prevAudioState = m_state;
}


void Game::run()
{
    while (m_window.isOpen())
    {
        float dt = m_clock.restart().asSeconds();
        if (dt > 0.05f) dt = 0.05f;
        processEvents();
        update(dt);
        render();
    }
}

// processEvents
void Game::processEvents()
{
    const float sx = static_cast<float>(WINDOW_WIDTH) / 800.f;
    const float sy = static_cast<float>(WINDOW_HEIGHT) / 600.f;

    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            m_window.close();

        if (event.type == sf::Event::KeyPressed &&    //debugging
            (event.key.code == sf::Keyboard::F1 ||
                event.key.code == sf::Keyboard::H))
            m_debugMode = !m_debugMode;

        if (m_startClickPlaying) continue;

        // ── VIDEO─────────────────────────────────────────────
        if (m_state == GameState::IntroVideo ||
            m_state == GameState::OutroVideo)
        {
            bool skip = false;
            if (event.type == sf::Event::KeyPressed)
                skip = (event.key.code == sf::Keyboard::Space ||
                    event.key.code == sf::Keyboard::Return ||
                    event.key.code == sf::Keyboard::Escape);
            if (event.type == sf::Event::MouseButtonPressed)
                skip = true;

            if (skip)
            {
                stopVideo();
                m_state = m_videoReturnState;
                if (m_state == GameState::Playing)
                    loadLevelMusic(m_levelManager.getCurrentLevel());
            }
            continue;
        }

        // ── SPLASH 
        if (m_state == GameState::Splash)
        {
            if (event.type == sf::Event::MouseButtonPressed &&
                event.mouseButton.button == sf::Mouse::Left)
            {
                float btnW = 340.f, btnH = 70.f;
                float btnX = (WINDOW_WIDTH - btnW) / 2.f;
                float btnY = WINDOW_HEIGHT * 0.72f;
                float mx = static_cast<float>(event.mouseButton.x);
                float my = static_cast<float>(event.mouseButton.y);
                if (mx >= btnX && mx <= btnX + btnW && my >= btnY && my <= btnY + btnH)
                {
                    m_music.stop(); m_currentBgmPath.clear();
                    playSfx("assets/music/STARTBUTTON.WAV");
                    m_startClickTimer = 6.f;
                    m_startClickPlaying = true;
                    m_authSelectSelection = 0;
                }
            }
            if (event.type == sf::Event::KeyPressed &&
                (event.key.code == sf::Keyboard::Return ||
                    event.key.code == sf::Keyboard::Space))
            {
                m_music.stop(); m_currentBgmPath.clear();
                playSfx("assets/music/STARTBUTTON.WAV");
                m_startClickTimer = 6.f;
                m_startClickPlaying = true;
                m_authSelectSelection = 0;
            }
            if (event.type == sf::Event::MouseMoved)
            {
                float btnW = 340.f, btnH = 70.f;
                float btnX = (WINDOW_WIDTH - btnW) / 2.f;
                float btnY = WINDOW_HEIGHT * 0.72f;
                float mx = static_cast<float>(event.mouseMove.x);
                float my = static_cast<float>(event.mouseMove.y);
                m_splashButtonHovered = (mx >= btnX && mx <= btnX + btnW &&
                    my >= btnY && my <= btnY + btnH);
            }
            continue;
        }

        if (m_state == GameState::Settings)
        {
            if (m_settings.handleEvent(event, m_music))
            {
                m_settings.applyVolume(m_music);
                applySettingsBindings();
                m_state = m_settingsReturnState;
            }
            continue;
        }
        if (m_state == GameState::SettingsPause)
        {
            if (m_settings.handleEvent(event, m_music))
            {
                m_settings.applyVolume(m_music);
                applySettingsBindings();
                m_state = GameState::Paused;
            }
            continue;
        }

        if (m_state == GameState::AuthSelect) { handleAuthSelectEvents(event); continue; }
        if (m_state == GameState::MainMenu) { handleMainMenuEvents(event, sx, sy); continue; }
        if (m_state == GameState::Login) { handleAuthEvents(event, false); continue; }
        if (m_state == GameState::SignUp) { handleAuthEvents(event, true);  continue; }
        if (m_state == GameState::LevelSelect) { handleLevelSelectEvents(event); continue; }

        if (m_state == GameState::DailyReward)
        {
            if (event.type == sf::Event::KeyPressed &&
                (event.key.code == sf::Keyboard::Return ||
                    event.key.code == sf::Keyboard::Escape))
                m_state = GameState::MainMenu;
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (!m_dailyRewardClaimed) { m_dailyRewardClaimed = true; saveDailyRewardState(); }
                m_state = GameState::MainMenu;
            }
            continue;
        }

        if (m_state == GameState::AboutUs)
        {
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                m_state = GameState::MainMenu;
            if (event.type == sf::Event::MouseButtonPressed)
                m_state = GameState::MainMenu;
            continue;
        }

        if (m_state == GameState::CharacterSelect)
        {
            m_charSelect.update(event, m_pendingPlayerCount);
            if (m_charSelect.isDone())
            {
                if (m_charSelect.isCancelled() || m_charSelect.isBrowseOnly())
                    m_state = GameState::MainMenu;
                else
                    startFromCharacterSelect();
            }
            continue;
        }

        if (m_state == GameState::Shop)
        {
            if (event.type == sf::Event::MouseMoved)
                m_shop.handleMouseMove(
                    static_cast<float>(event.mouseMove.x),
                    static_cast<float>(event.mouseMove.y));
            m_shop.handleEvent(event, m_players, m_playerCount);
            if (!m_shop.isOpen()) m_state = m_shopReturnState;
            continue;
        }

        if (m_state == GameState::Playing &&
            event.type == sf::Event::KeyPressed &&
            (event.key.code == sf::Keyboard::Escape ||
                event.key.code == sf::Keyboard::P))
        {
            if (!m_levelManager.isStarChoiceOpen())
            {
                m_pauseSelection = 0; m_state = GameState::Paused;
            }
            continue;
        }

        if (m_state == GameState::Playing &&
            event.type == sf::Event::KeyPressed &&
            event.key.code == sf::Keyboard::S)
        {
            if (!m_levelManager.isStarChoiceOpen())
            {
                m_shopReturnState = GameState::Playing; m_shop.open(); m_state = GameState::Shop;
            }
            continue;
        }

        if (m_state == GameState::Playing &&
            m_levelManager.isStarChoiceOpen() &&
            event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Num1) handleStarChoiceInput(event);
            if (event.key.code == sf::Keyboard::Num2) handleStarChoiceInput(event);
            if (event.key.code == sf::Keyboard::Num3) handleStarChoiceInput(event);
            continue;
        }

        if (m_state == GameState::Playing &&
            m_levelManager.isStarChoiceOpen() &&
            event.type == sf::Event::MouseButtonReleased &&
            event.mouseButton.button == sf::Mouse::Left)
        {
            handleStarChoiceInput(event); continue;
        }

        if (m_state == GameState::Paused && event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Up)
                m_pauseSelection = (m_pauseSelection - 1 + PAUSE_MENU_ITEMS) % PAUSE_MENU_ITEMS;
            if (event.key.code == sf::Keyboard::Down)
                m_pauseSelection = (m_pauseSelection + 1) % PAUSE_MENU_ITEMS;
            if (event.key.code == sf::Keyboard::Return ||
                event.key.code == sf::Keyboard::Space)
                handlePauseMenuConfirm();
            if (event.key.code == sf::Keyboard::Escape ||
                event.key.code == sf::Keyboard::P)
                m_state = GameState::Playing;
            continue;
        }

        if (m_state == GameState::LevelComplete &&
            event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Return) advanceToNextLevel();
            if (event.key.code == sf::Keyboard::S)
            {
                m_shopReturnState = GameState::LevelComplete; m_shop.open(); m_state = GameState::Shop;
            }
            continue;
        }

        if (m_state == GameState::GameOver && event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::C)
            {
                for (int i = 0; i < m_playerCount; i++)
                {
                    m_players[i]->resetForNewLevel(); m_players[i]->setActive(true);
                }
                m_levelManager.loadLevel(m_levelManager.getCurrentLevel());
                loadLevelMusic(m_levelManager.getCurrentLevel());
                m_state = GameState::Playing;
            }
            if (event.key.code == sf::Keyboard::M) { stopBgm(); m_state = GameState::MainMenu; }
            continue;
        }

        if (m_state == GameState::Victory && event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Return ||
                event.key.code == sf::Keyboard::Space)
            {
                stopBgm();
                loadLeaderboard();                          // ← FIXED: refresh before showing
                m_state = GameState::Leaderboard;
            }
            if (event.key.code == sf::Keyboard::M)
            {
                stopBgm(); m_state = GameState::MainMenu;
            }
            continue;
        }

        if (m_state == GameState::Leaderboard &&
            event.type == sf::Event::KeyPressed &&
            event.key.code == sf::Keyboard::Escape)
        {
            m_state = GameState::MainMenu; continue;
        }
    }
}

// handleStarChoiceInput
void Game::handleStarChoiceInput(const sf::Event& event)
{
    int chosen = -1;
    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Num1) chosen = 0;
        if (event.key.code == sf::Keyboard::Num2) chosen = 1;
        if (event.key.code == sf::Keyboard::Num3) chosen = 2;
    }
    else if (event.type == sf::Event::MouseButtonReleased)
    {
        float mx = static_cast<float>(event.mouseButton.x);
        float my = static_cast<float>(event.mouseButton.y);
        float cardW = 180.f, cardH = 160.f, spacing = 20.f;
        float totalW = 3.f * cardW + 2.f * spacing;
        float startX = (800.f - totalW) * 0.5f;
        float cardY = (600.f - 300.f) * 0.5f + 70.f;
        for (int i = 0; i < 3; i++)
        {
            float cx = startX + i * (cardW + spacing);
            if (mx >= cx && mx <= cx + cardW && my >= cardY && my <= cardY + cardH)
            {
                chosen = i; break;
            }
        }
    }
    if (chosen >= 0)
        m_levelManager.handleStarChoice(chosen, m_players, m_playerCount);
}

// handleMainMenuEvents
void Game::handleMainMenuEvents(const sf::Event& event, float sx, float sy)
{
    const float W = static_cast<float>(WINDOW_WIDTH);
    const float H = static_cast<float>(WINDOW_HEIGHT);
    const float bCX = W * 0.519f;
    const float bHW = W * 0.185f;
    const float bHH = H * 0.055f;
    const float rows[7] = { 0.33f, 0.40f, 0.47f, 0.53f, 0.60f, 0.67f, 0.73f };

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::L && m_saveSlot.valid) { loadGame(); return; }
        if (event.key.code == sf::Keyboard::Up)
        {
            m_menuSelection = (m_menuSelection - 1 + 7) % 7; return;
        }
        if (event.key.code == sf::Keyboard::Down)
        {
            m_menuSelection = (m_menuSelection + 1) % 7; return;
        }
        if (event.key.code == sf::Keyboard::Return ||
            event.key.code == sf::Keyboard::Space)
        {
            handleNewMenuConfirm(m_menuSelection); return;
        }
        return;
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        float mx = static_cast<float>(event.mouseButton.x);
        float my = static_cast<float>(event.mouseButton.y);

        for (int i = 0; i < 7; i++)
        {
            float cy = H * rows[i];
            if (mx >= bCX - bHW && mx <= bCX + bHW &&
                my >= cy - bHH && my <= cy + bHH)
            {
                m_menuSelection = i; handleNewMenuConfirm(i); return;
            }
        }

        const float iconY = H * 0.900f;
        const float iconHH = H * 0.050f;
        const float iconHW = W * 0.022f;
        const float iconXs[5] = {
            W * 0.362f, W * 0.399f, W * 0.437f, W * 0.474f, W * 0.511f
        };
        for (int i = 0; i < 5; i++)
        {
            float ix = iconXs[i];
            if (mx >= ix - iconHW && mx <= ix + iconHW &&
                my >= iconY - iconHH && my <= iconY + iconHH)
            {
                switch (i)
                {
                case 0: break;
                case 1:
                    loadLeaderboard();                      // ← FIXED: refresh on icon click
                    m_state = GameState::Leaderboard;
                    break;
                case 2: m_state = GameState::CharacterSelect; break;
                case 3:
                    m_settingsReturnState = GameState::MainMenu;
                    m_settings.open(); m_state = GameState::Settings; break;
                case 4: m_window.close(); break;
                }
                return;
            }
        }
        if (mx <= W * 0.17f && my <= H * 0.22f)
            m_state = GameState::DailyReward;
    }
}

// handleNewMenuConfirm
void Game::handleNewMenuConfirm(int idx)
{
    switch (idx)
    {
    case 0:
        m_pendingPlayerCount = 1;
        m_levelSelectChoice = 1;
        m_howToPlayOpen = false;
        m_howToPlayPage = 0;
        m_state = GameState::LevelSelect;
        break;
    case 1: m_state = GameState::CharacterSelect; break;
    case 2:
        m_pendingPlayerCount = 2;
        m_levelSelectChoice = 1;
        m_howToPlayOpen = false;
        m_howToPlayPage = 0;
        m_state = GameState::LevelSelect;
        break;
    case 3:
        loadLeaderboard();                                  // ← FIXED: refresh from menu
        m_state = GameState::Leaderboard;
        break;
    case 4:
        m_shopReturnState = GameState::MainMenu;
        m_shop.open(); m_state = GameState::Shop; break;
    case 5:
        m_settingsReturnState = GameState::MainMenu;
        m_settings.open(); m_state = GameState::Settings; break;
    case 6: m_window.close(); break;
    }
}

// handleAuthEvents
void Game::handleAuthEvents(const sf::Event& event, bool isSignUp)
{
    const int fieldCount = isSignUp ? 4 : 2;

    if (event.type == sf::Event::TextEntered)
    {
        sf::Uint32 c = event.text.unicode;
        if (c == 8)
        {
            if (!m_authField[m_authActiveField].empty())
                m_authField[m_authActiveField].pop_back();
            m_authError = "";
        }
        else if (c == 9) { m_authActiveField = (m_authActiveField + 1) % fieldCount; }
        else if (c >= 32 && c < 128)
        {
            if (m_authField[m_authActiveField].size() < 32)
                m_authField[m_authActiveField] += static_cast<char>(c);
            m_authError = "";
        }
        return;
    }

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Tab)
            m_authActiveField = (m_authActiveField + 1) % fieldCount;
        else if (event.key.code == sf::Keyboard::Up)
            m_authActiveField = (m_authActiveField - 1 + fieldCount) % fieldCount;
        else if (event.key.code == sf::Keyboard::Down)
            m_authActiveField = (m_authActiveField + 1) % fieldCount;
        else if (event.key.code == sf::Keyboard::Return)
        {
            if (isSignUp) attemptSignUp(); else attemptLogin();
        }
        else if (event.key.code == sf::Keyboard::Escape)
            m_state = GameState::AuthSelect;
        return;
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        float mx = static_cast<float>(event.mouseButton.x);
        float my = static_cast<float>(event.mouseButton.y);

        float cardW = isSignUp ? 370.f : 340.f;
        float cardH = isSignUp ? 340.f : 270.f;
        float cardX = (WINDOW_WIDTH - cardW) / 2.f;
        float cardY = (WINDOW_HEIGHT - cardH) / 2.f + (isSignUp ? 15.f : 20.f);

        float fieldH = 44.f, fieldGap = 14.f;
        float fieldX = cardX + 28.f;
        float fieldW = cardW - 56.f;
        float firstFieldY = cardY + 60.f;

        for (int i = 0; i < fieldCount; i++)
        {
            float fy = firstFieldY + i * (fieldH + fieldGap);
            if (mx >= fieldX && mx <= fieldX + fieldW &&
                my >= fy && my <= fy + fieldH)
            {
                m_authActiveField = i; return;
            }

            bool isPassField = (!isSignUp && i == 1) ||
                (isSignUp && (i == 2 || i == 3));
            if (isPassField)
            {
                float eyeX = fieldX + fieldW - 36.f;
                float eyeY = fy + fieldH / 2.f - 10.f;
                if (mx >= eyeX && mx <= eyeX + 26.f &&
                    my >= eyeY && my <= eyeY + 20.f)
                {
                    int eyeIdx = (i == 1 && !isSignUp) ? 0
                        : (i == 2 && isSignUp) ? 0 : 1;
                    m_authShowPassword[eyeIdx] = !m_authShowPassword[eyeIdx];
                    return;
                }
            }
        }

        float btnW = cardW - 56.f, btnH = 46.f;
        float btnX = cardX + 28.f;
        float btnY = firstFieldY + fieldCount * (fieldH + fieldGap) + 4.f;
        if (mx >= btnX && mx <= btnX + btnW && my >= btnY && my <= btnY + btnH)
        {
            if (isSignUp) attemptSignUp(); else attemptLogin(); return;
        }

        if (!isSignUp)
        {
            float guestY = btnY + btnH + 12.f;
            if (my >= guestY && my <= guestY + 20.f)
            {
                m_loggedIn = false; m_loggedInUser = "";
                m_enteredAsGuest = true;
                m_state = GameState::MainMenu; return;
            }
        }

        float linkY = cardY + cardH - 28.f;
        if (my >= linkY && my <= linkY + 22.f)
        {
            for (int i = 0; i < 4; i++) m_authField[i] = "";
            m_authActiveField = 0; m_authError = "";
            m_authShowPassword[0] = m_authShowPassword[1] = false;
            m_state = isSignUp ? GameState::Login : GameState::SignUp;
            return;
        }
        if (mx < 90.f && my < 55.f) { m_state = GameState::AuthSelect; return; }
    }
}

// handleAuthSelectEvents
void Game::handleAuthSelectEvents(const sf::Event& event)
{
    const float btnW = 380.f, btnH = 58.f, btnGap = 18.f;
    const float btnX = (WINDOW_WIDTH - btnW) / 2.f;
    const float firstBtnY = WINDOW_HEIGHT * 0.255f;

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Up)
            m_authSelectSelection = (m_authSelectSelection - 1 + 3) % 3;
        else if (event.key.code == sf::Keyboard::Down)
            m_authSelectSelection = (m_authSelectSelection + 1) % 3;
        else if (event.key.code == sf::Keyboard::Return ||
            event.key.code == sf::Keyboard::Space)
        {
            if (m_authSelectSelection == 0)
            {
                for (int i = 0; i < 4; i++) m_authField[i] = "";
                m_authActiveField = 0; m_authError = "";
                m_authShowPassword[0] = m_authShowPassword[1] = false;
                m_state = GameState::Login;
            }
            else if (m_authSelectSelection == 1)
            {
                for (int i = 0; i < 4; i++) m_authField[i] = "";
                m_authActiveField = 0; m_authError = "";
                m_authShowPassword[0] = m_authShowPassword[1] = false;
                m_state = GameState::SignUp;
            }
            else
            {
                m_loggedIn = false; m_loggedInUser = "";
                m_enteredAsGuest = true;
                m_state = GameState::MainMenu;
            }
        }
        else if (event.key.code == sf::Keyboard::Escape)
            m_state = GameState::Splash;
        return;
    }

    if (event.type == sf::Event::MouseMoved)
    {
        float mx = static_cast<float>(event.mouseMove.x);
        float my = static_cast<float>(event.mouseMove.y);
        for (int i = 0; i < 3; i++)
        {
            float by = firstBtnY + i * (btnH + btnGap);
            m_authSelectHovered[i] = (mx >= btnX && mx <= btnX + btnW &&
                my >= by && my <= by + btnH);
            if (m_authSelectHovered[i]) m_authSelectSelection = i;
        }
        return;
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        float mx = static_cast<float>(event.mouseButton.x);
        float my = static_cast<float>(event.mouseButton.y);
        for (int i = 0; i < 3; i++)
        {
            float by = firstBtnY + i * (btnH + btnGap);
            if (mx >= btnX && mx <= btnX + btnW && my >= by && my <= by + btnH)
            {
                m_authSelectSelection = i;
                if (i == 0)
                {
                    for (int j = 0; j < 4; j++) m_authField[j] = "";
                    m_authActiveField = 0; m_authError = "";
                    m_authShowPassword[0] = m_authShowPassword[1] = false;
                    m_state = GameState::Login;
                }
                else if (i == 1)
                {
                    for (int j = 0; j < 4; j++) m_authField[j] = "";
                    m_authActiveField = 0; m_authError = "";
                    m_authShowPassword[0] = m_authShowPassword[1] = false;
                    m_state = GameState::SignUp;
                }
                else
                {
                    m_loggedIn = false; m_loggedInUser = "";
                    m_enteredAsGuest = true;
                    m_state = GameState::MainMenu;
                }
                return;
            }
        }
        if (mx < 90.f && my < 55.f) m_state = GameState::Splash;
    }
}


// attemptLogin
void Game::attemptLogin()
{
    const std::string& user = m_authField[0];
    const std::string& pass = m_authField[1];
    if (user.empty() || pass.empty())
    {
        m_authError = "Username and password cannot be empty."; return;
    }

    std::string hashedInput = hashToHex(hashPassword(pass));
    std::ifstream fin("users.txt");
    if (!fin.is_open())
    {
        m_authError = "No accounts found. Please sign up first."; return;
    }

    std::string line;
    while (std::getline(fin, line))
    {
        std::istringstream ss(line);
        std::string u, p;
        if (std::getline(ss, u, '|') && std::getline(ss, p))
        {
            if (u == user && p == hashedInput)
            {
                m_loggedIn = true; m_loggedInUser = user;
                m_enteredAsGuest = false; m_authError = "";
                m_state = GameState::MainMenu;
                fin.close(); return;
            }
        }
    }
    fin.close();
    m_authError = "Incorrect username or password.";
}

void Game::attemptSignUp()
{
    const std::string& user = m_authField[0];
    const std::string& email = m_authField[1];
    const std::string& pass = m_authField[2];
    const std::string& confirm = m_authField[3];

    if (user.empty() || pass.empty())
    {
        m_authError = "Username and password cannot be empty."; return;
    }
    if (user.size() < 3)
    {
        m_authError = "Username must be at least 3 characters."; return;
    }
    if (pass.size() < 4)
    {
        m_authError = "Password must be at least 4 characters."; return;
    }
    if (pass != confirm)
    {
        m_authError = "Passwords do not match."; return;
    }
    if (!email.empty() && email.find('@') == std::string::npos)
    {
        m_authError = "Please enter a valid email address."; return;
    }

    std::ifstream fin("users.txt");
    if (fin.is_open())
    {
        std::string line;
        while (std::getline(fin, line))
        {
            std::istringstream ss(line);
            std::string u;
            if (std::getline(ss, u, '|') && u == user)
            {
                fin.close(); m_authError = "Username already taken."; return;
            }
        }
        fin.close();
    }

    std::string hashedPass = hashToHex(hashPassword(pass));
    std::ofstream fout("users.txt", std::ios::app);
    if (!fout.is_open())
    {
        m_authError = "Could not save account. Check write permissions."; return;
    }

    fout << user << "|" << hashedPass << "\n";
    fout.close();

    m_loggedIn = true; m_loggedInUser = user;
    m_enteredAsGuest = false; m_authError = "";
    m_state = GameState::MainMenu;
}


void Game::loadDailyRewardState()
{
    std::ifstream fin("daily_reward.txt");
    if (!fin.is_open()) return;
    std::string savedDate;
    std::getline(fin, savedDate);
    fin.close();

    time_t t = time(nullptr);
    char buf[16];
    struct tm timeInfo;
#ifdef _WIN32
    localtime_s(&timeInfo, &t);
#else
    localtime_r(&t, &timeInfo);
#endif
    strftime(buf, sizeof(buf), "%Y-%m-%d", &timeInfo);
    m_dailyRewardClaimed = (savedDate == std::string(buf));
}

void Game::saveDailyRewardState()
{
    std::ofstream fout("daily_reward.txt");
    if (!fout.is_open()) return;
    time_t t = time(nullptr);
    char buf[16];
    struct tm timeInfo;
#ifdef _WIN32
    localtime_s(&timeInfo, &t);
#else
    localtime_r(&t, &timeInfo);
#endif
    strftime(buf, sizeof(buf), "%Y-%m-%d", &timeInfo);
    fout << buf << "\n";
    fout.close();
}


// handlePauseMenuConfirm

void Game::handlePauseMenuConfirm()
{
    switch (m_pauseSelection)
    {
    case 0: m_state = GameState::Playing; break;
    case 1: m_shopReturnState = GameState::Paused; m_shop.open(); m_state = GameState::Shop; break;
    case 2: saveGame(); break;
    case 3: stopBgm(); clearPlayers(); m_state = GameState::MainMenu; break;
    case 4: m_window.close(); break;
    case 5: m_settings.open(); m_state = GameState::SettingsPause; break;
    }
}

// update
void Game::update(float dt)
{
    updateBgmForState();

    if (m_startClickPlaying)
    {
        m_startClickTimer -= dt;
        if (m_startClickTimer <= 0.f || m_sfx.getStatus() != sf::Music::Playing)
        {
            m_startClickPlaying = false;
            m_state = GameState::AuthSelect;
        }
        return;
    }

    // ── Video updat
    if (m_state == GameState::IntroVideo)
    {
        m_videoTimer += dt;
        m_introVideo.update(dt,
            static_cast<float>(WINDOW_WIDTH),
            static_cast<float>(WINDOW_HEIGHT));

        if (m_introVideo.isFinished() || m_videoTimer >= VIDEO_MAX_SECONDS)
        {
            stopVideo();
            m_state = m_videoReturnState;
            loadLevelMusic(m_levelManager.getCurrentLevel());
        }
        return;
    }

    if (m_state == GameState::OutroVideo)
    {
        m_videoTimer += dt;
        m_outroVideo.update(dt,
            static_cast<float>(WINDOW_WIDTH),
            static_cast<float>(WINDOW_HEIGHT));

        if (m_outroVideo.isFinished() || m_videoTimer >= VIDEO_MAX_SECONDS)
        {
            stopVideo();
            m_state = m_videoReturnState;
        }
        return;
    }

    if (m_state == GameState::Splash)
    {
        m_splashTimer += dt;
        float targetScale = m_splashButtonHovered ? 1.07f : 1.0f;
        m_splashButtonScale += (targetScale - m_splashButtonScale) * 10.f * dt;
        return;
    }

    if (m_state == GameState::Login || m_state == GameState::SignUp)
    {
        m_authCursorTimer += dt;
        if (m_authCursorTimer >= 0.5f)
        {
            m_authCursorTimer = 0.f;
            m_authCursorVisible = !m_authCursorVisible;
        }
    }

    if (m_saveMsgTimer > 0.f) m_saveMsgTimer -= dt;

    if (m_state == GameState::Playing)            updatePlaying(dt);
    else if (m_state == GameState::LevelComplete) updateLevelComplete(dt);
    else if (m_state == GameState::Shop)          m_shop.update(dt);
    else if (m_state == GameState::Settings ||
        m_state == GameState::SettingsPause) m_settings.applyVolume(m_music);
}

void Game::updatePlaying(float dt)
{
    for (int i = 0; i < m_playerCount; i++)
    {
        m_players[i]->update(dt);
        Snowball* sb = m_players[i]->tryThrow();
        if (sb) m_levelManager.addSnowball(sb);
    }
    m_levelManager.update(dt, m_players, m_playerCount);
    m_levelManager.resolveCollisions(m_players, m_playerCount);

    bool anyAlive = false;
    for (int i = 0; i < m_playerCount; i++)
        if (m_players[i]->isAlive()) anyAlive = true;

    if (!anyAlive)
    {
        const std::string scoreUser = (m_loggedIn && !m_loggedInUser.empty())
            ? m_loggedInUser : std::string(m_players[0]->getName());
        for (int i = 0; i < m_playerCount; i++)
            submitScore(scoreUser.c_str(), m_players[i]->getScore(),
                m_players[i]->getGems(), m_levelManager.getCurrentLevel());
        m_endSoundPlayed = false;
        m_state = GameState::GameOver;
    }

    if (m_levelManager.isLevelComplete())
    {
        const std::string scoreUser = (m_loggedIn && !m_loggedInUser.empty())
            ? m_loggedInUser : std::string(m_players[0]->getName());
        for (int i = 0; i < m_playerCount; i++)
            submitScore(scoreUser.c_str(), m_players[i]->getScore(),
                m_players[i]->getGems(), m_levelManager.getCurrentLevel());
        playSfx("assets/music/level_win.ogg");
        m_state = GameState::LevelComplete;
    }
}


void Game::updateLevelComplete(float /*dt*/) {}

// render
void Game::render()
{
    m_window.clear(sf::Color(30, 10, 40));
    switch (m_state)
    {
    case GameState::Splash:          drawSplash();            break;
    case GameState::AuthSelect:      drawAuthSelect();        break;
    case GameState::MainMenu:        drawMainMenu();          break;
    case GameState::Login:           drawAuthScreen(false);   break;
    case GameState::SignUp:          drawAuthScreen(true);    break;
    case GameState::LevelSelect:     drawLevelSelect();       break;
    case GameState::DailyReward:     drawDailyReward();       break;
    case GameState::AboutUs:         drawAboutUs();           break;
    case GameState::Leaderboard:     drawLeaderboard();       break;
    case GameState::CharacterSelect: m_charSelect.draw(m_window); break;

    case GameState::IntroVideo:
        drawVideo(m_introVideo, "SPACE / ENTER / CLICK  to skip"); break;
    case GameState::OutroVideo:
        drawVideo(m_outroVideo, "SPACE / ENTER / CLICK  to skip"); break;

    case GameState::Playing:         drawPlaying();           break;
    case GameState::Paused:          drawPlaying(); drawPaused(); break;
    case GameState::SettingsPause:
        drawPlaying(); drawPaused(); m_settings.draw(m_window); break;
    case GameState::Settings:
        drawMainMenu(); m_settings.draw(m_window); break;
    case GameState::Shop:
        if (m_shopReturnState == GameState::MainMenu) drawMainMenu();
        else drawPlaying();
        m_shop.draw(m_window, m_players, m_playerCount); break;
    case GameState::LevelComplete:   drawLevelComplete();     break;
    case GameState::GameOver:        drawGameOver();          break;
    case GameState::Victory:         drawVictory();           break;
    default: break;
    }
    m_window.display();
}

// drawVideo

void Game::drawVideo(VideoPlayer& vp, const std::string& skipHint)
{
    m_window.clear(sf::Color::Black);

    if (!vp.frames.empty())
        m_window.draw(vp.sprite);

    int total = static_cast<int>(vp.frames.size());
    int current = vp.current;
    if (total > 0)
    {
        float barW = static_cast<float>(WINDOW_WIDTH);
        float prog = static_cast<float>(current) / static_cast<float>(total);
        sf::RectangleShape bar(sf::Vector2f(barW * prog, 4.f));
        bar.setPosition(0.f, static_cast<float>(WINDOW_HEIGHT) - 4.f);
        bar.setFillColor(sf::Color(255, 100, 180, 200));
        m_window.draw(bar);
    }

    sf::Text hint;
    hint.setFont(m_font);
    hint.setCharacterSize(13);
    hint.setFillColor(sf::Color(220, 220, 220, 160));
    hint.setString(skipHint);
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setPosition(
        static_cast<float>(WINDOW_WIDTH) - hb.width - 12.f,
        static_cast<float>(WINDOW_HEIGHT) - hb.height - 14.f);
    m_window.draw(hint);
}


void Game::drawSplash()
{
    if (m_splashTexture.getSize().x > 0)
        m_window.draw(m_splashSprite);
    else
    {
        sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        bg.setFillColor(sf::Color(255, 182, 210));
        m_window.draw(bg);
    }

    const float btnW = 340.f, btnH = 70.f;
    const float btnX = (WINDOW_WIDTH - btnW) / 2.f;
    const float btnY = WINDOW_HEIGHT * 0.73f;

    const float s = m_splashButtonScale;
    const float cx = btnX + btnW / 2.f;
    const float cy = btnY + btnH / 2.f;
    const float scaledW = btnW * s;
    const float scaledH = btnH * s;
    const float scaledX = cx - scaledW / 2.f;
    const float liftY = (s - 1.f) * 8.f;
    const float scaledY = cy - scaledH / 2.f - liftY;

    sf::Uint8 shadowAlpha = static_cast<sf::Uint8>(90 + (s - 1.f) / 0.07f * 50.f);
    float     shadowDrop = 4.f + (s - 1.f) / 0.07f * 6.f;

    auto makePill = [](float x, float y, float w, float h,
        sf::Color fill, sf::Color outline,
        float outlineThick) -> sf::ConvexShape
        {
            const int   segs = 16;
            const float r = h / 2.f;
            const float pi = 3.14159265f;
            sf::ConvexShape pill;
            pill.setPointCount(segs * 4);
            int pt = 0;
            for (int i = 0; i < segs; i++) {
                float a = -pi / 2.f + (pi / 2.f) * i / (segs - 1);
                pill.setPoint(pt++, sf::Vector2f(x + w - r + r * std::cos(a), y + r + r * std::sin(a)));
            }
            for (int i = 0; i < segs; i++) {
                float a = 0.f + (pi / 2.f) * i / (segs - 1);
                pill.setPoint(pt++, sf::Vector2f(x + w - r + r * std::cos(a), y + r + r * std::sin(a)));
            }
            for (int i = 0; i < segs; i++) {
                float a = pi / 2.f + (pi / 2.f) * i / (segs - 1);
                pill.setPoint(pt++, sf::Vector2f(x + r + r * std::cos(a), y + r + r * std::sin(a)));
            }
            for (int i = 0; i < segs; i++) {
                float a = pi + (pi / 2.f) * i / (segs - 1);
                pill.setPoint(pt++, sf::Vector2f(x + r + r * std::cos(a), y + r + r * std::sin(a)));
            }
            pill.setFillColor(fill);
            pill.setOutlineColor(outline);
            pill.setOutlineThickness(outlineThick);
            return pill;
        };

    auto shadowPill = makePill(
        scaledX - 3.f + shadowDrop, scaledY - 3.f + shadowDrop,
        scaledW + 6.f, scaledH + 6.f,
        sf::Color(160, 20, 80, shadowAlpha), sf::Color::Transparent, 0.f);
    m_window.draw(shadowPill);

    sf::Color btnColor = m_splashButtonHovered
        ? sf::Color(255, 140, 185, 255) : sf::Color(240, 100, 150, 235);
    sf::Color outlineCol = sf::Color(255, 255, 255, m_splashButtonHovered ? 220 : 160);
    float     outlineThk = m_splashButtonHovered ? 3.f : 2.f;

    auto btnPill = makePill(scaledX, scaledY, scaledW, scaledH,
        btnColor, outlineCol, outlineThk);
    m_window.draw(btnPill);

    sf::Uint8 shineAlpha = static_cast<sf::Uint8>(55 + (s - 1.f) / 0.07f * 30.f);
    auto shinePill = makePill(scaledX + 10.f, scaledY + 4.f,
        scaledW - 20.f, scaledH * 0.38f,
        sf::Color(255, 255, 255, shineAlpha),
        sf::Color::Transparent, 0.f);
    m_window.draw(shinePill);

    sf::Text btnTxt;
    btnTxt.setFont(m_font);
    btnTxt.setCharacterSize(static_cast<unsigned int>(26.f * s));
    btnTxt.setFillColor(sf::Color::White);
    btnTxt.setString("START GAME");
    sf::FloatRect tb = btnTxt.getLocalBounds();
    btnTxt.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    btnTxt.setPosition(cx, cy - liftY);
    m_window.draw(btnTxt);
}


void Game::drawAuthSelect()
{
    if (m_authSelectBgTexture.getSize().x > 0) m_window.draw(m_authSelectBgSprite);
    else if (m_menuBgTexture.getSize().x > 0)  m_window.draw(m_menuBgSprite);
    else {
        sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        bg.setFillColor(sf::Color(255, 200, 220)); m_window.draw(bg);
    }

    const float cardW = 440.f, cardH = 280.f;
    const float cardX = (WINDOW_WIDTH - cardW) / 2.f;
    const float cardY = WINDOW_HEIGHT * 0.42f;

    if (m_authSelectCardTexture.getSize().x > 0)
    {
        m_authSelectCardSprite.setScale(
            cardW / m_authSelectCardTexture.getSize().x,
            cardH / m_authSelectCardTexture.getSize().y);
        m_authSelectCardSprite.setPosition(cardX, cardY);
        m_window.draw(m_authSelectCardSprite);
    }
    else
    {
        sf::RectangleShape card(sf::Vector2f(cardW, cardH));
        card.setPosition(cardX, cardY);
        card.setFillColor(sf::Color(250, 225, 235, 245));
        card.setOutlineColor(sf::Color(220, 160, 190, 180));
        card.setOutlineThickness(3.f);
        m_window.draw(card);
    }

    struct BtnDef {
        const char* label; const char* icon;
        sf::Color fill; sf::Color fillSel; sf::Color iconCol;
    };
    BtnDef btns[3] = {
        { "LOGIN",   "@", sf::Color(240,120,160,230), sf::Color(255,140,180,255), sf::Color(200,60,100) },
        { "SIGN UP", "@", sf::Color(100,170,245,230), sf::Color(120,190,255,255), sf::Color(40,100,200) },
        { "GUEST",   "?", sf::Color(170,110,230,230), sf::Color(190,130,255,255), sf::Color(100,50,180) },
    };

    const float btnW = 260.f, btnH = 52.f, btnGap = 14.f;
    const float btnX = (WINDOW_WIDTH - btnW) / 2.f;
    const float firstBtnY = cardY + 54.f;

    for (int i = 0; i < 3; i++)
    {
        float by = firstBtnY + i * (btnH + btnGap);
        bool  sel = (m_authSelectSelection == i);

        sf::RectangleShape shadow(sf::Vector2f(btnW + 4.f, btnH + 6.f));
        shadow.setPosition(btnX - 2.f + 4.f, by - 2.f + 6.f);
        shadow.setFillColor(sf::Color(0, 0, 0, 50));
        m_window.draw(shadow);

        sf::RectangleShape pill(sf::Vector2f(btnW, btnH));
        pill.setPosition(btnX, by);
        pill.setFillColor(sel ? btns[i].fillSel : btns[i].fill);
        pill.setOutlineColor(sel ? sf::Color(255, 255, 255, 220) : sf::Color(255, 255, 255, 100));
        pill.setOutlineThickness(sel ? 3.f : 1.5f);
        m_window.draw(pill);

        sf::RectangleShape shine(sf::Vector2f(btnW - 24.f, btnH * 0.35f));
        shine.setPosition(btnX + 12.f, by + 4.f);
        shine.setFillColor(sf::Color(255, 255, 255, 55));
        m_window.draw(shine);

        sf::CircleShape iconCircle(22.f);
        iconCircle.setOrigin(22.f, 22.f);
        iconCircle.setPosition(btnX + 36.f, by + btnH / 2.f);
        iconCircle.setFillColor(sf::Color(255, 255, 255, 200));
        iconCircle.setOutlineColor(btns[i].iconCol);
        iconCircle.setOutlineThickness(2.5f);
        m_window.draw(iconCircle);

        sf::Text ico; ico.setFont(m_font); ico.setCharacterSize(16);
        ico.setFillColor(btns[i].iconCol); ico.setString(btns[i].icon);
        sf::FloatRect ib = ico.getLocalBounds();
        ico.setOrigin(ib.left + ib.width / 2.f, ib.top + ib.height / 2.f);
        ico.setPosition(btnX + 36.f, by + btnH / 2.f - 1.f);
        m_window.draw(ico);

        sf::Text lbl; lbl.setFont(m_font); lbl.setCharacterSize(22);
        lbl.setFillColor(sf::Color::White); lbl.setString(btns[i].label);
        sf::FloatRect lb = lbl.getLocalBounds();
        lbl.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
        lbl.setPosition(btnX + btnW / 2.f + 16.f, by + btnH / 2.f);
        m_window.draw(lbl);

        if (sel) {
            sf::Text arrow; arrow.setFont(m_font); arrow.setCharacterSize(20);
            arrow.setFillColor(sf::Color(255, 255, 255, 200)); arrow.setString(">");
            arrow.setPosition(btnX + btnW - 32.f, by + (btnH - 24.f) / 2.f);
            m_window.draw(arrow);
        }
    }

    sf::Text hint; hint.setFont(m_font); hint.setCharacterSize(12);
    hint.setFillColor(sf::Color(140, 100, 120, 200));
    hint.setString("UP / DOWN  navigate     ENTER  confirm     ESC  back");
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.left + hb.width / 2.f, 0.f);
    hint.setPosition(WINDOW_WIDTH / 2.f, cardY + cardH - 22.f);
    m_window.draw(hint);

    sf::Text back; back.setFont(m_font); back.setCharacterSize(18);
    back.setFillColor(sf::Color(255, 255, 255, 200)); back.setString("< Back");
    back.setPosition(16.f, 14.f);
    m_window.draw(back);
}


void Game::drawAuthScreen(bool isSignUp)
{
    if (isSignUp)
    {
        if (m_signupBgTexture.getSize().x > 0)    m_window.draw(m_signupBgSprite);
        else if (m_menuBgTexture.getSize().x > 0) m_window.draw(m_menuBgSprite);
        else {
            sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
            bg.setFillColor(sf::Color(255, 182, 210)); m_window.draw(bg);
        }
    }
    else
    {
        if (m_loginBgTexture.getSize().x > 0)     m_window.draw(m_loginBgSprite);
        else if (m_menuBgTexture.getSize().x > 0) m_window.draw(m_menuBgSprite);
        else {
            sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
            bg.setFillColor(sf::Color(255, 182, 210)); m_window.draw(bg);
        }
    }

    const int fieldCount = isSignUp ? 4 : 2;
    float cardW = isSignUp ? 370.f : 340.f;
    float cardH = isSignUp ? 340.f : 270.f;
    float cardX = (WINDOW_WIDTH - cardW) / 2.f;
    float cardY = (WINDOW_HEIGHT - cardH) / 2.f + (isSignUp ? 85.f : 45.f);

    if (m_loginCardTexture.getSize().x > 0)
    {
        m_loginCardSprite.setScale(
            cardW / m_loginCardTexture.getSize().x,
            cardH / m_loginCardTexture.getSize().y);
        m_loginCardSprite.setPosition(cardX, cardY);
        m_window.draw(m_loginCardSprite);
    }
    else
    {
        sf::RectangleShape cardShadow(sf::Vector2f(cardW + 8.f, cardH + 8.f));
        cardShadow.setPosition(cardX - 4.f + 5.f, cardY - 4.f + 6.f);
        cardShadow.setFillColor(sf::Color(0, 0, 0, 60));
        m_window.draw(cardShadow);
        sf::RectangleShape card(sf::Vector2f(cardW, cardH));
        card.setPosition(cardX, cardY);
        card.setFillColor(sf::Color(235, 244, 255, 245));
        card.setOutlineColor(sf::Color(200, 220, 240));
        card.setOutlineThickness(2.f);
        m_window.draw(card);
    }

    {
        const char* titleStr = isSignUp ? "SIGN UP" : "LOGIN";
        sf::Color starCol = sf::Color(234, 100, 155);

        sf::CircleShape starL(9.f, 5);
        starL.setOrigin(9.f, 9.f);
        starL.setFillColor(starCol);
        starL.setRotation(-18.f);

        sf::CircleShape starR(9.f, 5);
        starR.setOrigin(9.f, 9.f);
        starR.setFillColor(starCol);
        starR.setRotation(-18.f);

        sf::Text titleTxt; titleTxt.setFont(m_font); titleTxt.setCharacterSize(22);
        titleTxt.setStyle(sf::Text::Bold);
        titleTxt.setFillColor(starCol);
        titleTxt.setString(titleStr);
        sf::FloatRect tb = titleTxt.getLocalBounds();
        float titleCX = cardX + cardW / 2.f;
        float titleCY = cardY + 22.f;
        titleTxt.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        titleTxt.setPosition(titleCX, titleCY);

        float gap = 10.f;
        starL.setPosition(titleCX - tb.width / 2.f - gap - 9.f, titleCY);
        starR.setPosition(titleCX + tb.width / 2.f + gap + 9.f, titleCY);

        m_window.draw(starL);
        m_window.draw(starR);
        m_window.draw(titleTxt);
    }

    float fieldH = 44.f, fieldGap = 14.f;
    float fieldX = cardX + 28.f;
    float fieldW = cardW - 56.f;
    float firstFieldY = cardY + 54.f;

    const char* fieldIcons[] = { "@", "~", "#", "#" };
    const char* fieldHints[] = { "Username","Email","Password","Confirm Password" };
    const char* loginIcons[] = { "@", "#" };
    const char* loginHints[] = { "Username","Password" };

    for (int i = 0; i < fieldCount; i++)
    {
        float fy = firstFieldY + i * (fieldH + fieldGap);
        bool isActive = (m_authActiveField == i);
        bool isPassField = (!isSignUp && i == 1) || (isSignUp && (i == 2 || i == 3));
        int  eyeIdx = (!isSignUp && i == 1) ? 0 : (isSignUp && i == 2) ? 0 : 1;

        sf::RectangleShape fieldBg(sf::Vector2f(fieldW, fieldH));
        fieldBg.setPosition(fieldX, fy);
        fieldBg.setFillColor(sf::Color(255, 255, 255, 240));
        fieldBg.setOutlineColor(isActive ? sf::Color(100, 160, 240) : sf::Color(190, 210, 230));
        fieldBg.setOutlineThickness(isActive ? 2.5f : 1.5f);
        m_window.draw(fieldBg);

        sf::Text icon; icon.setFont(m_font); icon.setCharacterSize(16);
        icon.setFillColor(sf::Color(80, 130, 210));
        icon.setString(isSignUp ? fieldIcons[i] : loginIcons[i]);
        sf::FloatRect ib = icon.getLocalBounds();
        icon.setPosition(fieldX + 10.f, fy + (fieldH - ib.height) / 2.f - 2.f);
        m_window.draw(icon);

        const std::string& raw = m_authField[i];
        if (raw.empty())
        {
            sf::Text ph; ph.setFont(m_font); ph.setCharacterSize(15);
            ph.setFillColor(sf::Color(160, 180, 200));
            ph.setString(isSignUp ? fieldHints[i] : loginHints[i]);
            ph.setPosition(fieldX + 36.f, fy + (fieldH - 18.f) / 2.f);
            m_window.draw(ph);
        }
        else
        {
            std::string displayText;
            if (isPassField && !m_authShowPassword[eyeIdx])
                displayText = std::string(raw.size(), '*');
            else
                displayText = raw;
            if (isActive && m_authCursorVisible) displayText += "|";

            sf::Text ft; ft.setFont(m_font); ft.setCharacterSize(16);
            ft.setFillColor(sf::Color(40, 60, 100));
            ft.setString(displayText);
            ft.setPosition(fieldX + 36.f, fy + (fieldH - 20.f) / 2.f);
            m_window.draw(ft);
        }

        if (raw.empty() && isActive && m_authCursorVisible)
        {
            sf::RectangleShape cursor(sf::Vector2f(2.f, fieldH * 0.55f));
            cursor.setPosition(fieldX + 36.f, fy + fieldH * 0.22f);
            cursor.setFillColor(sf::Color(80, 130, 200));
            m_window.draw(cursor);
        }

        if (isPassField)
        {
            sf::Text eye; eye.setFont(m_font); eye.setCharacterSize(13);
            eye.setFillColor(sf::Color(80, 130, 210));
            eye.setString(m_authShowPassword[eyeIdx] ? "O-" : "O");
            eye.setPosition(fieldX + fieldW - 34.f, fy + (fieldH - 16.f) / 2.f);
            m_window.draw(eye);
        }
    }

    float btnW = fieldW, btnH = 46.f;
    float btnX = fieldX;
    float btnY = firstFieldY + fieldCount * (fieldH + fieldGap) + 4.f;

    sf::RectangleShape btnShadow(sf::Vector2f(btnW, btnH + 4.f));
    btnShadow.setPosition(btnX + 3.f, btnY + 5.f);
    btnShadow.setFillColor(sf::Color(180, 50, 100, 80));
    m_window.draw(btnShadow);

    sf::RectangleShape submitBtn(sf::Vector2f(btnW, btnH));
    submitBtn.setPosition(btnX, btnY);
    submitBtn.setFillColor(sf::Color(240, 90, 145, 240));
    submitBtn.setOutlineColor(sf::Color(255, 255, 255, 160));
    submitBtn.setOutlineThickness(2.f);
    m_window.draw(submitBtn);

    sf::RectangleShape shine(sf::Vector2f(btnW - 20.f, btnH * 0.38f));
    shine.setPosition(btnX + 10.f, btnY + 4.f);
    shine.setFillColor(sf::Color(255, 255, 255, 55));
    m_window.draw(shine);

    sf::Text btnLabel; btnLabel.setFont(m_font); btnLabel.setCharacterSize(20);
    btnLabel.setFillColor(sf::Color::White);
    btnLabel.setString(isSignUp ? "SIGN UP" : "LOGIN");
    sf::FloatRect blb = btnLabel.getLocalBounds();
    btnLabel.setOrigin(blb.left + blb.width / 2.f, blb.top + blb.height / 2.f);
    btnLabel.setPosition(btnX + btnW / 2.f, btnY + btnH / 2.f);
    m_window.draw(btnLabel);

    if (!isSignUp)
    {
        sf::Text guestLink; guestLink.setFont(m_font); guestLink.setCharacterSize(13);
        guestLink.setFillColor(sf::Color(100, 140, 210));
        guestLink.setString("Continue as Guest");
        sf::FloatRect glb = guestLink.getLocalBounds();
        guestLink.setOrigin(glb.left + glb.width / 2.f, 0.f);
        guestLink.setPosition(WINDOW_WIDTH / 2.f, btnY + btnH + 12.f);
        m_window.draw(guestLink);
    }

    {
        sf::Text switchLink; switchLink.setFont(m_font); switchLink.setCharacterSize(12);
        switchLink.setFillColor(sf::Color(100, 140, 210));
        switchLink.setString(isSignUp ? "Already have an account? LOGIN" : "New here? SIGN UP");
        sf::FloatRect slb = switchLink.getLocalBounds();
        switchLink.setOrigin(slb.left + slb.width / 2.f, 0.f);
        switchLink.setPosition(WINDOW_WIDTH / 2.f, cardY + cardH - 22.f);
        m_window.draw(switchLink);
    }

    if (!m_authError.empty())
    {
        sf::Text err; err.setFont(m_font); err.setCharacterSize(13);
        err.setFillColor(sf::Color(210, 50, 70)); err.setString(m_authError);
        sf::FloatRect eb = err.getLocalBounds();
        err.setOrigin(eb.left + eb.width / 2.f, 0.f);
        err.setPosition(WINDOW_WIDTH / 2.f, btnY + btnH + (isSignUp ? 10.f : 36.f));
        m_window.draw(err);
    }

    sf::Text back; back.setFont(m_font); back.setCharacterSize(18);
    back.setFillColor(sf::Color(255, 255, 255, 200)); back.setString("< Back");
    back.setPosition(16.f, 14.f);
    m_window.draw(back);
}

// drawMainMenu

void Game::drawMainMenu()
{
    m_window.draw(m_menuBgSprite);

    const float W = static_cast<float>(WINDOW_WIDTH);
    const float H = static_cast<float>(WINDOW_HEIGHT);
    const float bCX = W * 0.519f;
    const float bHW = W * 0.185f;
    const float rows[7] = { 0.33f, 0.40f, 0.47f, 0.53f, 0.60f, 0.67f, 0.73f };

    int sel = m_menuSelection;
    if (sel >= 0 && sel < 7)
    {
        sf::Text arrow; arrow.setFont(m_font); arrow.setCharacterSize(28);
        arrow.setFillColor(sf::Color::White); arrow.setString(">");
        sf::FloatRect ab = arrow.getLocalBounds();
        arrow.setOrigin(ab.width, ab.top + ab.height / 2.f);
        arrow.setPosition(bCX - bHW - 6.f, H * rows[sel]);
        m_window.draw(arrow);
    }

    std::string badge; sf::Color badgeColor;
    if (m_loggedIn)
    {
        badge = "Logged in: " + m_loggedInUser; badgeColor = sf::Color(120, 255, 180);
    }
    else if (m_enteredAsGuest)
    {
        badge = "Playing as Guest"; badgeColor = sf::Color(255, 220, 100);
    }
    if (!badge.empty())
    {
        sf::Text t; t.setFont(m_font); t.setCharacterSize(13);
        t.setFillColor(badgeColor); t.setString(badge);
        sf::FloatRect tb = t.getLocalBounds();
        t.setOrigin(tb.left + tb.width / 2.f, 0.f);
        t.setPosition(W / 2.f, 6.f);
        m_window.draw(t);
    }

    if (m_saveSlot.valid)
    {
        char buf[80];
        snprintf(buf, sizeof(buf), "Save: Lv%d  [L] Load", m_saveSlot.level);
        sf::Text t; t.setFont(m_font); t.setCharacterSize(12);
        t.setFillColor(sf::Color(160, 255, 160)); t.setString(buf);
        sf::FloatRect tb = t.getLocalBounds();
        t.setOrigin(tb.left + tb.width / 2.f, 0.f);
        t.setPosition(W / 2.f, static_cast<float>(WINDOW_HEIGHT) - 20.f);
        m_window.draw(t);
    }
}

void Game::drawDailyReward()
{
    m_window.draw(m_menuBgSprite);
    sf::RectangleShape ov(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    ov.setFillColor(sf::Color(0, 0, 0, 160)); m_window.draw(ov);

    float pw = 360.f, ph = 260.f;
    float px = (WINDOW_WIDTH - pw) / 2.f, py = (WINDOW_HEIGHT - ph) / 2.f;

    sf::RectangleShape panel(sf::Vector2f(pw, ph));
    panel.setPosition(px, py);
    panel.setFillColor(sf::Color(30, 10, 50, 245));
    panel.setOutlineColor(sf::Color(255, 210, 50));
    panel.setOutlineThickness(3.f);
    m_window.draw(panel);

    sf::Text title; title.setFont(m_font); title.setCharacterSize(24);
    title.setFillColor(sf::Color(255, 220, 50)); title.setString("DAILY REWARD");
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition(px + (pw - tb.width) / 2.f, py + 16.f);
    m_window.draw(title);

    sf::Text body; body.setFont(m_font); body.setCharacterSize(16);
    body.setFillColor(sf::Color::White);
    if (m_dailyRewardClaimed)
        body.setString("You already claimed\nyour reward today!\n\nCome back tomorrow.");
    else
    {
        body.setString("You received:\n\n  +50 GEMS\n  +1 EXTRA LIFE");
        m_dailyRewardClaimed = true; saveDailyRewardState();
    }
    sf::FloatRect blb = body.getLocalBounds();
    body.setPosition(px + (pw - blb.width) / 2.f, py + 70.f);
    m_window.draw(body);

    sf::Text hint; hint.setFont(m_font); hint.setCharacterSize(13);
    hint.setFillColor(sf::Color(180, 180, 200)); hint.setString("Click or press ENTER to close");
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setPosition(px + (pw - hb.width) / 2.f, py + ph - 28.f);
    m_window.draw(hint);
}

// drawAboutUs
void Game::drawAboutUs()
{
    m_window.draw(m_menuBgSprite);
    sf::RectangleShape ov(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    ov.setFillColor(sf::Color(0, 0, 0, 170)); m_window.draw(ov);

    float pw = 500.f, ph = 320.f;
    float px = (WINDOW_WIDTH - pw) / 2.f, py = (WINDOW_HEIGHT - ph) / 2.f;

    sf::RectangleShape panel(sf::Vector2f(pw, ph));
    panel.setPosition(px, py);
    panel.setFillColor(sf::Color(20, 8, 40, 245));
    panel.setOutlineColor(sf::Color(240, 80, 160));
    panel.setOutlineThickness(2.5f);
    m_window.draw(panel);

    sf::Text title; title.setFont(m_font); title.setCharacterSize(26);
    title.setFillColor(sf::Color(255, 140, 200)); title.setString("ABOUT US");
    sf::FloatRect tlb = title.getLocalBounds();
    title.setPosition(px + (pw - tlb.width) / 2.f, py + 14.f);
    m_window.draw(title);

    const char* lines[] = {
        "Snow Bros: Powerpuff Adventures","",
        "A fan-made Snow Bros reimagining",
        "featuring the Powerpuff Girls universe.","",
        "Built with C++ and SFML.",
        "Version 1.0   |   OOP Game Project",
    };
    float lineY = py + 60.f;
    for (const char* l : lines)
    {
        sf::Text lt; lt.setFont(m_font); lt.setCharacterSize(15);
        lt.setFillColor(sf::Color(210, 200, 230)); lt.setString(l);
        sf::FloatRect lb = lt.getLocalBounds();
        lt.setPosition(px + (pw - lb.width) / 2.f, lineY);
        m_window.draw(lt); lineY += 22.f;
    }

    sf::Text back; back.setFont(m_font); back.setCharacterSize(13);
    back.setFillColor(sf::Color(160, 160, 180)); back.setString("Click or ESC to go back");
    sf::FloatRect bb = back.getLocalBounds();
    back.setPosition(px + (pw - bb.width) / 2.f, py + ph - 26.f);
    m_window.draw(back);
}

// drawLeaderboard
void Game::drawLeaderboard()
{
    if (m_leaderboardBgTexture.getSize().x > 0) m_window.draw(m_leaderboardBgSprite);
    else m_window.draw(m_menuBgSprite);

    sf::RectangleShape ov(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    ov.setFillColor(sf::Color(0, 0, 0, 155)); m_window.draw(ov);

    float pw = 620.f, ph = 480.f;
    float px = (WINDOW_WIDTH - pw) / 2.f, py = (WINDOW_HEIGHT - ph) / 2.f;

    sf::RectangleShape panel(sf::Vector2f(pw, ph));
    panel.setPosition(px, py);
    panel.setFillColor(sf::Color(4, 10, 4, 248));
    panel.setOutlineColor(sf::Color(50, 200, 60));
    panel.setOutlineThickness(3.f);
    m_window.draw(panel);

    sf::Text title; title.setFont(m_font); title.setCharacterSize(26);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(80, 255, 90));
    title.setOutlineColor(sf::Color(0, 60, 0));
    title.setOutlineThickness(2.f);
    title.setString("GLOBAL LEADERBOARD  -  TOP 10");
    sf::FloatRect tlb = title.getLocalBounds();
    title.setPosition(px + (pw - tlb.width) / 2.f, py + 18.f);
    m_window.draw(title);

    sf::Text hdr; hdr.setFont(m_font); hdr.setCharacterSize(12);
    hdr.setFillColor(sf::Color(100, 220, 110));
    hdr.setString("RANK   PLAYER NAME         SCORE   LVL   DATE");
    hdr.setPosition(px + 20.f, py + 65.f);
    m_window.draw(hdr);

    const int DISPLAY_ROWS = 10;
    for (int i = 0; i < m_leaderboardCount && i < DISPLAY_ROWS; i++)
    {
        sf::RectangleShape rowBg(sf::Vector2f(pw - 20.f, 26.f));
        rowBg.setPosition(px + 10.f, py + 90.f + i * 30.f);
        rowBg.setFillColor(i % 2 == 0 ? sf::Color(8, 28, 8, 180) : sf::Color(12, 38, 12, 180));
        m_window.draw(rowBg);

        sf::Color rowCol = i == 0 ? sf::Color(255, 215, 0)
            : i == 1 ? sf::Color(200, 200, 210)
            : i == 2 ? sf::Color(180, 120, 60)
            : sf::Color(80, 200, 90);

        const char* dateStr = m_leaderboard[i].date;
        if (dateStr[0] == '\0') dateStr = "-";

        char row[128];
        snprintf(row, sizeof(row), " #%-2d   %-18s  %6d   %2d    %s",
            i + 1, m_leaderboard[i].name, m_leaderboard[i].score,
            m_leaderboard[i].level, dateStr);

        sf::Text rt; rt.setFont(m_font); rt.setCharacterSize(13);
        rt.setFillColor(rowCol); rt.setString(row);
        rt.setPosition(px + 18.f, py + 93.f + i * 30.f);
        m_window.draw(rt);
    }

    sf::Text back; back.setFont(m_font); back.setCharacterSize(13);
    back.setFillColor(sf::Color(70, 200, 80));
    back.setString("ESC  Back to Main Menu");
    sf::FloatRect bb = back.getLocalBounds();
    back.setPosition(px + (pw - bb.width) / 2.f, py + ph - 31.f);
    m_window.draw(back);
}

// drawPlaying // drawStatsPanel
void Game::drawPlaying()
{
    m_levelManager.draw(m_window, m_debugMode, m_players, m_playerCount);
    for (int i = 0; i < m_playerCount; i++)
    {
        m_players[i]->draw(m_window);
        if (m_debugMode) m_players[i]->drawDebug(m_window);
    }
    m_hud.draw(m_window, m_players, m_playerCount,
        m_levelManager.getCurrentLevel(), m_levelManager.getTotalLevels());
    drawStatsPanel();
    if (m_levelManager.isStarChoiceOpen())
        m_levelManager.drawStarChoiceUI(m_window, m_font);
}

void Game::drawStatsPanel()
{
    float px = 5.f, py = WINDOW_HEIGHT - 70.f;
    sf::RectangleShape panel(sf::Vector2f(200.f, 65.f));
    panel.setPosition(px, py);
    panel.setFillColor(sf::Color(0, 0, 0, 160));
    panel.setOutlineColor(sf::Color(100, 100, 100));
    panel.setOutlineThickness(1.f);
    m_window.draw(panel);

    for (int i = 0; i < m_playerCount; i++)
    {
        Player* p = m_players[i];
        char buf[64];
        snprintf(buf, sizeof(buf), "%s  Sc:%d  Gm:%d", p->getName(), p->getScore(), p->getGems());
        sf::Text txt; txt.setFont(m_font); txt.setCharacterSize(13);
        txt.setFillColor(i == 0 ? sf::Color(150, 180, 255) : sf::Color(255, 160, 100));
        txt.setString(buf); txt.setPosition(px + 5.f, py + 4.f + i * 28.f);
        m_window.draw(txt);

        for (int l = 0; l < p->getLives(); l++)
        {
            sf::CircleShape heart(5.f);
            heart.setFillColor(sf::Color(220, 40, 40));
            heart.setPosition(px + 5.f + l * 14.f, py + 18.f + i * 28.f);
            m_window.draw(heart);
        }
    }
}


// drawPaused
void Game::drawPaused()
{
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 155));
    m_window.draw(overlay);

    float pw = 360.f, ph = 320.f;
    float panelX = (WINDOW_WIDTH - pw) / 2.f;
    float panelY = (WINDOW_HEIGHT - ph) / 2.f;

    sf::RectangleShape panel(sf::Vector2f(pw, ph));
    panel.setPosition(panelX, panelY);
    panel.setFillColor(sf::Color(8, 15, 35, 240));
    panel.setOutlineColor(sf::Color(80, 140, 255));
    panel.setOutlineThickness(2.5f);
    m_window.draw(panel);

    sf::Text title; title.setFont(m_font); title.setCharacterSize(34);
    title.setFillColor(sf::Color(255, 215, 0)); title.setString("PAUSED");
    sf::FloatRect tlb = title.getLocalBounds();
    title.setPosition(panelX + (pw - tlb.width) / 2.f, panelY + 12.f);
    m_window.draw(title);

    const char* labels[PAUSE_MENU_ITEMS] = {
        "Resume","Open Shop","Save Game","Exit to Main Menu","Quit Game","Settings"
    };
    float itemY = panelY + 60.f;
    for (int i = 0; i < PAUSE_MENU_ITEMS; i++)
    {
        bool sel = (i == m_pauseSelection);
        if (sel)
        {
            sf::RectangleShape rowBg(sf::Vector2f(pw - 20.f, 34.f));
            rowBg.setPosition(panelX + 10.f, itemY - 3.f);
            rowBg.setFillColor(sf::Color(30, 60, 120, 200));
            rowBg.setOutlineColor(sf::Color(100, 180, 255));
            rowBg.setOutlineThickness(1.5f);
            m_window.draw(rowBg);
            sf::Text arrow; arrow.setFont(m_font); arrow.setCharacterSize(18);
            arrow.setFillColor(sf::Color(100, 200, 255)); arrow.setString(">");
            arrow.setPosition(panelX + 14.f, itemY);
            m_window.draw(arrow);
        }
        sf::Text item; item.setFont(m_font); item.setCharacterSize(19);
        if (sel)        item.setFillColor(sf::Color(255, 220, 60));
        else if (i == 1)  item.setFillColor(sf::Color(200, 160, 40));
        else if (i == 4)  item.setFillColor(sf::Color(220, 80, 80));
        else if (i == 5)  item.setFillColor(sf::Color(140, 180, 255));
        else            item.setFillColor(sf::Color(200, 200, 210));
        item.setString(labels[i]); item.setPosition(panelX + 36.f, itemY);
        m_window.draw(item); itemY += 40.f;
    }

    if (m_saveMsgTimer > 0.f && m_saveMsg[0] != '\0')
    {
        sf::Text sm; sm.setFont(m_font); sm.setCharacterSize(14);
        sm.setFillColor(sf::Color(80, 240, 120)); sm.setString(m_saveMsg);
        sf::FloatRect smb = sm.getLocalBounds();
        sm.setPosition(panelX + (pw - smb.width) / 2.f, panelY + ph - 50.f);
        m_window.draw(sm);
    }

    sf::Text hint; hint.setFont(m_font); hint.setCharacterSize(13);
    hint.setFillColor(sf::Color(90, 90, 110));
    hint.setString("UP/DOWN navigate   ENTER confirm   ESC/P resume");
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setPosition((WINDOW_WIDTH - hb.width) / 2.f, WINDOW_HEIGHT - 22.f);
    m_window.draw(hint);
}

// drawLevelComplete
void Game::drawLevelComplete()
{
    drawPlaying();
    sf::RectangleShape ov(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    ov.setFillColor(sf::Color(0, 0, 0, 130)); m_window.draw(ov);

    sf::Text txt; txt.setFont(m_font); txt.setCharacterSize(44);
    txt.setFillColor(sf::Color(255, 220, 50)); txt.setString("LEVEL COMPLETE!");
    sf::FloatRect tlb = txt.getLocalBounds();
    txt.setPosition((WINDOW_WIDTH - tlb.width) / 2.f, WINDOW_HEIGHT / 2.f - 80.f);
    m_window.draw(txt);

    char buf[64];
    for (int i = 0; i < m_playerCount; i++)
    {
        snprintf(buf, sizeof(buf), "%s  Score: %d   Gems: %d",
            m_players[i]->getName(), m_players[i]->getScore(), m_players[i]->getGems());
        sf::Text stat; stat.setFont(m_font); stat.setCharacterSize(20);
        stat.setFillColor(sf::Color::White); stat.setString(buf);
        sf::FloatRect slb = stat.getLocalBounds();
        stat.setPosition((WINDOW_WIDTH - slb.width) / 2.f, WINDOW_HEIGHT / 2.f - 10.f + i * 28.f);
        m_window.draw(stat);
    }

    sf::Text hint; hint.setFont(m_font); hint.setCharacterSize(17);
    hint.setFillColor(sf::Color(180, 180, 180));
    hint.setString("ENTER  Next level    S  Gem Shop");
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setPosition((WINDOW_WIDTH - hb.width) / 2.f, WINDOW_HEIGHT / 2.f + 70.f);
    m_window.draw(hint);
}

// drawGameOver
void Game::drawGameOver()
{
    if (m_gameOverBgTexture.getSize().x > 0) m_window.draw(m_gameOverBgSprite);
    else {
        sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        bg.setFillColor(sf::Color(10, 0, 0)); m_window.draw(bg);
    }
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 120)); m_window.draw(overlay);

    sf::Text txt; txt.setFont(m_font); txt.setCharacterSize(64);
    txt.setFillColor(sf::Color(220, 30, 30));
    txt.setOutlineColor(sf::Color(0, 0, 0, 200)); txt.setOutlineThickness(4.f);
    txt.setString("GAME OVER");
    sf::FloatRect tlb = txt.getLocalBounds();
    txt.setOrigin(tlb.left + tlb.width / 2.f, tlb.top + tlb.height / 2.f);
    txt.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT * 0.28f);
    m_window.draw(txt);

    char buf[80];
    for (int i = 0; i < m_playerCount; i++)
    {
        snprintf(buf, sizeof(buf), "%s   Score: %d   Gems: %d",
            m_players[i]->getName(), m_players[i]->getScore(), m_players[i]->getGems());
        sf::Text stat; stat.setFont(m_font); stat.setCharacterSize(22);
        stat.setFillColor(sf::Color(255, 180, 180));
        stat.setOutlineColor(sf::Color(0, 0, 0, 160)); stat.setOutlineThickness(2.f);
        stat.setString(buf);
        sf::FloatRect slb = stat.getLocalBounds();
        stat.setOrigin(slb.left + slb.width / 2.f, 0.f);
        stat.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT * 0.52f + i * 32.f);
        m_window.draw(stat);
    }

    float barW = 520.f, barH = 52.f;
    float barX = (WINDOW_WIDTH - barW) / 2.f, barY = WINDOW_HEIGHT * 0.72f;
    sf::RectangleShape bar(sf::Vector2f(barW, barH));
    bar.setPosition(barX, barY);
    bar.setFillColor(sf::Color(80, 0, 0, 200));
    bar.setOutlineColor(sf::Color(200, 60, 60, 200));
    bar.setOutlineThickness(2.f);
    m_window.draw(bar);

    sf::Text opts; opts.setFont(m_font); opts.setCharacterSize(19);
    opts.setFillColor(sf::Color(255, 220, 220));
    opts.setString("C  Try Again (same level)      M  Main Menu");
    sf::FloatRect olb = opts.getLocalBounds();
    opts.setOrigin(olb.left + olb.width / 2.f, olb.top + olb.height / 2.f);
    opts.setPosition(WINDOW_WIDTH / 2.f, barY + barH / 2.f);
    m_window.draw(opts);
}

// drawVictory
void Game::drawVictory()
{
    if (m_victoryBgTexture.getSize().x > 0) m_window.draw(m_victoryBgSprite);
    else {
        sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        bg.setFillColor(sf::Color(20, 10, 40)); m_window.draw(bg);
    }
    sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    overlay.setFillColor(sf::Color(0, 0, 0, 100)); m_window.draw(overlay);

    sf::Text title; title.setFont(m_font); title.setCharacterSize(72);
    title.setFillColor(sf::Color(255, 220, 40));
    title.setOutlineColor(sf::Color(120, 60, 0, 220)); title.setOutlineThickness(5.f);
    title.setString("YOU WIN!");
    sf::FloatRect tlb = title.getLocalBounds();
    title.setOrigin(tlb.left + tlb.width / 2.f, tlb.top + tlb.height / 2.f);
    title.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT * 0.26f);
    m_window.draw(title);

    sf::Text sub; sub.setFont(m_font); sub.setCharacterSize(22);
    sub.setFillColor(sf::Color(255, 255, 200));
    sub.setOutlineColor(sf::Color(0, 0, 0, 160)); sub.setOutlineThickness(2.f);
    sub.setString("You defeated all levels - Townsville is saved!");
    sf::FloatRect slb2 = sub.getLocalBounds();
    sub.setOrigin(slb2.left + slb2.width / 2.f, 0.f);
    sub.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT * 0.40f);
    m_window.draw(sub);

    char buf[80];
    for (int i = 0; i < m_playerCount; i++)
    {
        snprintf(buf, sizeof(buf), "%s   Final Score: %d   Gems: %d",
            m_players[i]->getName(), m_players[i]->getScore(), m_players[i]->getGems());
        sf::Text stat; stat.setFont(m_font); stat.setCharacterSize(22);
        stat.setFillColor(i == 0 ? sf::Color(150, 220, 255) : sf::Color(255, 180, 100));
        stat.setOutlineColor(sf::Color(0, 0, 0, 160)); stat.setOutlineThickness(2.f);
        stat.setString(buf);
        sf::FloatRect sb = stat.getLocalBounds();
        stat.setOrigin(sb.left + sb.width / 2.f, 0.f);
        stat.setPosition(WINDOW_WIDTH / 2.f, WINDOW_HEIGHT * 0.52f + i * 34.f);
        m_window.draw(stat);
    }

    float barW = 500.f, barH = 52.f;
    float barX = (WINDOW_WIDTH - barW) / 2.f, barY = WINDOW_HEIGHT * 0.75f;
    sf::RectangleShape bar(sf::Vector2f(barW, barH));
    bar.setPosition(barX, barY);
    bar.setFillColor(sf::Color(60, 40, 0, 200));
    bar.setOutlineColor(sf::Color(255, 200, 40, 200));
    bar.setOutlineThickness(2.f);
    m_window.draw(bar);

    sf::Text opts; opts.setFont(m_font); opts.setCharacterSize(19);
    opts.setFillColor(sf::Color(255, 240, 180));
    opts.setString("ENTER  View Leaderboard      M  Main Menu");
    sf::FloatRect olb = opts.getLocalBounds();
    opts.setOrigin(olb.left + olb.width / 2.f, olb.top + olb.height / 2.f);
    opts.setPosition(WINDOW_WIDTH / 2.f, barY + barH / 2.f);
    m_window.draw(opts);
}

// Game logic helpers


void Game::startNewGame(int numPlayers)
{
    clearPlayers();
    m_players[0] = new Player(100.f, 500.f, 0, "Nick");
    m_playerCount = 1;
    if (numPlayers >= 2)
    {
        m_players[1] = new Player(650.f, 500.f, 1, "Tom"); m_playerCount = 2;
    }
    applySettingsBindings();
    m_levelManager.loadLevel(1);
    loadLevelMusic(1);
    m_state = GameState::Playing;
}

void Game::startFromCharacterSelect()
{
    clearPlayers();
    m_playerCount = m_pendingPlayerCount;
    float spawnX[2] = { 100.f, 650.f };
    float spawnY[2] = { 500.f, 500.f };

    for (int i = 0; i < m_playerCount; i++)
    {
        int charIdx = m_charSelect.getSelectedCharacter(i);
        m_selectedCharIdx[i] = charIdx;
        const CharacterData* cd = &m_charSelect.getCharacterData(charIdx);
        m_players[i] = new Player(spawnX[i], spawnY[i], i, cd);
    }
    applySettingsBindings();
    int startLevel = (m_levelSelectChoice >= 1) ? m_levelSelectChoice : 1;
    m_levelManager.loadLevel(startLevel);
    playIntroVideo();
}


void Game::advanceToNextLevel()
{
    int next = m_levelManager.getCurrentLevel() + 1;
    if (next > m_levelManager.getTotalLevels())
    {
        if (m_loggedIn && !m_loggedInUser.empty())
        {
            for (int i = 0; i < m_playerCount; i++)
                submitScore(m_loggedInUser.c_str(), m_players[i]->getScore(),
                    m_players[i]->getGems(), m_levelManager.getCurrentLevel());
        }
        m_endSoundPlayed = false;
        playOutroVideo();
        return;
    }

    m_levelManager.resetLevelPowerUps(m_players, m_playerCount);
    m_levelManager.loadLevel(next);
    loadLevelMusic(next);
    m_state = GameState::Playing;
}
void Game::loadLevelMusic(int levelNumber)
{
    m_currentBgmPath.clear();
    const char* path = "assets/music/LEVELS.WAV";
    if (levelNumber == 5)       path = "assets/music/NINJA.WAV";
    else if (levelNumber == 10) path = "assets/music/NINJA.WAV";
    else if (levelNumber >= 6)  path = "assets/music/LEVELS.WAV";
    loadMusic(path);
}

void Game::loadMusic(const char* path)
{
    m_music.stop();
    m_currentBgmPath = path;
    if (m_music.openFromFile(path))
    {
        m_music.setLoop(true);
        m_music.setVolume(m_settings.getMusicVolume());
        m_music.play();
    }
}

void Game::clearPlayers()
{
    for (int i = 0; i < MAX_PLAYERS; i++) { delete m_players[i]; m_players[i] = nullptr; }
    m_playerCount = 0;
}

void Game::applySettingsBindings()
{
    const KeyBindings& kb = m_settings.getBindings();
    for (int i = 0; i < m_playerCount; i++)
    {
        if (i == 0)
            m_players[i]->applyKeyBindings(
                kb.get(KeySlot::P1Left), kb.get(KeySlot::P1Right),
                kb.get(KeySlot::P1Jump), kb.get(KeySlot::P1Throw));
        else
            m_players[i]->applyKeyBindings(
                kb.get(KeySlot::P2Left), kb.get(KeySlot::P2Right),
                kb.get(KeySlot::P2Jump), kb.get(KeySlot::P2Throw));
    }
}

void Game::saveGame()
{
    if (m_playerCount == 0)
    {
        snprintf(m_saveMsg, sizeof(m_saveMsg), "Nothing to save!"); m_saveMsgTimer = 2.f; return;
    }
    m_saveSlot.valid = true;
    m_saveSlot.level = m_levelManager.getCurrentLevel();
    m_saveSlot.playerCount = m_playerCount;
    for (int i = 0; i < m_playerCount; i++)
    {
        m_saveSlot.playerData[i] = m_players[i]->getSaveData(m_selectedCharIdx[i]);
        m_saveSlot.charIndex[i] = m_selectedCharIdx[i];
    }
    saveGameToFile();
    snprintf(m_saveMsg, sizeof(m_saveMsg), "Game saved!  (Level %d)", m_saveSlot.level);
    m_saveMsgTimer = 2.5f;
}

bool Game::loadGame()
{
    if (!m_saveSlot.valid) return false;
    clearPlayers();
    m_playerCount = m_saveSlot.playerCount;
    float spawnX[2] = { 100.f, 650.f };
    float spawnY[2] = { 500.f, 500.f };
    for (int i = 0; i < m_playerCount; i++)
    {
        int ci = m_saveSlot.charIndex[i];
        m_selectedCharIdx[i] = ci;
        const CharacterData* cd = &m_charSelect.getCharacterData(ci);
        m_players[i] = new Player(spawnX[i], spawnY[i], i, cd);
        m_players[i]->applyRestoreData(m_saveSlot.playerData[i]);
    }
    applySettingsBindings();
    m_levelManager.loadLevel(m_saveSlot.level);
    loadLevelMusic(m_saveSlot.level);
    m_state = GameState::Playing;
    return true;
}

void Game::initMainMenu()
{
    m_menuTitle.setFont(m_font);
    m_menuTitle.setFillColor(sf::Color(0, 0, 0, 0));
    m_menuTitle.setString(" ");

    const float cx = WINDOW_WIDTH / 2.f;
    const float cy = WINDOW_HEIGHT / 2.f;
    const float sx = static_cast<float>(WINDOW_WIDTH) / 800.f;
    const float sy = static_cast<float>(WINDOW_HEIGHT) / 600.f;
    const float btnW = 300.f * sx, btnH = 56.f * sy;
    const float gap = 68.f * sy;
    const float startY = cy - 30.f * sy;

    for (int i = 0; i < 3; i++)
    {
        m_centreButtons[i].setSize(sf::Vector2f(btnW, btnH));
        m_centreButtons[i].setOrigin(btnW / 2.f, btnH / 2.f);
        m_centreButtons[i].setPosition(cx, startY + i * gap);
    }
    m_menuSelection = 0;
}

void Game::initPauseMenu() {}


void Game::handleLevelSelectEvents(const sf::Event& event)
{
    int totalLevels = m_levelManager.getTotalLevels();
    if (totalLevels < 1) totalLevels = 10;

    if (m_howToPlayOpen)
    {
        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Escape) m_howToPlayOpen = false;
            else if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::A)
            {
                if (m_howToPlayPage > 0) m_howToPlayPage--;
            }
            else if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::D)
            {
                if (m_howToPlayPage < 4) m_howToPlayPage++;
            }
        }
        if (event.type == sf::Event::MouseButtonPressed &&
            event.mouseButton.button == sf::Mouse::Left)
        {
            const float W = static_cast<float>(WINDOW_WIDTH);
            const float H = static_cast<float>(WINDOW_HEIGHT);
            const float panW = 680.f, panH = 490.f;
            const float panX = (W - panW) / 2.f, panY = (H - panH) / 2.f;
            float mx = static_cast<float>(event.mouseButton.x);
            float my = static_cast<float>(event.mouseButton.y);
            if (mx >= panX + panW - 48.f && mx <= panX + panW - 10.f && my >= panY + 7.f && my <= panY + 45.f)
                m_howToPlayOpen = false;
            else if (m_howToPlayPage > 0 &&
                mx >= panX + 16.f && mx <= panX + 116.f &&
                my >= panY + panH - 54.f && my <= panY + panH - 20.f)
                m_howToPlayPage--;
            else if (m_howToPlayPage < 4 &&
                mx >= panX + panW - 116.f && mx <= panX + panW - 16.f &&
                my >= panY + panH - 54.f && my <= panY + panH - 20.f)
                m_howToPlayPage++;
        }
        return;
    }

    if (event.type == sf::Event::KeyPressed)
    {
        if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::A)
        {
            if (m_levelSelectChoice > 1) m_levelSelectChoice--;
        }
        else if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::D)
        {
            if (m_levelSelectChoice < totalLevels) m_levelSelectChoice++;
        }
        else if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W)
        {
            m_levelSelectChoice -= 5; if (m_levelSelectChoice < 1) m_levelSelectChoice = 1;
        }
        else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S)
        {
            m_levelSelectChoice += 5; if (m_levelSelectChoice > totalLevels) m_levelSelectChoice = totalLevels;
        }
        else if (event.key.code == sf::Keyboard::Return ||
            event.key.code == sf::Keyboard::Num1 ||
            event.key.code == sf::Keyboard::Numpad1)
        {
            m_pendingPlayerCount = 1; m_charSelect.reset(1); m_state = GameState::CharacterSelect;
        }
        else if (event.key.code == sf::Keyboard::Num2 || event.key.code == sf::Keyboard::Numpad2)
        {
            m_pendingPlayerCount = 2; m_charSelect.reset(2); m_state = GameState::CharacterSelect;
        }
        else if (event.key.code == sf::Keyboard::Escape)
            m_state = GameState::MainMenu;
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        sf::Vector2f m(static_cast<float>(event.mouseButton.x),
            static_cast<float>(event.mouseButton.y));

        const float cellW = 110.f, cellH = 70.f, gap = 10.f;
        const float gridX = (WINDOW_WIDTH - (5 * cellW + 4 * gap)) / 2.f;
        const float gridY = 160.f;
        const int   cols = 5;

        int total = m_levelManager.getTotalLevels();
        if (total < 1) total = 10;

        for (int i = 0; i < total; i++)
        {
            int col = i % cols, row = i / cols;
            float cx = gridX + col * (cellW + gap);
            float cy = gridY + row * (cellH + gap);
            if (m.x >= cx && m.x <= cx + cellW && m.y >= cy && m.y <= cy + cellH)
                m_levelSelectChoice = i + 1;
        }

        float btnX = WINDOW_WIDTH / 2.f - 120.f, btnY = 430.f;
        if (m.x >= btnX && m.x <= btnX + 240.f && m.y >= btnY && m.y <= btnY + 50.f)
        {
            m_pendingPlayerCount = 1; m_charSelect.reset(1); m_state = GameState::CharacterSelect;
        }

        float btn2X = WINDOW_WIDTH / 2.f - 120.f, btn2Y = 490.f;
        if (m.x >= btn2X && m.x <= btn2X + 240.f && m.y >= btn2Y && m.y <= btn2Y + 50.f)
        {
            m_pendingPlayerCount = 2; m_charSelect.reset(2); m_state = GameState::CharacterSelect;
        }

        if (m.x < 100.f && m.y < 50.f) m_state = GameState::MainMenu;

        const float htpW = 130.f, htpH = 46.f;
        const float htpX = static_cast<float>(WINDOW_WIDTH) - htpW - 14.f, htpY = 492.f;
        if (m.x >= htpX && m.x <= htpX + htpW && m.y >= htpY && m.y <= htpY + htpH)
        {
            m_howToPlayOpen = true; m_howToPlayPage = 0;
        }
    }
}

// drawLevelSelect

void Game::drawLevelSelect()
{
    if (m_levelSelectBgTexture.getSize().x > 0) m_window.draw(m_levelSelectBgSprite);
    else m_window.draw(m_menuBgSprite);

    sf::RectangleShape ov(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    ov.setFillColor(sf::Color(10, 5, 30, 130)); m_window.draw(ov);

    const float W = static_cast<float>(WINDOW_WIDTH);
    const float H = static_cast<float>(WINDOW_HEIGHT);

    sf::Text title; title.setFont(m_font); title.setCharacterSize(32);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(255, 255, 255));
    title.setString("SELECT LEVEL");
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition((W - tb.width) / 2.f, 173.f);
    m_window.draw(title);

    const float cellW = 108.f, cellH = 68.f, gap = 10.f;
    const int   cols = 5;
    const float gridX = (W - (cols * cellW + (cols - 1) * gap)) / 2.f;
    const float gridY = 255.f;

    int total = m_levelManager.getTotalLevels();
    if (total < 1) total = 10;

    for (int i = 0; i < total; i++)
    {
        int col = i % cols, row = i / cols;
        float cx = gridX + col * (cellW + gap);
        float cy = gridY + row * (cellH + gap);
        bool selected = (i + 1 == m_levelSelectChoice);
        bool isBoss = (i + 1 == 5 || i + 1 == 10);
        bool isBonus = (i + 1 == 4 || i + 1 == 9);

        sf::RectangleShape cell(sf::Vector2f(cellW, cellH));
        cell.setPosition(cx, cy);
        if (selected)
        {
            cell.setFillColor(sf::Color(255, 100, 180, 240));
            cell.setOutlineColor(sf::Color(255, 255, 255));
            cell.setOutlineThickness(3.f);
        }
        else if (isBoss)
        {
            cell.setFillColor(sf::Color(120, 20, 60, 210));
            cell.setOutlineColor(sf::Color(255, 80, 120, 200));
            cell.setOutlineThickness(2.f);
        }
        else
        {
            cell.setFillColor(sf::Color(40, 80, 180, 200));
            cell.setOutlineColor(sf::Color(130, 200, 255, 200));
            cell.setOutlineThickness(2.f);
        }
        m_window.draw(cell);

        sf::Text lvlText; lvlText.setFont(m_font);
        lvlText.setCharacterSize(selected ? 24 : 20);
        lvlText.setStyle(sf::Text::Bold);
        lvlText.setFillColor(selected ? sf::Color(255, 255, 255)
            : (isBoss ? sf::Color(255, 180, 200) : sf::Color(210, 235, 255)));
        char buf[8]; snprintf(buf, sizeof(buf), "%d", i + 1);
        lvlText.setString(buf);
        sf::FloatRect lb = lvlText.getLocalBounds();
        lvlText.setPosition(cx + (cellW - lb.width) / 2.f,
            cy + (isBonus || isBoss ? 10.f : (cellH - lb.height) / 2.f - 6.f));
        m_window.draw(lvlText);

        if (isBoss) {
            sf::Text bl; bl.setFont(m_font); bl.setCharacterSize(9);
            bl.setFillColor(sf::Color(255, 140, 80)); bl.setString("BOSS");
            sf::FloatRect blb = bl.getLocalBounds();
            bl.setPosition(cx + (cellW - blb.width) / 2.f, cy + cellH - 20.f);
            m_window.draw(bl);
        }
        if (isBonus) {
            sf::Text bn; bn.setFont(m_font); bn.setCharacterSize(9);
            bn.setFillColor(sf::Color(200, 255, 160)); bn.setString("BONUS");
            sf::FloatRect bnb = bn.getLocalBounds();
            bn.setPosition(cx + (cellW - bnb.width) / 2.f, cy + cellH - 20.f);
            m_window.draw(bn);
        }

        if (i + 1 == m_levelManager.getStarLevelNumber())
        {
            const int pts = 10;
            sf::ConvexShape star;
            star.setPointCount(pts);
            float scx = cx + cellW - 14.f;
            float scy = cy + 13.f;
            float outerR = 10.f, innerR = 4.5f;
            for (int p = 0; p < pts; p++)
            {
                float angle = (p * 3.14159f / (pts / 2)) - 3.14159f / 2.f;
                float r = (p % 2 == 0) ? outerR : innerR;
                star.setPoint(p, sf::Vector2f(scx + r * cosf(angle), scy + r * sinf(angle)));
            }
            star.setFillColor(sf::Color(255, 220, 30));
            star.setOutlineColor(sf::Color(200, 120, 0));
            star.setOutlineThickness(1.f);
            m_window.draw(star);
        }
    }

    // 1 Player button
    {
        float bW = 200.f, bH = 46.f, bX = W / 2.f - bW - 10.f, bY = 492.f;
        sf::RectangleShape btn(sf::Vector2f(bW, bH)); btn.setPosition(bX, bY);
        btn.setFillColor(sf::Color(230, 60, 140, 235));
        btn.setOutlineColor(sf::Color(255, 200, 230));
        btn.setOutlineThickness(2.5f); m_window.draw(btn);
        sf::Text bt; bt.setFont(m_font); bt.setCharacterSize(16); bt.setStyle(sf::Text::Bold);
        bt.setFillColor(sf::Color(255, 255, 255)); bt.setString("1 Player  (Enter)");
        sf::FloatRect bl = bt.getLocalBounds();
        bt.setPosition(bX + (bW - bl.width) / 2.f, bY + (bH - bl.height) / 2.f - 4.f);
        m_window.draw(bt);
    }
    // 2 Players button
    {
        float bW = 200.f, bH = 46.f, bX = W / 2.f + 10.f, bY = 492.f;
        sf::RectangleShape btn(sf::Vector2f(bW, bH)); btn.setPosition(bX, bY);
        btn.setFillColor(sf::Color(50, 120, 230, 235));
        btn.setOutlineColor(sf::Color(160, 210, 255));
        btn.setOutlineThickness(2.5f); m_window.draw(btn);
        sf::Text bt; bt.setFont(m_font); bt.setCharacterSize(16); bt.setStyle(sf::Text::Bold);
        bt.setFillColor(sf::Color(255, 255, 255)); bt.setString("2 Players  (Key 2)");
        sf::FloatRect bl = bt.getLocalBounds();
        bt.setPosition(bX + (bW - bl.width) / 2.f, bY + (bH - bl.height) / 2.f - 4.f);
        m_window.draw(bt);
    }
    // How to Play button
    {
        const float htpW = 130.f, htpH = 46.f;
        const float htpX = W - htpW - 14.f, htpY = 492.f;

        sf::RectangleShape shadow(sf::Vector2f(htpW, htpH));
        shadow.setPosition(htpX + 3.f, htpY + 3.f);
        shadow.setFillColor(sf::Color(0, 80, 0, 120));
        m_window.draw(shadow);

        sf::RectangleShape htpBtn(sf::Vector2f(htpW, htpH));
        htpBtn.setPosition(htpX, htpY);
        htpBtn.setFillColor(sf::Color(30, 160, 80, 240));
        htpBtn.setOutlineColor(sf::Color(160, 255, 180, 230));
        htpBtn.setOutlineThickness(2.5f);
        m_window.draw(htpBtn);

        sf::RectangleShape stripe(sf::Vector2f(htpW, 6.f));
        stripe.setPosition(htpX, htpY);
        stripe.setFillColor(sf::Color(255, 100, 180, 220));
        m_window.draw(stripe);

        sf::Text htpTxt; htpTxt.setFont(m_font); htpTxt.setCharacterSize(14);
        htpTxt.setStyle(sf::Text::Bold); htpTxt.setFillColor(sf::Color(255, 255, 255));
        htpTxt.setString("? How to Play");
        sf::FloatRect htpLb = htpTxt.getLocalBounds();
        htpTxt.setPosition(htpX + (htpW - htpLb.width) / 2.f, htpY + (htpH - htpLb.height) / 2.f - 2.f);
        m_window.draw(htpTxt);
    }

    if (m_howToPlayOpen) drawHowToPlay();
}

// drawHowToPlay

void Game::drawHowToPlay()
{
    const float W = static_cast<float>(WINDOW_WIDTH);
    const float H = static_cast<float>(WINDOW_HEIGHT);

    sf::RectangleShape backdrop(sf::Vector2f(W, H));
    backdrop.setFillColor(sf::Color(10, 0, 15, 210));
    m_window.draw(backdrop);

    const float panW = 680.f, panH = 490.f;
    const float panX = (W - panW) / 2.f, panY = (H - panH) / 2.f;

    sf::RectangleShape glow(sf::Vector2f(panW + 8.f, panH + 8.f));
    glow.setPosition(panX - 4.f, panY - 4.f);
    glow.setFillColor(sf::Color::Transparent);
    glow.setOutlineColor(sf::Color(80, 255, 140, 140));
    glow.setOutlineThickness(4.f);
    m_window.draw(glow);

    sf::RectangleShape panel(sf::Vector2f(panW, panH));
    panel.setPosition(panX, panY);
    panel.setFillColor(sf::Color(28, 8, 28, 252));
    panel.setOutlineColor(sf::Color(255, 80, 160, 230));
    panel.setOutlineThickness(3.f);
    m_window.draw(panel);

    sf::RectangleShape topL(sf::Vector2f(panW / 2.f, 52.f));
    topL.setPosition(panX, panY);
    topL.setFillColor(sf::Color(220, 50, 130, 240));
    m_window.draw(topL);

    sf::RectangleShape topR(sf::Vector2f(panW / 2.f, 52.f));
    topR.setPosition(panX + panW / 2.f, panY);
    topR.setFillColor(sf::Color(30, 170, 90, 240));
    m_window.draw(topR);

    sf::RectangleShape headerLine(sf::Vector2f(panW, 3.f));
    headerLine.setPosition(panX, panY + 52.f);
    headerLine.setFillColor(sf::Color(255, 220, 240, 200));
    m_window.draw(headerLine);

    sf::Text title; title.setFont(m_font); title.setCharacterSize(19);
    title.setStyle(sf::Text::Bold); title.setFillColor(sf::Color(255, 255, 255));
    title.setString("HOW TO PLAY  --  Snow Bros Powerpuff Adventures");
    sf::FloatRect tlb = title.getLocalBounds();
    title.setPosition(panX + (panW - tlb.width) / 2.f, panY + 12.f);
    m_window.draw(title);

    struct HTPLine { const char* text; bool isSub; };
    struct HTPPage { const char* heading; HTPLine lines[10]; int lineCount; };
    static const HTPPage pages[5] =
    {
        { "Controls", {
            {"Player 1:", false},
            {"  Move Left / Right  :  A / D  or  Left / Right Arrow", true},
            {"  Jump               :  W  or  Up Arrow", true},
            {"  Throw Snowball     :  Space", true},
            {"Player 2:", false},
            {"  Move Left / Right  :  Arrow Left / Right", true},
            {"  Jump               :  Arrow Up", true},
            {"  Throw Snowball     :  Numpad 0", true},
            {"", false}, {"", false} }, 8 },
        { "Objective", {
            {"Freeze every enemy on the stage using snowballs.", false},
            {"Then kick the frozen enemy to defeat it.", false},
            {"Roll frozen enemies into others for combo kills!", false},
            {"Combos award large score bonuses.", false},
            {"Clear ALL enemies to complete the level.", false},
            {"Collect gems dropped by defeated enemies.", false},
            {"Spend gems in the Gem Shop to upgrade your run.", false},
            {"", false}, {"", false}, {"", false} }, 7 },
        { "Power-ups & Shop", {
            {"Open the Gem Shop from the pause menu or main menu.", false},
            {"Available upgrades:", false},
            {"  Extra Life      -- gain one additional life", true},
            {"  Speed Boost     -- move faster for 30 seconds", true},
            {"  Power Snowball  -- freeze enemies in one hit", true},
            {"  Max Range       -- throw snowballs further", true},
            {"  Balloon Mode    -- float over hazards for 30 s", true},
            {"", false}, {"", false}, {"", false} }, 7 },
        { "Levels & Bosses", {
            {"There are 10 levels total.", false},
            {"Level 4 / 9  BONUS: collect items, no enemies.", false},
            {"Level 5      BOSS: Mogera.", false},
            {"  Hit with powered snowballs many times to win.", true},
            {"Level 10     FINAL BOSS: Gamakichi.", false},
            {"  Dodge his children and strike the boss directly!", true},
            {"", false}, {"", false}, {"", false}, {"", false} }, 6 },
        { "Tips & Tricks", {
            {"Chain-freeze multiple enemies with one rolling snowball", false},
            {"for huge score multipliers.", false},
            {"Claim your Daily Login Reward for free gems every day.", false},
            {"Save your game from the pause menu to continue later.", false},
            {"Check the Leaderboard to compare your best run.", false},
            {"Press ESC at any time to pause or go back.", false},
            {"Bosses need multiple powered hits -- stock up first!", false},
            {"", false}, {"", false}, {"", false} }, 7 },
    };

    static const int PAGE_COUNT = 5;
    int page = m_howToPlayPage;
    if (page < 0) page = 0;
    if (page >= PAGE_COUNT) page = PAGE_COUNT - 1;
    const HTPPage& cur = pages[page];

    for (int d = 0; d < PAGE_COUNT; d++)
    {
        sf::CircleShape dot(5.f);
        dot.setOrigin(5.f, 5.f);
        float dotX = panX + panW / 2.f + (d - PAGE_COUNT / 2) * 18.f;
        dot.setPosition(dotX, panY + 72.f);
        if (d == page)
        {
            dot.setFillColor(sf::Color(255, 100, 180));
            dot.setOutlineColor(sf::Color(255, 200, 230));
            dot.setOutlineThickness(2.f);
        }
        else
        {
            dot.setFillColor(sf::Color(60, 180, 100, 160));
            dot.setOutlineColor(sf::Color::Transparent);
            dot.setOutlineThickness(0.f);
        }
        m_window.draw(dot);
    }

    sf::Text heading; heading.setFont(m_font); heading.setCharacterSize(19);
    heading.setStyle(sf::Text::Bold); heading.setFillColor(sf::Color(80, 255, 150));
    heading.setString(cur.heading);
    sf::FloatRect hlb = heading.getLocalBounds();
    heading.setPosition(panX + (panW - hlb.width) / 2.f, panY + 82.f);
    m_window.draw(heading);

    sf::RectangleShape underline(sf::Vector2f(hlb.width + 20.f, 2.f));
    underline.setPosition(panX + (panW - hlb.width) / 2.f - 10.f, panY + 82.f + hlb.height + 6.f);
    underline.setFillColor(sf::Color(255, 80, 160, 180));
    m_window.draw(underline);

    const float lineStartY = panY + 118.f, lineStep = 34.f;
    for (int i = 0; i < cur.lineCount; i++)
    {
        if (cur.lines[i].text[0] == '\0') continue;
        bool sub = cur.lines[i].isSub;
        sf::Text ln; ln.setFont(m_font);
        ln.setCharacterSize(sub ? 13 : 15);
        ln.setFillColor(sub ? sf::Color(130, 230, 160) : sf::Color(255, 220, 240));
        ln.setString(cur.lines[i].text);
        float textX = panX + (sub ? 48.f : 30.f);
        ln.setPosition(textX, lineStartY + i * lineStep);
        m_window.draw(ln);

        if (!sub)
        {
            sf::CircleShape bullet(4.f, 3);
            bullet.setOrigin(4.f, 4.f);
            bullet.setPosition(panX + 18.f, lineStartY + i * lineStep + 8.f);
            bullet.setFillColor(sf::Color(255, 100, 180));
            m_window.draw(bullet);
        }
    }

    if (page > 0) {
        sf::RectangleShape prevBtn(sf::Vector2f(100.f, 34.f));
        prevBtn.setPosition(panX + 16.f, panY + panH - 54.f);
        prevBtn.setFillColor(sf::Color(200, 40, 110, 220));
        prevBtn.setOutlineColor(sf::Color(255, 160, 210));
        prevBtn.setOutlineThickness(2.f);
        m_window.draw(prevBtn);
        sf::Text prevTxt; prevTxt.setFont(m_font); prevTxt.setCharacterSize(14);
        prevTxt.setStyle(sf::Text::Bold); prevTxt.setFillColor(sf::Color(255, 255, 255));
        prevTxt.setString("< Prev");
        sf::FloatRect pb = prevTxt.getLocalBounds();
        prevTxt.setPosition(panX + 16.f + (100.f - pb.width) / 2.f, panY + panH - 47.f);
        m_window.draw(prevTxt);
    }
    if (page < PAGE_COUNT - 1) {
        sf::RectangleShape nextBtn(sf::Vector2f(100.f, 34.f));
        nextBtn.setPosition(panX + panW - 116.f, panY + panH - 54.f);
        nextBtn.setFillColor(sf::Color(25, 155, 80, 220));
        nextBtn.setOutlineColor(sf::Color(140, 255, 180));
        nextBtn.setOutlineThickness(2.f);
        m_window.draw(nextBtn);
        sf::Text nextTxt; nextTxt.setFont(m_font); nextTxt.setCharacterSize(14);
        nextTxt.setStyle(sf::Text::Bold); nextTxt.setFillColor(sf::Color(255, 255, 255));
        nextTxt.setString("Next >");
        sf::FloatRect nb = nextTxt.getLocalBounds();
        nextTxt.setPosition(panX + panW - 116.f + (100.f - nb.width) / 2.f, panY + panH - 47.f);
        m_window.draw(nextTxt);
    }

    sf::RectangleShape closeBtn(sf::Vector2f(38.f, 38.f));
    closeBtn.setPosition(panX + panW - 48.f, panY + 7.f);
    closeBtn.setFillColor(sf::Color(220, 30, 100, 240));
    closeBtn.setOutlineColor(sf::Color(255, 180, 210));
    closeBtn.setOutlineThickness(2.f);
    m_window.draw(closeBtn);
    sf::Text closeTxt; closeTxt.setFont(m_font); closeTxt.setCharacterSize(20);
    closeTxt.setStyle(sf::Text::Bold); closeTxt.setFillColor(sf::Color(255, 255, 255));
    closeTxt.setString("X");
    sf::FloatRect cb = closeTxt.getLocalBounds();
    closeTxt.setPosition(panX + panW - 48.f + (38.f - cb.width) / 2.f,
        panY + 7.f + (38.f - cb.height) / 2.f - 4.f);
    m_window.draw(closeTxt);

    sf::Text hint; hint.setFont(m_font); hint.setCharacterSize(11);
    hint.setFillColor(sf::Color(200, 140, 180, 200));
    hint.setString("Left / Right Arrow to flip pages   |   ESC to close");
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setPosition(panX + (panW - hb.width) / 2.f, panY + panH - 16.f);
    m_window.draw(hint);
}


static std::string todayDateString()
{
    std::time_t t = std::time(nullptr);
    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d", std::localtime(&t));
    return buf;
}


void Game::loadLeaderboard()
{
    m_leaderboardCount = 0;

    std::ifstream fin("leaderboard.txt");
    if (!fin.is_open()) return;

    LeaderboardEntry all[500];
    int allCount = 0;

    std::string line;
    while (std::getline(fin, line) && allCount < 500)
    {
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string name, scoreStr, gemsStr, levelStr, dateStr;
        if (std::getline(ss, name, '|') &&
            std::getline(ss, scoreStr, '|') &&
            std::getline(ss, gemsStr, '|') &&
            std::getline(ss, levelStr, '|') &&
            std::getline(ss, dateStr))
        {
            LeaderboardEntry& e = all[allCount++];
            strncpy(e.name, name.c_str(), 31); e.name[31] = '\0';
            strncpy(e.date, dateStr.c_str(), 31); e.date[31] = '\0';
            e.score = std::stoi(scoreStr);
            e.gems = std::stoi(gemsStr);
            e.level = std::stoi(levelStr);
        }
    }
    fin.close();

    // Sort descending by score (bubble sort — fine for ≤500 entries)
    for (int a = 0; a < allCount - 1; a++)
        for (int b = a + 1; b < allCount; b++)
            if (all[b].score > all[a].score)
            {
                LeaderboardEntry tmp = all[a]; all[a] = all[b]; all[b] = tmp;
            }

    // Copy top MAX_LEADERBOARD_ENTRIES
    m_leaderboardCount = 0;
    for (int i = 0; i < allCount && m_leaderboardCount < MAX_LEADERBOARD_ENTRIES; i++)
        m_leaderboard[m_leaderboardCount++] = all[i];
}

void Game::saveLeaderboard() {}


void Game::submitScore(const char* name, int score, int gems, int level)
{
    if (!name || name[0] == '\0') return;
    if (score <= 0) return;

    std::string dateStr = todayDateString();

    // ── Step 1: Read all existing entries ────────────────────────────
    struct RawEntry { char name[32]; int score; int gems; int level; char date[32]; };
    RawEntry all[500];
    int allCount = 0;

    {
        std::ifstream fin("leaderboard.txt");
        if (fin.is_open())
        {
            std::string line;
            while (std::getline(fin, line) && allCount < 500)
            {
                if (line.empty()) continue;
                std::istringstream ss(line);
                std::string n, hs, gm, lv, dt;
                if (std::getline(ss, n, '|') && std::getline(ss, hs, '|') &&
                    std::getline(ss, gm, '|') && std::getline(ss, lv, '|') &&
                    std::getline(ss, dt))
                {
                    RawEntry& e = all[allCount++];
                    strncpy(e.name, n.c_str(), 31); e.name[31] = '\0';
                    strncpy(e.date, dt.c_str(), 31); e.date[31] = '\0';
                    e.score = std::stoi(hs);
                    e.gems = std::stoi(gm);
                    e.level = std::stoi(lv);
                }
            }
        }
    }

    //Upsert this player's best entry ───────────────────────
    bool found = false;
    for (int i = 0; i < allCount; i++)
    {
        if (strcmp(all[i].name, name) == 0)
        {
            if (score > all[i].score) all[i].score = score;
            if (gems > all[i].gems)  all[i].gems = gems;
            if (level > all[i].level) all[i].level = level;
            strncpy(all[i].date, dateStr.c_str(), 31); all[i].date[31] = '\0';
            found = true;
            break;
        }
    }
    if (!found && allCount < 500)
    {
        RawEntry& e = all[allCount++];
        strncpy(e.name, name, 31); e.name[31] = '\0';
        strncpy(e.date, dateStr.c_str(), 31); e.date[31] = '\0';
        e.score = score; e.gems = gems; e.level = level;
    }

    //write leaderboard.txt (one row per player) ───────
    {
        std::ofstream fout("leaderboard.txt");   // truncate — NOT append
        if (fout.is_open())
            for (int i = 0; i < allCount; i++)
                fout << all[i].name << "|" << all[i].score << "|"
                << all[i].gems << "|" << all[i].level << "|"
                << all[i].date << "\n";
    }

    // Update player_progress.txt ───────────────────────────
    struct ProgressRow { char name[32]; int high_score; int gems; int level; char date[32]; };
    ProgressRow rows[200]; int rowCount = 0;

    {
        std::ifstream fin2("player_progress.txt");
        if (fin2.is_open())
        {
            std::string line;
            while (std::getline(fin2, line) && rowCount < 200)
            {
                if (line.empty()) continue;
                std::istringstream ss(line);
                std::string n, hs, gm, lv, dt;
                if (std::getline(ss, n, '|') && std::getline(ss, hs, '|') &&
                    std::getline(ss, gm, '|') && std::getline(ss, lv, '|') &&
                    std::getline(ss, dt))
                {
                    ProgressRow& r = rows[rowCount++];
                    strncpy(r.name, n.c_str(), 31); r.name[31] = '\0';
                    strncpy(r.date, dt.c_str(), 31); r.date[31] = '\0';
                    r.high_score = std::stoi(hs);
                    r.gems = std::stoi(gm);
                    r.level = std::stoi(lv);
                }
            }
        }
    }

    bool found2 = false;
    for (int i = 0; i < rowCount; i++)
    {
        if (strcmp(rows[i].name, name) == 0)
        {
            if (score > rows[i].high_score) rows[i].high_score = score;
            if (gems > rows[i].gems)       rows[i].gems = gems;
            if (level > rows[i].level)      rows[i].level = level;
            strncpy(rows[i].date, dateStr.c_str(), 31); rows[i].date[31] = '\0';
            found2 = true; break;
        }
    }
    if (!found2 && rowCount < 200)
    {
        ProgressRow& r = rows[rowCount++];
        strncpy(r.name, name, 31); r.name[31] = '\0';
        strncpy(r.date, dateStr.c_str(), 31); r.date[31] = '\0';
        r.high_score = score; r.gems = gems; r.level = level;
    }

    {
        std::ofstream fout2("player_progress.txt");
        if (fout2.is_open())
            for (int i = 0; i < rowCount; i++)
                fout2 << rows[i].name << "|" << rows[i].high_score << "|"
                << rows[i].gems << "|" << rows[i].level << "|"
                << rows[i].date << "\n";
    }

    // Refresh in-memory leaderboard immediately
    loadLeaderboard();
}

// saveGameToFile / loadGameFromFile
void Game::saveGameToFile()
{
    if (!m_saveSlot.valid) return;
    std::ofstream fout("savegame.dat", std::ios::binary);
    if (!fout.is_open()) return;
    fout.write(reinterpret_cast<const char*>(&m_saveSlot.level), sizeof(int));
    fout.write(reinterpret_cast<const char*>(&m_saveSlot.playerCount), sizeof(int));
    for (int i = 0; i < m_saveSlot.playerCount; i++)
    {
        fout.write(reinterpret_cast<const char*>(&m_saveSlot.charIndex[i]), sizeof(int));
        fout.write(reinterpret_cast<const char*>(&m_saveSlot.playerData[i]), sizeof(Player::SaveData));
    }
    fout.close();
}

bool Game::loadGameFromFile()
{
    std::ifstream fin("savegame.dat", std::ios::binary);
    if (!fin.is_open()) return false;
    GameSaveSlot tmp;
    fin.read(reinterpret_cast<char*>(&tmp.level), sizeof(int));
    fin.read(reinterpret_cast<char*>(&tmp.playerCount), sizeof(int));
    if (tmp.playerCount < 1 || tmp.playerCount > MAX_PLAYERS) { fin.close(); return false; }
    for (int i = 0; i < tmp.playerCount; i++)
    {
        fin.read(reinterpret_cast<char*>(&tmp.charIndex[i]), sizeof(int));
        fin.read(reinterpret_cast<char*>(&tmp.playerData[i]), sizeof(Player::SaveData));
    }
    fin.close();
    if (!fin) return false;
    tmp.valid = true;
    m_saveSlot = tmp;
    return true;
}