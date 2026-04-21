#include "Game.h"

Game::Game() : window(sf::VideoMode(800, 600), "Snow Bros Level 1") 
{
    level.load();
}

void Game::run() {
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        level.update();
        window.clear();
        level.draw(window);
        window.display();
    }
}