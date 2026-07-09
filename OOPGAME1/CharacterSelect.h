#pragma once
// =========================================================
// CharacterSelect.h — Character selection screen
// No STL libraries
// =========================================================

#include <SFML/Graphics.hpp>
#include "CharacterData.h"

class CharacterSelect
{
public:
    CharacterSelect(sf::Font& font);

    // Returns true when selection is done
    // numPlayers: 1 or 2
    bool update(sf::Event& event, int numPlayers);

    void draw(sf::RenderWindow& window);

    // After isDone() == true, read these
    int getSelectedCharacter(int playerSlot) const { return m_selected[playerSlot]; }
    const CharacterData& getCharacterData(int idx) const { return CHARACTER_ROSTER[idx]; }

    // Reset for normal game-start flow (browseOnly = false)
    void reset(int numPlayers);

    // Reset for browse-only mode (from main menu "Select Character")
    // In this mode, confirming saves the default character and returns
    // to main menu instead of starting the game.
    void resetBrowse();

    bool isDone()       const { return m_done; }
    bool isCancelled()  const { return m_cancelled; }
    bool isBrowseOnly() const { return m_browseOnly; }

    // Default character remembered from browse mode (used by 1-player new game)
    int getDefaultCharacter() const { return m_defaultCharacter; }

private:
    sf::Font& m_font;
    int       m_numPlayers = 1;

    int  m_selected[2] = { 0, 1 };
    bool m_confirmed[2] = { false, false };
    bool m_done = false;
    bool m_cancelled = false;

    bool m_browseOnly = false;
    int  m_defaultCharacter = 0;   // persists across resets

    // Detailed info panel shown when a card is highlighted in browse mode
    void drawDetailPanel(sf::RenderWindow& window, int charIdx);

    void drawCharacterCard(sf::RenderWindow& window,
        int charIdx, float cx, float cy,
        bool isHighlighted, bool isConfirmed,
        int playerOwner,
        sf::Color playerColor);

    void drawStarBar(sf::RenderWindow& window,
        const char* label, int stars,
        float x, float y);

    sf::Texture m_bgTexture;
    sf::Sprite  m_bgSprite;
    bool        m_bgLoaded = false;

    // Per-character portrait textures (loaded once via loadCharacterImages)
    sf::Texture m_charTexture[CHARACTER_COUNT];
    sf::Sprite  m_charSprite[CHARACTER_COUNT];
    bool        m_charImgLoaded[CHARACTER_COUNT] = {};

public:
    void loadBackground(const char* path)
    {
        m_bgLoaded = m_bgTexture.loadFromFile(path);
        if (m_bgLoaded)
        {
            m_bgSprite.setTexture(m_bgTexture);
            m_bgSprite.setScale(
                800.f / m_bgTexture.getSize().x,
                600.f / m_bgTexture.getSize().y);
        }
    }

    // Call once after construction (or after changing asset folder)
    void loadCharacterImages()
    {
        for (int i = 0; i < CHARACTER_COUNT; i++)
        {
            m_charImgLoaded[i] = m_charTexture[i].loadFromFile(
                CHARACTER_ROSTER[i].imagePath);
            if (m_charImgLoaded[i])
                m_charSprite[i].setTexture(m_charTexture[i]);
        }
    }
};