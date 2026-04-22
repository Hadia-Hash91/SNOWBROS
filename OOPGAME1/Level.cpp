#include "Level.h"
#include <iostream>
using namespace std;
// Constructor
Level::Level() 
{
    platforms = nullptr;
    platformCount = 0;
}

// Destructor (VERY IMPORTANT to avoid memory leak)
Level::~Level() {
    delete[] platforms;
}

// Load Level 1
void Level::load() {
    // Load background
    if (!bgTexture.loadFromFile("bg1.png")) {
        cout << "Error loading background!\n";
    }
    background.setTexture(bgTexture);
    // Level.cpp - load()
    float scaleX = static_cast<float>(bgTexture.getSize().x) / 800.0f;
    float scaleY = static_cast<float>(bgTexture.getSize().y) / 600.0f;
    background.setScale(800.0f / bgTexture.getSize().x, 600.0f / bgTexture.getSize().y);
    
    if (!platformTex.loadFromFile("platformfinal1.png")) {

        cout << "Platform texture failed to load!" << endl;
    }
    // Set the scale back to your original simple formula
    // Define number of platforms (can be changed anytime)
    platformCount = 7;
    bgTexture.setRepeated(true);
    platformTex.setRepeated(true);
    // Allocate memory dynamically
    platforms = new sf::RectangleShape[platformCount];

   
    // Platform 0 (Ground)
    platforms[0].setSize(sf::Vector2f(800.f, 50.f));
    platforms[0].setPosition(0.f, 550.f);
    platforms[0].setTexture(&platformTex);       
    platforms[1].setFillColor(sf::Color::White);
    // 1. Assign Texture
    platforms[0].setFillColor(sf::Color(180,160,255));      // 2. Set to White to see the image
    platforms[0].setTextureRect(sf::IntRect(0, 0, 800, 300));

    // Platform 1
    platforms[1].setSize(sf::Vector2f(200.f, 50.f));
    platforms[1].setPosition(0.f, 400.f);
    platforms[1].setTexture(&platformTex);            // 1. Assign Texture
    // 2. Set to White to see the image
    platforms[1].setTextureRect(sf::IntRect(0, 0, 800, 300));

    // Platform 2
    platforms[3].setSize(sf::Vector2f(200.f, 50.f));
    platforms[3].setPosition(300.f, 400.f);
    platforms[3].setTexture(&platformTex);            // 1. Assign Texture
    platforms[3].setFillColor(sf::Color::White);      // 2. Set to White to see the image
    platforms[3].setTextureRect(sf::IntRect(0, 0, 800, 300));    //platform3
    
    platforms[2].setSize(sf::Vector2f(200.f, 50.f));
    platforms[2].setPosition(600.f, 400.f);
    platforms[2].setTexture(&platformTex);            // 1. Assign Texture
    platforms[2].setFillColor(sf::Color::White);      // 2. Set to White to see the image
    platforms[2].setTextureRect(sf::IntRect(0, 0,800, 300));

    platforms[4].setSize(sf::Vector2f(200.f, 50.f));
    platforms[4].setPosition(600.f, 100.f);
    platforms[4].setTexture(&platformTex);            // 1. Assign Texture
    platforms[4].setFillColor(sf::Color::White);      // 2. Set to White to see the image
    platforms[4].setTextureRect(sf::IntRect(0, 0, 800, 300));

    platforms[5].setSize(sf::Vector2f(600.f, 50.f));
    platforms[5].setPosition(100.f, 250.f);
    platforms[5].setTexture(&platformTex);            // 1. Assign Texture
    platforms[5].setFillColor(sf::Color::White);      // 2. Set to White to see the image
    platforms[5].setTextureRect(sf::IntRect(0, 0, 800, 300));

    platforms[6].setSize(sf::Vector2f(200.f, 50.f));
    platforms[6].setPosition(0.f, 100.f);
    platforms[6].setTexture(&platformTex);            // 1. Assign Texture
    platforms[6].setFillColor(sf::Color::White);      // 2. Set to White to see the image
    platforms[6].setTextureRect(sf::IntRect(0, 0, 800, 300));

}
float offset = 0.0f;
// Update game logic
void Level::update() {

    // Update player (movement + gravity)
    player.update();
    float scrollSpeed = 0.1f; // Adjust this for speed
    offset += scrollSpeed;
    int texW = bgTexture.getSize().x;
    int texH = bgTexture.getSize().y;

    background.setTextureRect(sf::IntRect(static_cast<int>(offset), 0, texW, texH));
    // We tell the sprite: "Start looking at the texture from 'offset' 
    // and show a box that is 800x600 pixels wide."
    // Collision with platforms

    for (int i = 0; i < platformCount; i++) {
        if (player.getBounds().intersects(platforms[i].getGlobalBounds())) {
            float playerHeight = player.getBounds().height;
            float platformTop = platforms[i].getPosition().y;

            // Place player on top of platform
            player.land(platformTop - playerHeight);
        }
    }
}

// Draw everything
void Level::draw(sf::RenderWindow& window) {
    // Draw background
    window.draw(background);

    // Draw platforms
    for (int i = 0; i < platformCount; i++) {
        window.draw(platforms[i]);
    }

    // Draw player
    player.draw(window);
}