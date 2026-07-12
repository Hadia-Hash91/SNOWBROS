// =========================================================
// HUD.cpp
// =========================================================

#include "HUD.h"
#include "Constants.h"
#include <cstdio>

HUD::HUD(sf::Font& font) : m_font(font) {}

void HUD::draw(sf::RenderWindow& window,
    Player** players, int playerCount,
    int levelNumber, int totalLevels,
    int bossHP, int bossMaxHP)
{
    float yOffset = 8.f;
    for (int p = 0; p < playerCount; p++)
    {
        Player* pl = players[p];

        // Score
        char scoreBuf[64];
        snprintf(scoreBuf, sizeof(scoreBuf), "%s  Score: %d",
            pl->getName(), pl->getScore());
        sf::Text scoreTxt;
        scoreTxt.setFont(m_font);
        scoreTxt.setCharacterSize(18);
        scoreTxt.setFillColor(sf::Color::White);
        scoreTxt.setString(scoreBuf);
        scoreTxt.setPosition(8.f, yOffset);
        window.draw(scoreTxt);

        drawLives(window, pl->getLives(), 8.f, yOffset + 22.f);

        // Gems
        char gemBuf[32];
        snprintf(gemBuf, sizeof(gemBuf), "Gems: %d", pl->getGems());
        sf::Text gemTxt;
        gemTxt.setFont(m_font);
        gemTxt.setCharacterSize(18);
        gemTxt.setFillColor(sf::Color(255, 215, 0));
        gemTxt.setString(gemBuf);
        gemTxt.setPosition(WINDOW_WIDTH - 150.f, yOffset);
        window.draw(gemTxt);

        yOffset += 50.f;
    }

    // Level indicator
    char lvlBuf[32];
    snprintf(lvlBuf, sizeof(lvlBuf), "Level %d / %d", levelNumber, totalLevels);
    sf::Text lvlTxt;
    lvlTxt.setFont(m_font);
    lvlTxt.setCharacterSize(20);
    lvlTxt.setFillColor(sf::Color(255, 200, 100));
    lvlTxt.setString(lvlBuf);
    sf::FloatRect lb = lvlTxt.getLocalBounds();
    lvlTxt.setPosition((WINDOW_WIDTH - lb.width) / 2.f, 8.f);
    window.draw(lvlTxt);

    if (bossHP >= 0)
        drawBossHealthBar(window, bossHP, bossMaxHP);
}

void HUD::drawLives(sf::RenderWindow& window, int lives, float x, float y)
{
    for (int i = 0; i < lives; i++)
    {
        sf::CircleShape heart(7.f);
        heart.setFillColor(sf::Color(220, 40, 40));
        heart.setPosition(x + i * 18.f, y);
        window.draw(heart);
    }
}

void HUD::drawBossHealthBar(sf::RenderWindow& window, int hp, int maxHp)
{
    float barW = 400.f, barH = 18.f;
    float barX = (WINDOW_WIDTH - barW) / 2.f;
    float barY = 36.f;

    sf::RectangleShape bg(sf::Vector2f(barW, barH));
    bg.setPosition(barX, barY);
    bg.setFillColor(sf::Color(60, 0, 0));
    window.draw(bg);

    float fill = maxHp > 0 ? barW * hp / static_cast<float>(maxHp) : 0.f;
    sf::RectangleShape bar(sf::Vector2f(fill, barH));
    bar.setPosition(barX, barY);
    bar.setFillColor(sf::Color(220, 30, 30));
    window.draw(bar);

    sf::RectangleShape border(sf::Vector2f(barW, barH));
    border.setPosition(barX, barY);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::White);
    border.setOutlineThickness(2.f);
    window.draw(border);

    sf::Text lbl;
    lbl.setFont(m_font);
    lbl.setCharacterSize(14);
    lbl.setFillColor(sf::Color::White);
    lbl.setString("BOSS");
    lbl.setPosition(barX - 44.f, barY + 1.f);
    window.draw(lbl);
}