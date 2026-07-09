

#include "CharacterSelect.h"
#include "Constants.h"
#include <cstdio>
#include <cstring>

CharacterSelect::CharacterSelect(sf::Font& font) : m_font(font) {}

// =========================================================
void CharacterSelect::reset(int numPlayers)
{
    m_numPlayers = numPlayers;
    m_selected[0] = m_defaultCharacter; // remember last chosen
    m_selected[1] = 1;
    m_confirmed[0] = false;
    m_confirmed[1] = false;
    m_done = false;
    m_cancelled = false;
    m_browseOnly = false;
}

void CharacterSelect::resetBrowse()
{
    m_numPlayers = 1;
    m_selected[0] = m_defaultCharacter;
    m_selected[1] = 1;
    m_confirmed[0] = false;
    m_confirmed[1] = false;
    m_done = false;
    m_cancelled = false;
    m_browseOnly = true;
}

// =========================================================

bool CharacterSelect::update(sf::Event& event, int numPlayers)
{
    if (event.type != sf::Event::KeyPressed) return false;

    // ESC always cancels
    if (event.key.code == sf::Keyboard::Escape) {
        m_cancelled = true;
        m_done = true;
        return true;
    }

    // ---- Browse-only mode (unchanged) ----
    if (m_browseOnly)
    {
        if (event.key.code == sf::Keyboard::A || event.key.code == sf::Keyboard::Left)
            m_selected[0] = (m_selected[0] - 1 + CHARACTER_COUNT) % CHARACTER_COUNT;
        if (event.key.code == sf::Keyboard::D || event.key.code == sf::Keyboard::Right)
            m_selected[0] = (m_selected[0] + 1) % CHARACTER_COUNT;
        if (event.key.code == sf::Keyboard::W || event.key.code == sf::Keyboard::Up)
            m_selected[0] = (m_selected[0] - 3 + CHARACTER_COUNT) % CHARACTER_COUNT;
        if (event.key.code == sf::Keyboard::S || event.key.code == sf::Keyboard::Down)
            m_selected[0] = (m_selected[0] + 3) % CHARACTER_COUNT;
        if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
            m_defaultCharacter = m_selected[0];
            m_confirmed[0] = true;
            m_done = true;
            return true;
        }
        return false;
    }

    // ---- Normal mode ----
    // P1 = WASD + SPACE  (arrows also work in 1-player mode)
    if (!m_confirmed[0])
    {
        bool p1CanUseArrows = (numPlayers == 1);
        if (event.key.code == sf::Keyboard::A ||
            (p1CanUseArrows && event.key.code == sf::Keyboard::Left))
            m_selected[0] = (m_selected[0] - 1 + CHARACTER_COUNT) % CHARACTER_COUNT;
        if (event.key.code == sf::Keyboard::D ||
            (p1CanUseArrows && event.key.code == sf::Keyboard::Right))
            m_selected[0] = (m_selected[0] + 1) % CHARACTER_COUNT;
        if (event.key.code == sf::Keyboard::W ||
            (p1CanUseArrows && event.key.code == sf::Keyboard::Up))
            m_selected[0] = (m_selected[0] - 3 + CHARACTER_COUNT) % CHARACTER_COUNT;
        if (event.key.code == sf::Keyboard::S ||
            (p1CanUseArrows && event.key.code == sf::Keyboard::Down))
            m_selected[0] = (m_selected[0] + 3) % CHARACTER_COUNT;
        if (event.key.code == sf::Keyboard::Space) {
            m_confirmed[0] = true;
            m_defaultCharacter = m_selected[0];
        }
    }

    if (!m_confirmed[1])
    {
        if (event.key.code == sf::Keyboard::Left)
            m_selected[1] = (m_selected[1] - 1 + CHARACTER_COUNT) % CHARACTER_COUNT;
        if (event.key.code == sf::Keyboard::Right)
            m_selected[1] = (m_selected[1] + 1) % CHARACTER_COUNT;
        if (event.key.code == sf::Keyboard::Up)
            m_selected[1] = (m_selected[1] - 3 + CHARACTER_COUNT) % CHARACTER_COUNT;
        if (event.key.code == sf::Keyboard::Down)
            m_selected[1] = (m_selected[1] + 3) % CHARACTER_COUNT;
        if (event.key.code == sf::Keyboard::RShift ||
            event.key.code == sf::Keyboard::Return)
            m_confirmed[1] = true;
    }

    if (numPlayers == 1 && m_confirmed[0]) { m_confirmed[1] = true; m_done = true; }
    if (numPlayers >= 2 && m_confirmed[0] && m_confirmed[1]) m_done = true;

    return m_done;
}
// =========================================================
void CharacterSelect::draw(sf::RenderWindow& window)
{
    const float W = static_cast<float>(WINDOW_WIDTH);
    const float H = static_cast<float>(WINDOW_HEIGHT);

    // ── Background ──────────────────────────────────────────
    if (m_bgLoaded)
        window.draw(m_bgSprite);
    else
    {
        sf::RectangleShape bg(sf::Vector2f(W, H));
        bg.setFillColor(sf::Color(8, 10, 35));
        window.draw(bg);
    }

    sf::RectangleShape tint(sf::Vector2f(W, H));
    tint.setFillColor(sf::Color(0, 0, 20, 80));
    window.draw(tint);

    // ── Title bar ───────────────────────────────────────────
    sf::RectangleShape titleBg(sf::Vector2f(480.f, 48.f));
    titleBg.setPosition((W - 480.f) / 2.f, 8.f);
    titleBg.setFillColor(sf::Color(15, 25, 80, 210));
    titleBg.setOutlineColor(sf::Color(80, 160, 255, 220));
    titleBg.setOutlineThickness(2.f);
    window.draw(titleBg);

    sf::Text title;
    title.setFont(m_font);
    title.setCharacterSize(28);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(sf::Color(180, 220, 255));
    title.setOutlineColor(sf::Color(20, 60, 180));
    title.setOutlineThickness(2.f);
    title.setString(m_browseOnly ? "CHARACTER ROSTER" : "SELECT YOUR CHARACTER");
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition((W - tb.width) / 2.f, 16.f);
    window.draw(title);

    // ── Controls hint ────────────────────────────────────────
    sf::Text hint;
    hint.setFont(m_font);
    hint.setCharacterSize(12);
    hint.setFillColor(sf::Color(140, 190, 255));
    if (m_browseOnly)
        hint.setString("A/D or LEFT/RIGHT to browse   UP/DOWN row   ENTER to set default   ESC back");
    else if (m_numPlayers == 1)
        hint.setString("P1: A / D to browse   SPACE to confirm   ESC to go back");
    else
        hint.setString("P1: A/D + SPACE    P2: Arrow Keys + Enter    ESC to cancel");
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setPosition((W - hb.width) / 2.f, 60.f);
    window.draw(hint);

   
    float cardW, cardH, startX, startY, gapX, gapY;

    if (m_browseOnly)
    {
        cardW = 118.f; cardH = 130.f;
        gapX = 12.f;  gapY = 10.f;
        startX = (W - (3 * cardW + 2 * gapX)) / 2.f;
        startY = 80.f;
    }
    else
    {
        // Match the 3x2 frame layout in the background image
        cardW = 218.f; cardH = 212.f;
        gapX = 28.f;  gapY = 24.f;
        startX = (W - (3 * cardW + 2 * gapX)) / 2.f;
        startY = 88.f;
    }

    for (int i = 0; i < CHARACTER_COUNT; i++)
    {
        int   col = i % 3, row = i / 3;
        float cx = startX + col * (cardW + gapX);
        float cy = startY + row * (cardH + gapY);

        bool hiP1 = (m_selected[0] == i);
        bool hiP2 = (m_numPlayers >= 2 && m_selected[1] == i);
        bool confP1 = (m_confirmed[0] && m_selected[0] == i);
        bool confP2 = (m_numPlayers >= 2 && m_confirmed[1] && m_selected[1] == i);

        bool highlighted = hiP1 || hiP2;
        bool confirmed = confP1 || confP2;
        int  owner = confP1 ? 0 : (confP2 ? 1 : -1);
        sf::Color pColor = (owner == 0) ? sf::Color(80, 160, 255)
            : (owner == 1) ? sf::Color(255, 130, 60)
            : sf::Color(80, 80, 100);

        if (m_browseOnly)
        {
            // Compact browse card — blue theme
            sf::RectangleShape card(sf::Vector2f(cardW, cardH));
            card.setPosition(cx, cy);
            card.setFillColor(hiP1 ? sf::Color(20, 45, 110, 220)
                : sf::Color(10, 18, 55, 200));
            card.setOutlineThickness(hiP1 ? 3.f : 1.5f);
            card.setOutlineColor(hiP1 ? sf::Color(80, 180, 255)
                : sf::Color(40, 70, 140));
            window.draw(card);

            const CharacterData& cd = CHARACTER_ROSTER[i];
            float sSize = 36.f;
            float sX = cx + (cardW - sSize) / 2.f, sY = cy + 10.f;

            if (m_charImgLoaded[i])
            {
                sf::Sprite& spr = m_charSprite[i];
                sf::Vector2u ts = m_charTexture[i].getSize();
                float scale = sSize / (float)(ts.x > ts.y ? ts.x : ts.y);
                spr.setScale(scale, scale);
                float offX = (sSize - ts.x * scale) / 2.f;
                float offY = (sSize - ts.y * scale) / 2.f;
                spr.setPosition(sX + offX, sY + offY);
                spr.setColor(sf::Color(255, 255, 255, 230));
                window.draw(spr);
            }
            else
            {
                sf::RectangleShape body(sf::Vector2f(sSize, sSize));
                body.setPosition(sX, sY);
                body.setFillColor(cd.bodyColor);
                body.setOutlineColor(cd.accentColor);
                body.setOutlineThickness(2.f);
                window.draw(body);

                sf::CircleShape eye(3.f);
                eye.setFillColor(sf::Color::White);
                eye.setPosition(sX + 6.f, sY + 8.f);  window.draw(eye);
                eye.setPosition(sX + 20.f, sY + 8.f);  window.draw(eye);
                sf::CircleShape pupil(1.5f);
                pupil.setFillColor(sf::Color::Black);
                pupil.setPosition(sX + 8.f, sY + 10.f); window.draw(pupil);
                pupil.setPosition(sX + 22.f, sY + 10.f); window.draw(pupil);
            }

            sf::Text name; name.setFont(m_font); name.setCharacterSize(14);
            name.setFillColor(hiP1 ? sf::Color(140, 210, 255) : sf::Color::White);
            name.setString(cd.name);
            sf::FloatRect nb = name.getLocalBounds();
            name.setPosition(cx + (cardW - nb.width) / 2.f, sY + sSize + 4.f);
            window.draw(name);

            sf::Text stat; stat.setFont(m_font); stat.setCharacterSize(10);
            stat.setFillColor(sf::Color(120, 200, 255));
            stat.setString(cd.statLine);
            sf::FloatRect stb = stat.getLocalBounds();
            stat.setPosition(cx + (cardW - stb.width) / 2.f, sY + sSize + 22.f);
            window.draw(stat);

            if (i == m_defaultCharacter)
            {
                sf::RectangleShape badge(sf::Vector2f(cardW - 10.f, 16.f));
                badge.setPosition(cx + 5.f, cy + cardH - 20.f);
                badge.setFillColor(sf::Color(20, 60, 160, 210));
                window.draw(badge);
                sf::Text def; def.setFont(m_font); def.setCharacterSize(10);
                def.setFillColor(sf::Color(140, 210, 255));
                def.setString("DEFAULT");
                sf::FloatRect db = def.getLocalBounds();
                def.setPosition(cx + (cardW - db.width) / 2.f, cy + cardH - 18.f);
                window.draw(def);
            }
        }
        else
        {
            drawCharacterCard(window, i, cx, cy,
                highlighted, confirmed, owner, pColor);

            if (hiP1 && hiP2 && m_numPlayers >= 2)
            {
                sf::Text dbl; dbl.setFont(m_font); dbl.setCharacterSize(11);
                dbl.setFillColor(sf::Color(140, 210, 255));
                dbl.setString("P1+P2");
                dbl.setPosition(cx + cardW - 48.f, cy + 3.f);
                window.draw(dbl);
            }
        }
    }

    if (m_browseOnly)
        drawDetailPanel(window, m_selected[0]);

    // ── Status bar at bottom ─────────────────────────────────
    if (!m_browseOnly)
    {
        sf::RectangleShape bar(sf::Vector2f(W, 40.f));
        bar.setPosition(0.f, H - 40.f);
        bar.setFillColor(sf::Color(5, 12, 50, 220));
        bar.setOutlineColor(sf::Color(60, 130, 255, 180));
        bar.setOutlineThickness(1.5f);
        window.draw(bar);

        char statusBuf[128];
        if (m_numPlayers == 1)
        {
            if (!m_confirmed[0])
                snprintf(statusBuf, sizeof(statusBuf),
                    "P1 browsing: %s", CHARACTER_ROSTER[m_selected[0]].name);
            else
                snprintf(statusBuf, sizeof(statusBuf),
                    "P1 ready: %s  —  Starting!", CHARACTER_ROSTER[m_selected[0]].name);
        }
        else
        {
            const char* p1s = m_confirmed[0] ? "READY" : "browsing";
            const char* p2s = m_confirmed[1] ? "READY" : "browsing";
            snprintf(statusBuf, sizeof(statusBuf),
                "P1 %s: %-8s     P2 %s: %-8s",
                p1s, CHARACTER_ROSTER[m_selected[0]].name,
                p2s, CHARACTER_ROSTER[m_selected[1]].name);
        }

        sf::Text status; status.setFont(m_font); status.setCharacterSize(15);
        status.setStyle(sf::Text::Bold);
        status.setFillColor(sf::Color(180, 220, 255));
        status.setString(statusBuf);
        sf::FloatRect sb2 = status.getLocalBounds();
        status.setPosition((W - sb2.width) / 2.f, H - 30.f);
        window.draw(status);
    }
}

// =========================================================
// Detail panel shown at the bottom in browse mode
// =========================================================
void CharacterSelect::drawDetailPanel(sf::RenderWindow& window, int charIdx)
{
    const CharacterData& cd = CHARACTER_ROSTER[charIdx];

    float panelX = 20.f;
    float panelY = 290.f;
    float panelW = WINDOW_WIDTH - 40.f;
    float panelH = WINDOW_HEIGHT - panelY - 10.f;

    sf::RectangleShape panel(sf::Vector2f(panelW, panelH));
    panel.setPosition(panelX, panelY);
    panel.setFillColor(sf::Color(12, 20, 48, 230));
    panel.setOutlineColor(sf::Color(80, 130, 255));
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    // Large character portrait on the left
    float spriteSize = 72.f;
    float sX = panelX + 18.f;
    float sY = panelY + (panelH - spriteSize) / 2.f;

    if (m_charImgLoaded[charIdx])
    {
        sf::Sprite& spr = m_charSprite[charIdx];
        sf::Vector2u ts = m_charTexture[charIdx].getSize();
        float scale = spriteSize / (float)(ts.x > ts.y ? ts.x : ts.y);
        spr.setScale(scale, scale);
        float offX = (spriteSize - ts.x * scale) / 2.f;
        float offY = (spriteSize - ts.y * scale) / 2.f;
        spr.setPosition(sX + offX, sY + offY);
        spr.setColor(sf::Color(255, 255, 255, 255));
        window.draw(spr);
    }
    else
    {
        sf::RectangleShape body(sf::Vector2f(spriteSize, spriteSize));
        body.setPosition(sX, sY);
        body.setFillColor(cd.bodyColor);
        body.setOutlineColor(cd.accentColor);
        body.setOutlineThickness(3.f);
        window.draw(body);

        // Eyes
        sf::CircleShape eye(6.f);
        eye.setFillColor(sf::Color::White);
        eye.setPosition(sX + 12.f, sY + 18.f); window.draw(eye);
        eye.setPosition(sX + 42.f, sY + 18.f); window.draw(eye);
        sf::CircleShape pupil(3.f);
        pupil.setFillColor(sf::Color::Black);
        pupil.setPosition(sX + 16.f, sY + 22.f); window.draw(pupil);
        pupil.setPosition(sX + 46.f, sY + 22.f); window.draw(pupil);
        sf::RectangleShape smile(sf::Vector2f(32.f, 3.f));
        smile.setFillColor(sf::Color::White);
        smile.setPosition(sX + 20.f, sY + 50.f);
        window.draw(smile);
    }

    // Name
    sf::Text name;
    name.setFont(m_font);
    name.setCharacterSize(22);
    name.setFillColor(sf::Color(255, 220, 60));
    name.setString(cd.name);
    name.setPosition(sX + spriteSize + 16.f, panelY + 10.f);
    window.draw(name);

    // Stat line
    sf::Text statLine;
    statLine.setFont(m_font);
    statLine.setCharacterSize(13);
    statLine.setFillColor(sf::Color(160, 240, 160));
    statLine.setString(cd.statLine);
    statLine.setPosition(sX + spriteSize + 16.f, panelY + 38.f);
    window.draw(statLine);

    // Lives indicator
    char livesBuf[32];
    snprintf(livesBuf, sizeof(livesBuf), "Starting Lives: %d", cd.startLives);
    sf::Text livesTxt;
    livesTxt.setFont(m_font);
    livesTxt.setCharacterSize(13);
    livesTxt.setFillColor(sf::Color(220, 100, 100));
    livesTxt.setString(livesBuf);
    livesTxt.setPosition(sX + spriteSize + 16.f, panelY + 56.f);
    window.draw(livesTxt);

    // Description (multi-line)
    const char* desc = cd.description;
    float dy = panelY + 78.f;
    float descX = sX + spriteSize + 16.f;
    char line[80];
    int  li = 0;
    for (int ci = 0; ; ci++)
    {
        char ch = desc[ci];
        if (ch == '\n' || ch == '\0')
        {
            line[li] = '\0';
            sf::Text dl;
            dl.setFont(m_font);
            dl.setCharacterSize(13);
            dl.setFillColor(sf::Color(190, 190, 210));
            dl.setString(line);
            dl.setPosition(descX, dy);
            window.draw(dl);
            dy += 17.f;
            li = 0;
            if (ch == '\0') break;
        }
        else if (li < 78)
            line[li++] = ch;
    }

    // Stat bars (Speed / Power / Defence)
    struct StatBar { const char* label; float value; sf::Color color; };
    StatBar bars[3] = {
        { "Speed",   cd.speedMult / 1.5f,        sf::Color(60, 180, 255) },
        { "Throw",   1.f - (cd.throwRate - 0.7f) / 0.4f, sf::Color(255, 140, 60) },
        { "Lives",   cd.startLives / 3.f,        sf::Color(220, 80, 80)  }
    };
    float barStartX = panelX + panelW - 200.f;
    float barStartY = panelY + 12.f;
    float barMaxW = 160.f;
    float barH = 14.f;

    for (int b = 0; b < 3; b++)
    {
        float bY = barStartY + b * 30.f;

        sf::Text lbl;
        lbl.setFont(m_font);
        lbl.setCharacterSize(12);
        lbl.setFillColor(sf::Color(180, 180, 200));
        lbl.setString(bars[b].label);
        lbl.setPosition(barStartX, bY);
        window.draw(lbl);

        sf::RectangleShape track(sf::Vector2f(barMaxW, barH));
        track.setPosition(barStartX + 52.f, bY + 1.f);
        track.setFillColor(sf::Color(20, 20, 45));
        track.setOutlineColor(sf::Color(50, 50, 80));
        track.setOutlineThickness(1.f);
        window.draw(track);

        float fill = bars[b].value;
        if (fill < 0.f) fill = 0.f;
        if (fill > 1.f) fill = 1.f;

        sf::RectangleShape fillBar(sf::Vector2f(barMaxW * fill, barH));
        fillBar.setPosition(barStartX + 52.f, bY + 1.f);
        fillBar.setFillColor(bars[b].color);
        window.draw(fillBar);
    }

    // "ENTER to set as default" prompt
    sf::Text setDefault;
    setDefault.setFont(m_font);
    setDefault.setCharacterSize(13);
    setDefault.setFillColor(sf::Color(100, 200, 100));
    setDefault.setString("ENTER — set as default character for New Game");
    sf::FloatRect sdb = setDefault.getLocalBounds();
    setDefault.setPosition(panelX + (panelW - sdb.width) / 2.f,
        panelY + panelH - 22.f);
    window.draw(setDefault);
}

// =========================================================
void CharacterSelect::drawCharacterCard(sf::RenderWindow& window,
    int charIdx, float cx, float cy,
    bool isHighlighted, bool isConfirmed,
    int playerOwner, sf::Color playerColor)
{
    const CharacterData& cd = CHARACTER_ROSTER[charIdx];
    const float cardW = 218.f, cardH = 212.f;

    // Card background — translucent blue, brighter when selected
    sf::RectangleShape card(sf::Vector2f(cardW, cardH));
    card.setPosition(cx, cy);
    sf::Color bgCol = isConfirmed ? sf::Color(10, 40, 30, 200)
        : isHighlighted ? sf::Color(15, 35, 100, 210)
        : sf::Color(5, 12, 45, 190);
    card.setFillColor(bgCol);
    card.setOutlineThickness(isHighlighted ? 3.f : 1.5f);
    card.setOutlineColor(isConfirmed ? sf::Color(60, 220, 120)
        : isHighlighted ? playerColor
        : sf::Color(35, 70, 150, 180));
    window.draw(card);

    // Top gloss line
    sf::RectangleShape gloss(sf::Vector2f(cardW - 16.f, 3.f));
    gloss.setPosition(cx + 8.f, cy + 8.f);
    gloss.setFillColor(isHighlighted ? sf::Color(120, 180, 255, 100)
        : sf::Color(60, 100, 200, 60));
    window.draw(gloss);

    // Character portrait — image if loaded, coloured block fallback
    float spriteSize = 60.f;
    float spriteX = cx + (cardW - spriteSize) / 2.f;
    float spriteY = cy + 18.f;

    if (m_charImgLoaded[charIdx])
    {
        sf::Sprite& spr = m_charSprite[charIdx];
        sf::Vector2u ts = m_charTexture[charIdx].getSize();
        float scale = spriteSize / (float)(ts.x > ts.y ? ts.x : ts.y);
        spr.setScale(scale, scale);
        float offX = (spriteSize - ts.x * scale) / 2.f;
        float offY = (spriteSize - ts.y * scale) / 2.f;
        spr.setPosition(spriteX + offX, spriteY + offY);
        spr.setColor(isHighlighted ? sf::Color(255, 255, 255, 230)
            : sf::Color(200, 200, 200, 200));
        window.draw(spr);
    }
    else
    {
        sf::RectangleShape body(sf::Vector2f(spriteSize, spriteSize));
        body.setPosition(spriteX, spriteY);
        body.setFillColor(cd.bodyColor);
        body.setOutlineColor(isHighlighted ? sf::Color(200, 230, 255) : cd.accentColor);
        body.setOutlineThickness(2.5f);
        window.draw(body);

        // Eyes
        sf::CircleShape eye(4.5f);
        eye.setFillColor(sf::Color::White);
        eye.setPosition(spriteX + 10.f, spriteY + 14.f); window.draw(eye);
        eye.setPosition(spriteX + 34.f, spriteY + 14.f); window.draw(eye);
        sf::CircleShape pupil(2.f);
        pupil.setFillColor(sf::Color::Black);
        pupil.setPosition(spriteX + 13.f, spriteY + 17.f); window.draw(pupil);
        pupil.setPosition(spriteX + 37.f, spriteY + 17.f); window.draw(pupil);
        sf::RectangleShape smile(sf::Vector2f(24.f, 2.f));
        smile.setFillColor(sf::Color::White);
        smile.setPosition(spriteX + 18.f, spriteY + 40.f);
        window.draw(smile);
    }

    // Divider line
    sf::RectangleShape divider(sf::Vector2f(cardW - 20.f, 1.f));
    divider.setPosition(cx + 10.f, cy + 88.f);
    divider.setFillColor(isHighlighted ? sf::Color(80, 150, 255, 160)
        : sf::Color(40, 80, 160, 100));
    window.draw(divider);

    // Name
    sf::Text name; name.setFont(m_font); name.setCharacterSize(17);
    name.setStyle(sf::Text::Bold);
    name.setFillColor(isHighlighted ? sf::Color(140, 210, 255) : sf::Color(200, 225, 255));
    name.setOutlineColor(sf::Color(0, 10, 50, 160));
    name.setOutlineThickness(1.f);
    name.setString(cd.name);
    sf::FloatRect nb = name.getLocalBounds();
    name.setPosition(cx + (cardW - nb.width) / 2.f, cy + 94.f);
    window.draw(name);

    // Stat line
    sf::Text stat; stat.setFont(m_font); stat.setCharacterSize(11);
    stat.setFillColor(sf::Color(100, 180, 255));
    stat.setString(cd.statLine);
    sf::FloatRect stb = stat.getLocalBounds();
    stat.setPosition(cx + (cardW - stb.width) / 2.f, cy + 116.f);
    window.draw(stat);

    const char* desc = cd.description;
    float dy = cy + 132.f;
    char line[64]; int li = 0;
    for (int ci = 0; ; ci++)
    {
        char ch = desc[ci];
        if (ch == '\n' || ch == '\0')
        {
            line[li] = '\0';
            sf::Text dl; dl.setFont(m_font); dl.setCharacterSize(10);
            dl.setFillColor(sf::Color(150, 185, 230));
            dl.setString(line);
            sf::FloatRect dlb = dl.getLocalBounds();
            dl.setPosition(cx + (cardW - dlb.width) / 2.f, dy);
            window.draw(dl);
            dy += 13.f; li = 0;
            if (ch == '\0') break;
        }
        else if (li < 62) line[li++] = ch;
    }

    if (playerOwner >= 0)
    {
        sf::RectangleShape badge(sf::Vector2f(38.f, 18.f));
        badge.setPosition(cx + 4.f, cy + 4.f);
        badge.setFillColor(playerOwner == 0 ? sf::Color(30, 80, 210, 210)
            : sf::Color(200, 80, 30, 210));
        window.draw(badge);
        sf::Text ptxt; ptxt.setFont(m_font); ptxt.setCharacterSize(11);
        ptxt.setStyle(sf::Text::Bold);
        ptxt.setFillColor(sf::Color::White);
        ptxt.setString(playerOwner == 0 ? "P1" : "P2");
        ptxt.setPosition(cx + 10.f, cy + 5.f);
        window.draw(ptxt);
    }

    if (isConfirmed)
    {
        sf::RectangleShape readyBg(sf::Vector2f(cardW - 8.f, 22.f));
        readyBg.setPosition(cx + 4.f, cy + cardH - 26.f);
        readyBg.setFillColor(sf::Color(10, 60, 30, 210));
        window.draw(readyBg);
        sf::Text tick; tick.setFont(m_font); tick.setCharacterSize(14);
        tick.setStyle(sf::Text::Bold);
        tick.setFillColor(sf::Color(60, 220, 120));
        tick.setString("READY!");
        sf::FloatRect tkb = tick.getLocalBounds();
        tick.setPosition(cx + (cardW - tkb.width) / 2.f, cy + cardH - 24.f);
        window.draw(tick);
    }
}

void CharacterSelect::drawStarBar(sf::RenderWindow& window,
    const char* label, int stars,
    float x, float y)
{
    sf::Text lbl;
    lbl.setFont(m_font);
    lbl.setCharacterSize(12);
    lbl.setFillColor(sf::Color(180, 180, 200));
    lbl.setString(label);
    lbl.setPosition(x, y);
    window.draw(lbl);

    for (int i = 0; i < 3; i++)
    {
        sf::CircleShape star(5.f);
        star.setFillColor(i < stars ? sf::Color(255, 210, 0) : sf::Color(50, 50, 70));
        star.setPosition(x + 60.f + i * 14.f, y + 1.f);
        window.draw(star);
    }
}