#pragma once
#include <SFML/Graphics.hpp>
#include "Player.h"

class Level {
private:
    sf::Texture bgTexture;
    sf::Sprite background;
    sf::Texture platformTex;
    sf::RectangleShape* platforms;
    int platformCount;

    Player player;

public:
    Level();
    ~Level();

    void load();
    void update();
    void draw(sf::RenderWindow& window);
};