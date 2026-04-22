#include "Player.h"
#include<iostream>
using namespace std;
Player::Player() {

    if (!player1.loadFromFile("player.png")) {
        std::cout << "Error loading player image\n";
    }
    pl1.setTexture(player1);
    pl1.setPosition(400, 500);

    // resize if too big
    pl1.setScale(1.5f, 1.5f);

    velocityY = 0;
    onGround = false;

   
}

void Player::handleInput() {
    float s = 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {   
        pl1.setScale(s, s);
        pl1.move(-0.2f, 0);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        pl1.setScale(-s, s);
        pl1.move(0.2f, 0);

    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        pl1.setScale(s, s);
        pl1.move(0, 0.2f);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ) {
        pl1.setScale(s, s);
        pl1.move(0, -0.2f);
    }
}

void Player::applyGravity() {
    pl1.move(0, velocityY);
}

void Player::update() {
    handleInput();
    applyGravity();
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(pl1);
}

sf::FloatRect Player::getBounds() {
    return pl1.getGlobalBounds();
}
void Player::land(float y) {
    pl1.setPosition(pl1.getPosition().x, y);
    velocityY = 0;
    onGround = true;
}