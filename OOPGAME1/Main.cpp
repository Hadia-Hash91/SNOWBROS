#include <SFML/Graphics.hpp>
#include "Game.h"


int main()
{
    Game game;
    game.run();
    sf::Image img;
    img.loadFromFile("mogera.png");
    return 0;
}