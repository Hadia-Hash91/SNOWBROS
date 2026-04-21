#pragma once
#include <SFML/Graphics.hpp>

class Player {
private:
    sf::RectangleShape body;
    float velocityY;
    bool onGround;

public:
    Player();

    void handleInput();
    void applyGravity();
    void update();
    void draw(sf::RenderWindow& window);

    sf::FloatRect getBounds();
    void land(float y);
};