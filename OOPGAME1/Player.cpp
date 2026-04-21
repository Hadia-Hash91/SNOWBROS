#include "Player.h"

Player::Player() {
    body.setSize(sf::Vector2f(40, 40));
    body.setFillColor(sf::Color::Blue);
    body.setPosition(100, 100);

    velocityY = 0;
    onGround = false;
}

void Player::handleInput() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        body.move(-0.2f, 0);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        body.move(0.2f, 0);

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) && onGround) {
        velocityY = -0.5f;
        onGround = false;
    }
}

void Player::applyGravity() {
    velocityY += 0.001f;
    body.move(0, velocityY);
}

void Player::update() {
    handleInput();
    applyGravity();
}

void Player::draw(sf::RenderWindow& window) {
    window.draw(body);
}

sf::FloatRect Player::getBounds() {
    return body.getGlobalBounds();
}

void Player::land(float y) {
    body.setPosition(body.getPosition().x, y);
    velocityY = 0;
    onGround = true;
}