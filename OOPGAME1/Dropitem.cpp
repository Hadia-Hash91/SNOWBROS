// =========================================================
// DropItem.cpp  — Sprite-sheet rendering via Items.png
// =========================================================
//
// Items.png layout (698 x 1596, black background):
//
//  Row 0  y=30-163   : SushiLarge | SushiMedium  | BottleRed(1) | BottleRed(2)
//  Row 1  y=188-320  : SushiSmall | SushiMedium2 | BottleBlue(1)| BottleBlue(2)
//  Row 2  y=345-477  : SushiSml2  | SushiWhite   | BottleYellow | BottleYellow2
//  Row 3  y=502-635  : SushiRed   | SushiGold    | BottleGreen  | BottleGreen2
//  Row 4  y=653-796  : Kunai      | Bomb         | FaceRed | FaceBlue (pair)
//  Row 5  y=841-940  : Scroll     | FriedBall    | Lightning(B) | Lightning(R)
//  Row 6  y=982-1100 : (map/scroll large)         | FaceSmall(1) | FaceSmall(2)
//  Row 7  y=1110-1230:                             | Snowflake(B) | Snowflake(R)
//  Row 8  y=1258-1385: (empty left)               | BottleRed+frame|BottleBlue+frame
//  Row 9  y=1488-1581: (empty left)               | BottleYellow+frame
//
// Mapping to DropType:
//   SushiLarge   → Row0 col0  (x=27-137,  y=30-163)
//   SushiMedium  → Row0 col1  (x=174-303, y=30-163)
//   SushiSmall   → Row1 col0  (x=26-137,  y=188-320)
//   BottleRed    → Row0 col2  (x=348-445, y=30-163)
//   BottleBlue   → Row1 col2  (x=348-445, y=188-320)
//   BottleYellow → Row2 col2  (x=348-445, y=345-477)
//   BottleGreen  → Row3 col2  (x=348-445, y=502-635)
//   FaceRed      → Row4 col2  left face  (x=321-473, y=653-796)
//   FaceBlue     → Row4 col2  right face (x=477-629, y=653-796)
//   Gem          → Row5 col1  fried/gem  (x=174-304, y=841-940)
// =========================================================

#include "DropItem.h"
#include "Constants.h"
#include <cmath>
#include <iostream>

// Static members
sf::Texture DropItem::s_sheet;
bool        DropItem::s_sheetLoaded = false;

// ── initSheet: load Items.png once ───────────────────────
void DropItem::initSheet()
{
    if (s_sheetLoaded) return;

    bool ok = s_sheet.loadFromFile("Items.png");
    if (!ok) ok = s_sheet.loadFromFile("assets/images/Items.png");
    if (!ok) ok = s_sheet.loadFromFile("assets/images/Items.png");

    s_sheetLoaded = true; // even on failure — avoid retry spam
    if (ok)
        s_sheet.setSmooth(false);
    else
        std::cout << "[DropItem] Items.png not found — using fallback shapes\n";
}

// ── Constructors ─────────────────────────────────────────
DropItem::DropItem()
    : m_active(false), m_type(DropType::Gem)
{
    m_rect = sf::FloatRect(0.f, 0.f, 0.f, 0.f);
}

DropItem::DropItem(float x, float y, DropType type)
    : m_type(type), m_active(true)
{
    initSheet();
    assignVisual();
    m_rect = sf::FloatRect(x, y - 10.f, m_size, m_size);
}

// ── assignVisual: pick source rect on sheet ──────────────
void DropItem::assignVisual()
{
    // Default fallback colours (used when sheet is missing)
    switch (m_type)
    {
    case DropType::SushiSmall:   m_color = sf::Color(210, 100, 60); m_size = 28.f; break;
    case DropType::SushiMedium:  m_color = sf::Color(210, 60, 60); m_size = 32.f; break;
    case DropType::SushiLarge:   m_color = sf::Color(180, 30, 30); m_size = 36.f; break;
    case DropType::BottleRed:    m_color = sf::Color(200, 30, 30); m_size = 28.f; break;
    case DropType::BottleBlue:   m_color = sf::Color(30, 80, 200); m_size = 28.f; break;
    case DropType::BottleYellow: m_color = sf::Color(200, 180, 20); m_size = 28.f; break;
    case DropType::BottleGreen:  m_color = sf::Color(20, 160, 60); m_size = 28.f; break;
    case DropType::FaceBlue:     m_color = sf::Color(100, 180, 255); m_size = 32.f; break;
    case DropType::FaceRed:      m_color = sf::Color(255, 100, 100); m_size = 32.f; break;
    case DropType::Gem:          m_color = sf::Color(100, 220, 255); m_size = 24.f; break;
    }

    // Map each type to its exact pixel rectangle on Items.png
    // All coordinates verified from the 698x1596 sprite sheet.
    bool mapped = true;
    switch (m_type)
    {
    case DropType::SushiLarge:
        // Row 0, col 0 — large sushi bowl
        m_srcRect = sf::IntRect(27, 30, 111, 134);
        m_size = 36.f;
        break;

    case DropType::SushiMedium:
        // Row 0, col 1 — medium sushi plate
        m_srcRect = sf::IntRect(174, 30, 130, 134);
        m_size = 32.f;
        break;

    case DropType::SushiSmall:
        // Row 1, col 0 — small sushi piece
        m_srcRect = sf::IntRect(26, 188, 112, 133);
        m_size = 28.f;
        break;

    case DropType::BottleRed:
        // Row 0, col 2 — red bottle
        m_srcRect = sf::IntRect(348, 30, 98, 134);
        m_size = 28.f;
        break;

    case DropType::BottleBlue:
        // Row 1, col 2 — blue bottle
        m_srcRect = sf::IntRect(348, 188, 98, 133);
        m_size = 28.f;
        break;

    case DropType::BottleYellow:
        // Row 2, col 2 — yellow bottle
        m_srcRect = sf::IntRect(348, 345, 98, 133);
        m_size = 28.f;
        break;

    case DropType::BottleGreen:
        // Row 3, col 2 — green bottle
        m_srcRect = sf::IntRect(348, 502, 98, 134);
        m_size = 28.f;
        break;

    case DropType::FaceRed:
        // Row 4, left face (pink/red border face)
        m_srcRect = sf::IntRect(321, 653, 153, 144);
        m_size = 32.f;
        break;

    case DropType::FaceBlue:
        // Row 4, right face (green/blue border face)
        m_srcRect = sf::IntRect(477, 653, 153, 144);
        m_size = 32.f;
        break;

    case DropType::Gem:
        // Row 5, col 1 — fried ball / gem item
        m_srcRect = sf::IntRect(174, 841, 131, 100);
        m_size = 24.f;
        break;

    default:
        mapped = false;
        break;
    }

    m_rect.width = m_size;
    m_rect.height = m_size;

    if (mapped && s_sheetLoaded && s_sheet.getSize().x > 0)
    {
        m_sprite.setTexture(s_sheet);
        m_sprite.setTextureRect(m_srcRect);
        m_useSprite = true;
    }
}

// ── update ───────────────────────────────────────────────
void DropItem::update(float dt)
{
    if (!m_active) return;

    m_lifetime -= dt;
    if (m_lifetime <= 0.f) { m_active = false; return; }

    // Gravity
    m_vy += GRAVITY * dt * 0.5f;
    m_rect.top += m_vy * dt;

    // Bounce off floor
    float floorY = WINDOW_HEIGHT - 48.f - m_size;
    if (m_rect.top >= floorY)
    {
        m_rect.top = floorY;
        m_vy = -(float)fabs(m_vy) * 0.45f;
        if ((float)fabs(m_vy) < 20.f) m_vy = 0.f;
    }

    // Keep on screen horizontally
    if (m_rect.left < 0.f)
        m_rect.left = 0.f;
    if (m_rect.left + m_size > WINDOW_WIDTH)
        m_rect.left = WINDOW_WIDTH - m_size;

    m_bobTimer += dt;
}

// ── draw ─────────────────────────────────────────────────
void DropItem::draw(sf::RenderWindow& window)
{
    if (!m_active) return;

    // Flicker in last 2 seconds
    if (m_lifetime < 2.f && static_cast<int>(m_lifetime * 8) % 2 == 0) return;

    if (m_useSprite)
    {
        // Scale the source sprite to our display size
        float scaleX = m_size / static_cast<float>(m_srcRect.width);
        float scaleY = m_size / static_cast<float>(m_srcRect.height);
        // Gentle bob
        float bob = std::sin(m_bobTimer * 3.f) * 2.f;
        m_sprite.setScale(scaleX, scaleY);
        m_sprite.setPosition(m_rect.left, m_rect.top + bob);
        window.draw(m_sprite);
        return;
    }

    // ── Fallback shape rendering ──────────────────────────
    bool isBottle = (m_type == DropType::BottleRed ||
        m_type == DropType::BottleBlue ||
        m_type == DropType::BottleYellow ||
        m_type == DropType::BottleGreen);
    bool isFace = (m_type == DropType::FaceBlue || m_type == DropType::FaceRed);

    if (isBottle)
    {
        sf::RectangleShape cap(sf::Vector2f(m_size * 0.5f, m_size * 0.25f));
        cap.setPosition(m_rect.left + m_size * 0.25f, m_rect.top);
        cap.setFillColor(sf::Color(180, 180, 180));
        window.draw(cap);

        sf::RectangleShape body(sf::Vector2f(m_size * 0.7f, m_size * 0.75f));
        body.setPosition(m_rect.left + m_size * 0.15f, m_rect.top + m_size * 0.25f);
        body.setFillColor(sf::Color(220, 220, 220, 180));
        body.setOutlineColor(sf::Color(160, 160, 160));
        body.setOutlineThickness(1.f);
        window.draw(body);

        sf::RectangleShape liquid(sf::Vector2f(m_size * 0.55f, m_size * 0.5f));
        liquid.setPosition(m_rect.left + m_size * 0.22f, m_rect.top + m_size * 0.42f);
        liquid.setFillColor(m_color);
        window.draw(liquid);
    }
    else if (isFace)
    {
        sf::CircleShape face(m_size * 0.5f);
        face.setPosition(m_rect.left, m_rect.top);
        face.setFillColor(sf::Color::White);
        face.setOutlineColor(m_color);
        face.setOutlineThickness(3.f);
        window.draw(face);

        sf::CircleShape eye(2.5f);
        eye.setFillColor(sf::Color::Black);
        eye.setPosition(m_rect.left + m_size * 0.28f, m_rect.top + m_size * 0.35f);
        window.draw(eye);
        eye.setPosition(m_rect.left + m_size * 0.58f, m_rect.top + m_size * 0.35f);
        window.draw(eye);

        sf::RectangleShape smile(sf::Vector2f(m_size * 0.4f, 2.f));
        smile.setFillColor(sf::Color::Black);
        smile.setPosition(m_rect.left + m_size * 0.3f, m_rect.top + m_size * 0.6f);
        window.draw(smile);
    }
    else if (m_type == DropType::Gem)
    {
        sf::CircleShape gem(m_size * 0.5f, 6);
        gem.setPosition(m_rect.left, m_rect.top);
        gem.setFillColor(m_color);
        gem.setOutlineColor(sf::Color::White);
        gem.setOutlineThickness(1.5f);
        window.draw(gem);
    }
    else
    {
        // Sushi fallback
        sf::RectangleShape rice(sf::Vector2f(m_size, m_size * 0.55f));
        rice.setPosition(m_rect.left, m_rect.top + m_size * 0.45f);
        rice.setFillColor(sf::Color(245, 235, 210));
        window.draw(rice);

        sf::RectangleShape topping(sf::Vector2f(m_size, m_size * 0.5f));
        topping.setPosition(m_rect.left, m_rect.top);
        topping.setFillColor(m_color);
        window.draw(topping);
    }
}

// ── collect / scoreValue ─────────────────────────────────
int DropItem::collect()
{
    m_active = false;
    m_collected = true;
    return scoreValue();
}

int DropItem::scoreValue() const
{
    switch (m_type)
    {
    case DropType::SushiSmall:   return 100;
    case DropType::SushiMedium:  return 300;
    case DropType::SushiLarge:   return 500;
    case DropType::FaceBlue:     return 1000;
    case DropType::FaceRed:      return 2000;
    default:                     return 0;
    }
}