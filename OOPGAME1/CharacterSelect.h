#pragma once


#include <SFML/Graphics.hpp>
#include "CharacterData.h"

class CharacterSelect
{
public:
    CharacterSelect(sf::Font& font);

  
    bool update(sf::Event& event, int numPlayers);

    void draw(sf::RenderWindow& window);

    int getSelectedCharacter(int playerSlot) const { return m_selected[playerSlot]; }
    const CharacterData& getCharacterData(int idx) const { return CHARACTER_ROSTER[idx]; }

    void reset(int numPlayers);

    
    void resetBrowse();

    bool isDone()       const { return m_done; }
    bool isCancelled()  const { return m_cancelled; }
    bool isBrowseOnly() const { return m_browseOnly; }

    int getDefaultCharacter() const { return m_defaultCharacter; }

private:
    sf::Font& m_font;
    int       m_numPlayers = 1;

    int  m_selected[2] = { 0, 1 };
    bool m_confirmed[2] = { false, false };
    bool m_done = false;
    bool m_cancelled = false;

    bool m_browseOnly = false;
    int  m_defaultCharacter = 0;   

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