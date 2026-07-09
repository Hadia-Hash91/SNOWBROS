#pragma once


#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string>
#include "Constants.h"

// Key binding slots
enum class KeySlot
{
    P1Left, P1Right, P1Jump, P1Throw,
    P2Left, P2Right, P2Jump, P2Throw,
    COUNT
};

static const int KEY_SLOT_COUNT = static_cast<int>(KeySlot::COUNT);

// Friendly labels for each slot
static const char* KEY_SLOT_LABELS[KEY_SLOT_COUNT] =
{
    "P1 Move Left",  "P1 Move Right",  "P1 Jump",  "P1 Throw",
    "P2 Move Left",  "P2 Move Right",  "P2 Jump",  "P2 Throw"
};

struct KeyBindings
{
    sf::Keyboard::Key keys[KEY_SLOT_COUNT];

    void setDefaults()
    {
        keys[static_cast<int>(KeySlot::P1Left)] = sf::Keyboard::A;
        keys[static_cast<int>(KeySlot::P1Right)] = sf::Keyboard::D;
        keys[static_cast<int>(KeySlot::P1Jump)] = sf::Keyboard::W;
        keys[static_cast<int>(KeySlot::P1Throw)] = sf::Keyboard::Space;

        keys[static_cast<int>(KeySlot::P2Left)] = sf::Keyboard::Numpad4;
        keys[static_cast<int>(KeySlot::P2Right)] = sf::Keyboard::Numpad6;
        keys[static_cast<int>(KeySlot::P2Jump)] = sf::Keyboard::Numpad8;
        keys[static_cast<int>(KeySlot::P2Throw)] = sf::Keyboard::Numpad0;
    }

    sf::Keyboard::Key get(KeySlot slot) const
    {
        return keys[static_cast<int>(slot)];
    }
    void set(KeySlot slot, sf::Keyboard::Key k)
    {
        keys[static_cast<int>(slot)] = k;
    }
};


static const char* keyName(sf::Keyboard::Key k)
{
    switch (k)
    {
    case sf::Keyboard::A:        return "A";
    case sf::Keyboard::B:        return "B";
    case sf::Keyboard::C:        return "C";
    case sf::Keyboard::D:        return "D";
    case sf::Keyboard::E:        return "E";
    case sf::Keyboard::F:        return "F";
    case sf::Keyboard::G:        return "G";
    case sf::Keyboard::H:        return "H";
    case sf::Keyboard::I:        return "I";
    case sf::Keyboard::J:        return "J";
    case sf::Keyboard::K:        return "K";
    case sf::Keyboard::L:        return "L";
    case sf::Keyboard::M:        return "M";
    case sf::Keyboard::N:        return "N";
    case sf::Keyboard::O:        return "O";
    case sf::Keyboard::P:        return "P";
    case sf::Keyboard::Q:        return "Q";
    case sf::Keyboard::R:        return "R";
    case sf::Keyboard::S:        return "S";
    case sf::Keyboard::T:        return "T";
    case sf::Keyboard::U:        return "U";
    case sf::Keyboard::V:        return "V";
    case sf::Keyboard::W:        return "W";
    case sf::Keyboard::X:        return "X";
    case sf::Keyboard::Y:        return "Y";
    case sf::Keyboard::Z:        return "Z";
    case sf::Keyboard::Space:    return "Space";
    case sf::Keyboard::Return:   return "Enter";
    case sf::Keyboard::Escape:   return "Esc";
    case sf::Keyboard::Up:       return "Up";
    case sf::Keyboard::Down:     return "Down";
    case sf::Keyboard::Left:     return "Left";
    case sf::Keyboard::Right:    return "Right";
    case sf::Keyboard::Numpad0:  return "Num0";
    case sf::Keyboard::Numpad1:  return "Num1";
    case sf::Keyboard::Numpad2:  return "Num2";
    case sf::Keyboard::Numpad3:  return "Num3";
    case sf::Keyboard::Numpad4:  return "Num4";
    case sf::Keyboard::Numpad5:  return "Num5";
    case sf::Keyboard::Numpad6:  return "Num6";
    case sf::Keyboard::Numpad7:  return "Num7";
    case sf::Keyboard::Numpad8:  return "Num8";
    case sf::Keyboard::Numpad9:  return "Num9";
    case sf::Keyboard::Tab:      return "Tab";
    case sf::Keyboard::LShift:   return "LShift";
    case sf::Keyboard::RShift:   return "RShift";
    case sf::Keyboard::LControl: return "LCtrl";
    case sf::Keyboard::RControl: return "RCtrl";
    case sf::Keyboard::LAlt:     return "LAlt";
    case sf::Keyboard::RAlt:     return "RAlt";
    default:                     return "???";
    }
}


class Settings
{
public:
    Settings(sf::Font& font);

    // Load the settings background image (call once after construction)
    void loadBackground(const std::string& path)
    {
        if (m_bgTexture.loadFromFile(path))
        {
            m_bgSprite.setTexture(m_bgTexture);
            // Scale sprite to fill the whole window
            sf::Vector2u texSize = m_bgTexture.getSize();
            m_bgSprite.setScale(
                static_cast<float>(WINDOW_WIDTH) / static_cast<float>(texSize.x),
                static_cast<float>(WINDOW_HEIGHT) / static_cast<float>(texSize.y)
            );
            m_bgLoaded = true;
        }
    }

    bool handleEvent(sf::Event& event, sf::Music& music);

    void draw(sf::RenderWindow& window);

    void open() { m_open = true;  m_section = 0; m_row = 0; m_rebinding = false; }
    void close() { m_open = false; m_rebinding = false; }
    bool isOpen() const { return m_open; }

    // Access current bindings / volume
    const KeyBindings& getBindings() const { return m_bindings; }
    float getMusicVolume()  const { return m_musicVolume; }
    float getSoundVolume()  const { return m_soundVolume; }

    void applyVolume(sf::Music& music) const
    {
        music.setVolume(m_musicVolume);
    }

private:
    sf::Font& m_font;
    sf::Texture     m_bgTexture;
    sf::Sprite      m_bgSprite;
    bool            m_bgLoaded = false;
    bool            m_open = false;

    // 0 = Volume, 1 = Key Bindings P1, 2 = Key Bindings P2
    int  m_section = 0;
    int  m_row = 0;           

    float m_musicVolume = 80.f;   // 0..100
    float m_soundVolume = 80.f;

    KeyBindings m_bindings;

    bool            m_rebinding = false;  // waiting for a key press
    int             m_rebindSlot = 0;      // which slot is being rebound

    void drawVolume(sf::RenderWindow& window,
        float contentX, float contentY,
        float contentW, float contentH);

    void drawKeyBindings(sf::RenderWindow& window,
        float contentX, float contentY,
        float contentW, float contentH,
        int playerIndex);

    void drawSectionTabs(sf::RenderWindow& window,
        float startX, float startY,
        float tabW, float tabH, float tabGap);

    int rowCount() const;
};