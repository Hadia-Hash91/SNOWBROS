

#include "KnifeProjectile.h"
#include "Constants.h"
#include <cmath>

sf::Texture        KnifeProjectile::s_texture;
bool               KnifeProjectile::s_loaded = false;

const sf::IntRect  KnifeProjectile::FRAMES[4] =
{
    sf::IntRect(285, 892, 59, 23),   // white
    sf::IntRect(356, 892, 59, 23),   // yellow
    sf::IntRect(427, 892, 60, 23),   // blue
    sf::IntRect(499, 892, 59, 24),   // red
};


void KnifeProjectile::ensureLoaded()
{
    if (s_loaded) return;
    if (!s_texture.loadFromFile("assets/images/Tornado_Blue.png"))
        s_texture.loadFromFile("Tornado_Blue.png");
    s_texture.setSmooth(false);
    s_loaded = true;
}


KnifeProjectile::KnifeProjectile(float x, float y, sf::Vector2f direction)
    : m_position(x, y)
    , m_direction(direction)
    , m_active(true)
    , m_frame(0)
    , m_animTimer(0.f)
{
    ensureLoaded();
    m_sprite.setTexture(s_texture);
    applyFrame();

    float angle = std::atan2(direction.y, direction.x) * 180.f / 3.14159265f;
    m_sprite.setRotation(angle);
}

void KnifeProjectile::applyFrame()
{
    const sf::IntRect& r = FRAMES[m_frame];
    m_sprite.setTextureRect(r);

    m_sprite.setOrigin(r.width / 2.f, r.height / 2.f);
}

// ---------------------------------------------------------
void KnifeProjectile::update(float dt)
{
    if (!m_active) return;

    m_position += m_direction * m_speed * dt;

    m_animTimer += dt;
    if (m_animTimer >= 0.08f)
    {
        m_animTimer = 0.f;
        m_frame = (m_frame + 1) % 4;
        applyFrame();
        float angle = std::atan2(m_direction.y, m_direction.x) * 180.f / 3.14159265f;
        m_sprite.setRotation(angle);
    }

    if (m_position.x < -40.f || m_position.x > WINDOW_WIDTH + 40.f ||
        m_position.y < -40.f || m_position.y > WINDOW_HEIGHT + 40.f)
        m_active = false;
}

// ---------------------------------------------------------
void KnifeProjectile::draw(sf::RenderWindow& window)
{
    if (!m_active) return;

    if (s_loaded)
    {
        m_sprite.setPosition(m_position);
        window.draw(m_sprite);
    }
    else
    {
        sf::RectangleShape fb(sf::Vector2f(18.f, 6.f));
        fb.setOrigin(9.f, 3.f);
        fb.setPosition(m_position);
        float angle = std::atan2(m_direction.y, m_direction.x) * 180.f / 3.14159265f;
        fb.setRotation(angle);
        fb.setFillColor(sf::Color(255, 100, 30));
        window.draw(fb);
    }
}

// ---------------------------------------------------------
sf::FloatRect KnifeProjectile::getRect() const
{
    // 16x16 collision box centred on the knife — tight enough for fair gameplay
    return sf::FloatRect(m_position.x - 8.f, m_position.y - 8.f, 16.f, 16.f);
}