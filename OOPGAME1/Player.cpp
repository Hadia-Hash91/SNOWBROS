#include "Player.h"
#include<iostream>
using namespace std;
Player::Player() {

    if (!player1.loadFromFile("player.png")) {
        std::cout << "Error loading player image\n";
    }
    pl1.setTexture(player1);
    pl1.setPosition(100, 500);

    // resize if too big
    pl1.setScale(1.f, 1.f);

    velocityY = 0;
    onGround = false;

    velocityY = 0;
    onGround = false;
}

void Player::handleInput() {

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))

    {
        pl1.move(-0.2f, 0);
        pl1.setScale(1.f, 1.f); // Flip horizontally when moving left
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        pl1.move(0.2f, 0);
        pl1.setScale(-1.f, 1.f); // face right
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && onGround) {
        velocityY = -0.5f;
        onGround = false;
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