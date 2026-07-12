#pragma once
// =========================================================
// DropItem.h  — Sprite-sheet rendering via Items.png
// =========================================================

#include <SFML/Graphics.hpp>

enum class DropType
{
    SushiSmall,
    SushiMedium,
    SushiLarge,
    BottleRed,
    BottleBlue,
    BottleYellow,
    BottleGreen,
    FaceBlue,
    FaceRed,
    Gem
};

class DropItem
{
public:
    DropItem();
    DropItem(float x, float y, DropType type);

    void update(float dt);
    void draw(sf::RenderWindow& window);

    bool          isActive()    const { return m_active; }
    bool          isCollected() const { return m_collected; }
    sf::FloatRect getRect()     const { return m_rect; }
    DropType      getType()     const { return m_type; }

    int  collect();
    void setLifetime(float t) { m_lifetime = t; }

private:
    // Shared sprite sheet (loaded once for all DropItems)
    static sf::Texture  s_sheet;
    static bool         s_sheetLoaded;

    sf::Sprite    m_sprite;
    sf::IntRect   m_srcRect;
    bool          m_useSprite = false;

    sf::FloatRect m_rect;
    DropType      m_type = DropType::Gem;
    bool          m_active = false;
    bool          m_collected = false;
    float         m_lifetime = 8.f;
    float         m_vy = -200.f;
    float         m_bobTimer = 0.f;

    // Fallback colour rendering if sheet not found
    sf::Color     m_color;
    float         m_size = 28.f;

    void assignVisual();
    void initSheet();
    int  scoreValue() const;
};