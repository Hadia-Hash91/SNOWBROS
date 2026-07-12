#pragma once
// =========================================================
// Shop.h — In-game gem shop
// Accessible from pause menu, between levels, and during play (S key)
// =========================================================

#include <SFML/Graphics.hpp>
#include "Player.h"

static const int SHOP_ITEM_COUNT = 5;

struct ShopItem
{
    const char* name;
    const char* description;
    int         cost;
    sf::Color   color;
};

static const ShopItem SHOP_ITEMS[SHOP_ITEM_COUNT] =
{
    { "Extra Life",           "Adds 1 life to your count.",           50, sf::Color(220, 40,  40)  },
    { "Speed Boost (30s)",    "Extended speed power-up for 30s.",     20, sf::Color(40,  180, 255) },
    { "Snowball Power",       "One-hit encasing for the level.",      30, sf::Color(180, 220, 255) },
    { "Distance Increase",    "Max-range snowball for the level.",    25, sf::Color(80,  220, 80)  },
    { "Balloon Mode (30s)",   "Extended balloon power-up for 30s.",   35, sf::Color(255, 180, 80)  },
};

class Shop
{
public:
    Shop(sf::Font& font);

    bool handleEvent(sf::Event& event, Player** players, int playerCount);

    // Call every frame with current mouse position so hover updates smoothly
    void handleMouseMove(float mouseX, float mouseY);

    void draw(sf::RenderWindow& window, Player** players, int playerCount);

    void open()
    {
        m_open = true;
        m_selection = 0;
        m_hoveredItem = -1;
        m_message[0] = '\0';
        m_msgTimer = 0.f;
    }
    void close() { m_open = false; }
    bool isOpen() const { return m_open; }

    void update(float dt);

private:
    sf::Font& m_font;
    bool      m_open = false;
    int       m_selection = 0;    // keyboard-driven selection
    int       m_hoveredItem = -1;   // mouse-driven hover (-1 = none)
    int       m_activePlayer = 0;

    // Cached row bounds so hover & click share the same geometry
    sf::FloatRect m_rowBounds[SHOP_ITEM_COUNT];

    char  m_message[128] = "";
    float m_msgTimer = 0.f;

    void applyPurchase(int itemIdx, Player& player);
    void setMessage(const char* msg)
    {
        int i = 0;
        while (msg[i] && i < 126) { m_message[i] = msg[i]; i++; }
        m_message[i] = '\0';
        m_msgTimer = 2.5f;
    }

    sf::Texture m_bgTexture;
    sf::Sprite  m_bgSprite;
    bool        m_bgLoaded = false;

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
};