#include <SFML/Graphics.hpp>
#include "Game.h"


int main()
{
    Game game;
    game.run();
    sf::Image img;
    img.loadFromFile("mogera.png");
    printf("Size: %u x %u\n", img.getSize().x, img.getSize().y);
    return 0;
}