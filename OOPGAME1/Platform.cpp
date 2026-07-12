#include "Platform.h"

// Total visual height the platform image will be drawn at.
// Completely independent of the hitbox height.
// Increase this number if platforms still look too thin.
static const float VISUAL_DRAW_HEIGHT = 37.f;

Platform::Platform()
    : m_hitBox(0.f, 0.f)
{
    m_rect = sf::FloatRect(0.f, 0.f, 0.f, 0.f);
}

Platform::Platform(float x, float y, float width, float height)
    : m_hitBox(width, height, 0.f, 0.f)
{
    m_rect = sf::FloatRect(x, y, width, height);
    m_hitBox.update(sf::Vector2f(x, y));
}

void Platform::load(const std::string& imagePath)
{
    if (m_texture.loadFromFile(imagePath))
    {
        m_sprite.setTexture(m_texture);

        // Scale image to platform width and fixed visual height
        float scaleX = m_rect.width / static_cast<float>(m_texture.getSize().x);
        float scaleY = VISUAL_DRAW_HEIGHT / static_cast<float>(m_texture.getSize().y);

        m_sprite.setScale(scaleX, scaleY);

        // Draw centred on the hitbox â€” extends above and below equally
        float offsetY = (VISUAL_DRAW_HEIGHT - m_rect.height) / 2.f;
        m_sprite.setPosition(m_rect.left, m_rect.top - offsetY);

        m_hasImage = true;
    }
    else
    {
        printf("FAILED to load platform image: %s\n", imagePath.c_str());
    }
}

void Platform::draw(sf::RenderWindow& window)
{
    if (m_hasImage)
    {
        window.draw(m_sprite);
    }
    else
    {
        // Fallback coloured rectangle at full visual height
        sf::RectangleShape shape(sf::Vector2f(m_rect.width, VISUAL_DRAW_HEIGHT));
        float offsetY = (VISUAL_DRAW_HEIGHT - m_rect.height) / 2.f;
        shape.setPosition(m_rect.left, m_rect.top - offsetY);
        shape.setFillColor(sf::Color(80, 60, 40));
        shape.setOutlineColor(sf::Color(120, 90, 60));
        shape.setOutlineThickness(1.f);
        window.draw(shape);
    }
}

void Platform::drawDebug(sf::RenderWindow& window)
{
    // Blue outline shows the ACTUAL collision hitbox (not the visual)
    m_hitBox.drawDebug(window, sf::Color::Blue);
}