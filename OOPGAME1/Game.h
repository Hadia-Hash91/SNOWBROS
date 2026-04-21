#pragma once
#include <SFML/Graphics.hpp>
#include "level.h"

class Game {
private:
    sf::RenderWindow window;
    Level level;

public:
    Game();
    void run();
};