

#include "Settings.h"
#include "Constants.h"
#include <cstdio>
#include <cstring>

Settings::Settings(sf::Font& font) : m_font(font)
{
    m_bindings.setDefaults();
}

int Settings::rowCount() const
{
    
    if (m_section == 0) return 2;
    return 4;
}

// =========================================================
bool Settings::handleEvent(sf::Event& event, sf::Music& music)
{
    if (!m_open) return false;
    if (event.type != sf::Event::KeyPressed) return false;

    // If rebinding, capture the next key
    if (m_rebinding)
    {
        sf::Keyboard::Key pressed = event.key.code;
        if (pressed == sf::Keyboard::Escape)
        {
            m_rebinding = false;
            return false;
        }
        m_bindings.keys[m_rebindSlot] = pressed;
        m_rebinding = false;
        return false;
    }

    // ESC closes the settings
    if (event.key.code == sf::Keyboard::Escape)
    {
        close();
        return true;
    }

    // Tab / Left / Right switch section
    if (event.key.code == sf::Keyboard::Tab ||
        event.key.code == sf::Keyboard::Right)
    {
        m_section = (m_section + 1) % 3;
        m_row = 0;
        return false;
    }
    if (event.key.code == sf::Keyboard::Left)
    {
        m_section = (m_section + 2) % 3;
        m_row = 0;
        return false;
    }

    // Up / Down navigate rows
    if (event.key.code == sf::Keyboard::Up)
    {
        int rc = rowCount();
        m_row = (m_row - 1 + rc) % rc;
        return false;
    }
    if (event.key.code == sf::Keyboard::Down)
    {
        m_row = (m_row + 1) % rowCount();
        return false;
    }

    // Volume section — adjust with + / -
    if (m_section == 0)
    {
        float step = 5.f;
        bool plus = (event.key.code == sf::Keyboard::Equal ||
            event.key.code == sf::Keyboard::Add ||
            event.key.code == sf::Keyboard::Return);
        bool minus = (event.key.code == sf::Keyboard::Dash ||
            event.key.code == sf::Keyboard::Subtract);

        if (m_row == 0) // Music volume
        {
            if (plus)  m_musicVolume = (m_musicVolume + step > 100.f) ? 100.f : m_musicVolume + step;
            if (minus) m_musicVolume = (m_musicVolume - step < 0.f) ? 0.f : m_musicVolume - step;
            music.setVolume(m_musicVolume);
        }
        else // Sound volume
        {
            if (plus)  m_soundVolume = (m_soundVolume + step > 100.f) ? 100.f : m_soundVolume + step;
            if (minus) m_soundVolume = (m_soundVolume - step < 0.f) ? 0.f : m_soundVolume - step;
        }
    }
    else // Key binding section — Enter starts rebinding
    {
        if (event.key.code == sf::Keyboard::Return ||
            event.key.code == sf::Keyboard::Space)
        {
            int base = (m_section == 1) ? 0 : 4; // P1 slots 0-3, P2 slots 4-7
            m_rebindSlot = base + m_row;
            m_rebinding = true;
        }
    }

    return false;
}

// =========================================================
void Settings::draw(sf::RenderWindow& window)
{
    if (!m_open) return;

    // ── Full-screen background image ─────────────────────────
    if (m_bgLoaded)
        window.draw(m_bgSprite);

    // ── Layout constants — UI sits directly on the background ─
    const float W = static_cast<float>(WINDOW_WIDTH);
    const float H = static_cast<float>(WINDOW_HEIGHT);

    // ── TITLE at the top ─────────────────────────────────────
    sf::Text title;
    title.setFont(m_font);
    title.setCharacterSize(36);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(255, 255, 255));
    title.setOutlineColor(sf::Color(30, 100, 220));
    title.setOutlineThickness(3.f);
    title.setString("SETTINGS");
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition((W - tb.width) / 2.f, 18.f);
    window.draw(title);

    // Decorative line under title
    sf::RectangleShape titleLine(sf::Vector2f(300.f, 3.f));
    titleLine.setFillColor(sf::Color(100, 200, 255, 220));
    titleLine.setPosition((W - 300.f) / 2.f, 60.f);
    window.draw(titleLine);

    // ── SECTION TABS — centered row of 3 blue buttons ─────────
    const float tabAreaY = 72.f;
    const float tabW = 160.f, tabH = 36.f, tabGap = 10.f;
    const float tabAreaX = (W - (3.f * tabW + 2.f * tabGap)) / 2.f;
    drawSectionTabs(window, tabAreaX, tabAreaY, tabW, tabH, tabGap);

    // ── CONTENT — volume sliders or key rows ───────────────────
    const float contentY = tabAreaY + tabH + 16.f;
    const float contentW = 500.f;
    const float contentX = (W - contentW) / 2.f;

    if (m_section == 0)
        drawVolume(window, contentX, contentY, contentW, H - contentY - 40.f);
    else if (m_section == 1)
        drawKeyBindings(window, contentX, contentY, contentW, H - contentY - 40.f, 0);
    else
        drawKeyBindings(window, contentX, contentY, contentW, H - contentY - 40.f, 1);

    // ── BOTTOM HINT ───────────────────────────────────────────
    sf::RectangleShape hintBar(sf::Vector2f(W, 28.f));
    hintBar.setPosition(0.f, H - 28.f);
    hintBar.setFillColor(sf::Color(10, 30, 100, 200));
    window.draw(hintBar);

    sf::Text hint;
    hint.setFont(m_font);
    hint.setCharacterSize(12);
    hint.setFillColor(sf::Color(180, 220, 255));
    if (m_rebinding)
        hint.setString("Press any key to bind   |   ESC to cancel");
    else if (m_section == 0)
        hint.setString("UP/DOWN select   |   +/- adjust volume   |   TAB switch section   |   ESC close");
    else
        hint.setString("UP/DOWN select   |   ENTER to rebind key   |   TAB switch section   |   ESC close");
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setPosition((W - hb.width) / 2.f, H - 22.f);
    window.draw(hint);
}

// =========================================================
void Settings::drawSectionTabs(sf::RenderWindow& window,
    float startX, float startY,
    float tabW, float tabH, float tabGap)
{
    const char* tabs[3] = { "Volume", "P1 Keys", "P2 Keys" };

    for (int i = 0; i < 3; i++)
    {
        bool active = (i == m_section);
        float x = startX + i * (tabW + tabGap);

        // Shadow / glow behind active tab
        if (active)
        {
            sf::RectangleShape glow(sf::Vector2f(tabW + 6.f, tabH + 6.f));
            glow.setPosition(x - 3.f, startY - 3.f);
            glow.setFillColor(sf::Color(80, 180, 255, 60));
            glow.setOutlineColor(sf::Color(120, 210, 255, 140));
            glow.setOutlineThickness(2.f);
            window.draw(glow);
        }

        // Button body
        sf::RectangleShape btn(sf::Vector2f(tabW, tabH));
        btn.setPosition(x, startY);
        btn.setFillColor(active
            ? sf::Color(30, 100, 220, 230)
            : sf::Color(20, 55, 130, 190));
        btn.setOutlineColor(active
            ? sf::Color(160, 230, 255, 255)
            : sf::Color(70, 120, 200, 180));
        btn.setOutlineThickness(active ? 2.5f : 1.5f);
        window.draw(btn);

        // Bright top edge on active
        if (active)
        {
            sf::RectangleShape topEdge(sf::Vector2f(tabW, 3.f));
            topEdge.setPosition(x, startY);
            topEdge.setFillColor(sf::Color(180, 235, 255));
            window.draw(topEdge);
        }

        // Label
        sf::Text txt;
        txt.setFont(m_font);
        txt.setCharacterSize(16);
        txt.setStyle(active ? sf::Text::Bold : sf::Text::Regular);
        txt.setFillColor(active ? sf::Color(255, 255, 255) : sf::Color(160, 200, 240));
        txt.setOutlineColor(sf::Color(10, 30, 90, 180));
        txt.setOutlineThickness(active ? 1.5f : 0.f);
        txt.setString(tabs[i]);
        sf::FloatRect tb = txt.getLocalBounds();
        txt.setPosition(x + (tabW - tb.width) / 2.f, startY + (tabH - tb.height) / 2.f - 2.f);
        window.draw(txt);
    }
}

// =========================================================
void Settings::drawVolume(sf::RenderWindow& window,
    float contentX, float contentY,
    float contentW, float /*contentH*/)
{
    const char* labels[2] = { "Music Volume", "Sound Volume" };
    float volumes[2] = { m_musicVolume, m_soundVolume };

    if (m_rebinding)
    {
        sf::Text rb;
        rb.setFont(m_font);
        rb.setCharacterSize(20);
        rb.setFillColor(sf::Color(255, 220, 80));
        rb.setOutlineColor(sf::Color(20, 60, 150));
        rb.setOutlineThickness(2.f);
        rb.setString("Press a key...");
        sf::FloatRect rbr = rb.getLocalBounds();
        rb.setPosition(contentX + (contentW - rbr.width) / 2.f, contentY + 80.f);
        window.draw(rb);
        return;
    }

    const float cardH = 80.f;
    const float cardGap = 18.f;
    const float sliderW = 220.f;
    const float sliderH = 14.f;
    const float labelX = contentX + 20.f;
    const float barX = contentX + 190.f;

    for (int i = 0; i < 2; i++)
    {
        bool sel = (m_row == i);
        float cardY = contentY + i * (cardH + cardGap);

        // Card background — frosted blue glass on top of image
        sf::RectangleShape card(sf::Vector2f(contentW, cardH));
        card.setPosition(contentX, cardY);
        card.setFillColor(sel
            ? sf::Color(30, 90, 200, 200)
            : sf::Color(15, 50, 130, 170));
        card.setOutlineColor(sel
            ? sf::Color(150, 220, 255, 255)
            : sf::Color(70, 130, 210, 200));
        card.setOutlineThickness(sel ? 2.5f : 1.5f);
        window.draw(card);

        // Left accent strip
        sf::RectangleShape strip(sf::Vector2f(5.f, cardH));
        strip.setPosition(contentX, cardY);
        strip.setFillColor(sel ? sf::Color(180, 235, 255) : sf::Color(80, 150, 230));
        window.draw(strip);

        // Label
        sf::Text lbl;
        lbl.setFont(m_font);
        lbl.setCharacterSize(17);
        lbl.setStyle(sel ? sf::Text::Bold : sf::Text::Regular);
        lbl.setFillColor(sf::Color(255, 255, 255));
        lbl.setOutlineColor(sf::Color(10, 30, 100, 160));
        lbl.setOutlineThickness(1.f);
        lbl.setString(labels[i]);
        lbl.setPosition(labelX + 8.f, cardY + 12.f);
        window.draw(lbl);

        // Slider track
        sf::RectangleShape track(sf::Vector2f(sliderW, sliderH));
        track.setPosition(barX, cardY + 46.f);
        track.setFillColor(sf::Color(10, 30, 90, 200));
        track.setOutlineColor(sf::Color(80, 160, 230));
        track.setOutlineThickness(1.f);
        window.draw(track);

        // Fill
        float fill = sliderW * volumes[i] / 100.f;
        if (fill > 0.f)
        {
            sf::RectangleShape fillBar(sf::Vector2f(fill, sliderH));
            fillBar.setPosition(barX, cardY + 46.f);
            fillBar.setFillColor(sel ? sf::Color(100, 210, 255) : sf::Color(60, 150, 230));
            window.draw(fillBar);

            // Bright tip
            sf::RectangleShape tip(sf::Vector2f(3.f, sliderH));
            tip.setPosition(barX + fill - 3.f, cardY + 46.f);
            tip.setFillColor(sf::Color(220, 245, 255));
            window.draw(tip);
        }

        // Knob
        sf::CircleShape knob(9.f);
        knob.setFillColor(sel ? sf::Color(240, 250, 255) : sf::Color(150, 200, 240));
        knob.setOutlineColor(sf::Color(60, 160, 255));
        knob.setOutlineThickness(2.f);
        knob.setPosition(barX + fill - 9.f, cardY + 44.f);
        window.draw(knob);

        // Percentage
        char pct[8];
        snprintf(pct, sizeof(pct), "%d%%", static_cast<int>(volumes[i]));
        sf::Text pctTxt;
        pctTxt.setFont(m_font);
        pctTxt.setCharacterSize(15);
        pctTxt.setFillColor(sf::Color(200, 235, 255));
        pctTxt.setString(pct);
        pctTxt.setPosition(barX + sliderW + 10.f, cardY + 44.f);
        window.draw(pctTxt);

        // Adjust hint on label line
        if (sel)
        {
            sf::Text arrows;
            arrows.setFont(m_font);
            arrows.setCharacterSize(12);
            arrows.setFillColor(sf::Color(160, 220, 255));
            arrows.setString("< - / + >");
            arrows.setPosition(labelX + 8.f, cardY + 36.f);
            window.draw(arrows);
        }
    }
}

// =========================================================
void Settings::drawKeyBindings(sf::RenderWindow& window,
    float contentX, float contentY,
    float contentW, float /*contentH*/,
    int playerIndex)
{
    int base = playerIndex * 4;

    // Player header
    sf::Text header;
    header.setFont(m_font);
    header.setCharacterSize(16);
    header.setStyle(sf::Text::Bold);
    header.setFillColor(sf::Color(200, 235, 255));
    header.setOutlineColor(sf::Color(10, 40, 120));
    header.setOutlineThickness(1.5f);
    header.setString(playerIndex == 0 ? "Player 1 Key Bindings" : "Player 2 Key Bindings");
    sf::FloatRect hb = header.getLocalBounds();
    header.setPosition(contentX + (contentW - hb.width) / 2.f, contentY);
    window.draw(header);

    const float cardH = 52.f;
    const float cardGap = 10.f;
    const float rowStartY = contentY + 28.f;

    for (int i = 0; i < 4; i++)
    {
        bool sel = (m_row == i);
        float cardY = rowStartY + i * (cardH + cardGap);

        // Card
        sf::RectangleShape card(sf::Vector2f(contentW, cardH));
        card.setPosition(contentX, cardY);
        card.setFillColor(sel
            ? sf::Color(30, 100, 220, 215)
            : sf::Color(15, 50, 130, 175));
        card.setOutlineColor(sel
            ? sf::Color(150, 225, 255, 255)
            : sf::Color(65, 125, 205, 200));
        card.setOutlineThickness(sel ? 2.5f : 1.5f);
        window.draw(card);

        // Left accent strip
        sf::RectangleShape strip(sf::Vector2f(5.f, cardH));
        strip.setPosition(contentX, cardY);
        strip.setFillColor(sel ? sf::Color(180, 235, 255) : sf::Color(70, 140, 220));
        window.draw(strip);

        // Action label
        sf::Text action;
        action.setFont(m_font);
        action.setCharacterSize(15);
        action.setStyle(sel ? sf::Text::Bold : sf::Text::Regular);
        action.setFillColor(sf::Color(255, 255, 255));
        action.setOutlineColor(sf::Color(10, 30, 100, 160));
        action.setOutlineThickness(1.f);
        action.setString(KEY_SLOT_LABELS[base + i]);
        action.setPosition(contentX + 16.f, cardY + (sel ? 8.f : 16.f));
        window.draw(action);

        // Sub-hint on selected row
        if (sel && !m_rebinding)
        {
            sf::Text hint2;
            hint2.setFont(m_font);
            hint2.setCharacterSize(11);
            hint2.setFillColor(sf::Color(160, 215, 255));
            hint2.setString("ENTER to change");
            hint2.setPosition(contentX + 16.f, cardY + 30.f);
            window.draw(hint2);
        }

        // Key badge
        bool isRebinding = m_rebinding && m_rebindSlot == (base + i);
        const float badgeW = 120.f, badgeH = 30.f;
        float badgeX = contentX + contentW - badgeW - 16.f;
        float badgeY = cardY + (cardH - badgeH) / 2.f;

        sf::RectangleShape badge(sf::Vector2f(badgeW, badgeH));
        badge.setPosition(badgeX, badgeY);
        badge.setFillColor(isRebinding
            ? sf::Color(180, 30, 60, 220)
            : sf::Color(20, 65, 165, 230));
        badge.setOutlineColor(isRebinding
            ? sf::Color(255, 130, 130)
            : sf::Color(110, 185, 255));
        badge.setOutlineThickness(2.f);
        window.draw(badge);

        sf::Text keyTxt;
        keyTxt.setFont(m_font);
        keyTxt.setCharacterSize(14);
        keyTxt.setStyle(sf::Text::Bold);
        keyTxt.setFillColor(isRebinding ? sf::Color(255, 190, 190) : sf::Color(210, 240, 255));
        keyTxt.setString(isRebinding ? "[ Press Key ]" : keyName(m_bindings.keys[base + i]));
        sf::FloatRect kb = keyTxt.getLocalBounds();
        keyTxt.setPosition(badgeX + (badgeW - kb.width) / 2.f, badgeY + (badgeH - kb.height) / 2.f - 2.f);
        window.draw(keyTxt);
    }
}