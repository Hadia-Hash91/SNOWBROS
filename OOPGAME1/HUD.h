#pragma once
// =========================================================
// HUD.h
// =========================================================

#include <SFML/Graphics.hpp>
#include "Player.h"

class HUD
{
public:
    HUD(sf::Font& font);

    // Takes plain array of Player pointers
    void draw(sf::RenderWindow& window,
        Player** players, int playerCount,
        int levelNumber, int totalLevels,
        int bossHP = -1, int bossMaxHP = 1);

private:
    sf::Font& m_font;
    void drawLives(sf::RenderWindow& window, int lives, float x, float y);
    void drawBossHealthBar(sf::RenderWindow& window, int hp, int maxHp);
};