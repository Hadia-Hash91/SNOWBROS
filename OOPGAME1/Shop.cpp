#include "Shop.h"
#include "Constants.h"
#include <cstdio>

Shop::Shop(sf::Font& font) : m_font(font) {}

void Shop::update(float dt)
{
    if (m_msgTimer > 0.f) m_msgTimer -= dt;
}


void Shop::handleMouseMove(float mouseX, float mouseY)
{
    if (!m_open) return;

    m_hoveredItem = -1;
    for (int i = 0; i < SHOP_ITEM_COUNT; i++)
    {
        if (m_rowBounds[i].contains(mouseX, mouseY))
        {
            m_hoveredItem = i;
            m_selection = i;
            break;
        }
    }
}

// ---------------------------------------------------------
bool Shop::handleEvent(sf::Event& event, Player** players, int playerCount)
{
    if (!m_open) return false;

    // ---- Mouse click ----
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left)
    {
        float mx = static_cast<float>(event.mouseButton.x);
        float my = static_cast<float>(event.mouseButton.y);

        for (int i = 0; i < SHOP_ITEM_COUNT; i++)
        {
            if (m_rowBounds[i].contains(mx, my))
            {
                m_selection = i;
                // P1 buys on click; P2 can use keyboard (Numpad0)
                if (playerCount >= 1)
                    applyPurchase(i, *players[m_activePlayer < playerCount ? m_activePlayer : 0]);
                return true;
            }
        }
        // Click outside the item list → close
        close();
        return true;
    }

    // ---- Keyboard only from here ----
    if (event.type != sf::Event::KeyPressed) return false;

    // Close
    if (event.key.code == sf::Keyboard::Escape ||
        event.key.code == sf::Keyboard::P)
    {
        close();
        return true;
    }

    // Navigate
    if (event.key.code == sf::Keyboard::Up)
        m_selection = (m_selection - 1 + SHOP_ITEM_COUNT) % SHOP_ITEM_COUNT;

    if (event.key.code == sf::Keyboard::Down)
        m_selection = (m_selection + 1) % SHOP_ITEM_COUNT;

    // Switch active buyer
    if (event.key.code == sf::Keyboard::Tab && playerCount >= 2)
        m_activePlayer = 1 - m_activePlayer;

    // P1 buys
    if ((event.key.code == sf::Keyboard::Return ||
        event.key.code == sf::Keyboard::Space) &&
        playerCount >= 1)
        applyPurchase(m_selection, *players[0]);

    // P2 buys
    if (event.key.code == sf::Keyboard::Numpad0 && playerCount >= 2)
        applyPurchase(m_selection, *players[1]);

    return false;
}

// ---------------------------------------------------------
void Shop::applyPurchase(int itemIdx, Player& player)
{
    const ShopItem& item = SHOP_ITEMS[itemIdx];
    if (!player.spendGems(item.cost))
    {
        setMessage("Not enough gems!");
        return;
    }

    char buf[128];
    switch (itemIdx)
    {
    case 0:
        player.addLife();
        snprintf(buf, sizeof(buf), "%s got an Extra Life!", player.getName());
        break;
    case 1:
        player.applySpeedBoost(30.f);
        snprintf(buf, sizeof(buf), "%s boosted speed for 30s!", player.getName());
        break;
    case 2:
        player.applySnowballPower();
        snprintf(buf, sizeof(buf), "%s has powered snowballs!", player.getName());
        break;
    case 3:
        player.applyDistanceIncrease();
        snprintf(buf, sizeof(buf), "%s has max-range throws!", player.getName());
        break;
    case 4:
        player.applyBalloonMode(30.f);
        snprintf(buf, sizeof(buf), "%s is in Balloon Mode for 30s!", player.getName());
        break;
    default:
        snprintf(buf, sizeof(buf), "Purchased!");
        break;
    }
    setMessage(buf);
}

// ---------------------------------------------------------
void Shop::draw(sf::RenderWindow& window, Player** players, int playerCount)
{
    if (!m_open) return;

    const float W = static_cast<float>(WINDOW_WIDTH);
    const float H = static_cast<float>(WINDOW_HEIGHT);

    // ── Full-screen gem shop background ──────────────────────
    if (m_bgLoaded)
        window.draw(m_bgSprite);
    else
    {
        sf::RectangleShape fallback(sf::Vector2f(W, H));
        fallback.setFillColor(sf::Color(10, 8, 30));
        window.draw(fallback);
    }

    // Light tint so our UI pops over the art
    sf::RectangleShape tint(sf::Vector2f(W, H));
    tint.setFillColor(sf::Color(0, 0, 10, 60));
    window.draw(tint);

    // ── Panel — positioned inside the dark display box in gem_shop_bg.png ──
    // The image's dark box occupies approx x:125-668, y:150-497 at 800x600
    const float panelX = 130.f;
    const float panelY = 155.f;
    const float panelW = 535.f;
    const float panelH = 338.f;

    sf::RectangleShape panel(sf::Vector2f(panelW, panelH));
    panel.setPosition(panelX, panelY);
    panel.setFillColor(sf::Color(6, 4, 22, 100));   // mostly transparent — dark box from image shows through
    panel.setOutlineColor(sf::Color(200, 80, 180, 0));   // no outline — image frame provides the border
    panel.setOutlineThickness(0.f);
    window.draw(panel);

    // Inner top gloss
    sf::RectangleShape gloss(sf::Vector2f(panelW - 16.f, 3.f));
    gloss.setPosition(panelX + 8.f, panelY + 8.f);
    gloss.setFillColor(sf::Color(255, 120, 220, 40));
    window.draw(gloss);

    // ── Player gem counts / browse note ──────────────────────
    if (playerCount == 0)
    {
        sf::Text note; note.setFont(m_font); note.setCharacterSize(13);
        note.setFillColor(sf::Color(180, 140, 255));
        note.setString("Browse items - start a game to spend gems");
        note.setPosition(panelX + 12.f, panelY + 14.f);
        window.draw(note);
    }
    else
    {
        for (int p = 0; p < playerCount && p < 2; p++)
        {
            char gbuf[48];
            snprintf(gbuf, sizeof(gbuf), "%s: %d gems%s",
                players[p]->getName(), players[p]->getGems(),
                (p == m_activePlayer) ? "  [ACTIVE]" : "");

            sf::Text gtxt; gtxt.setFont(m_font); gtxt.setCharacterSize(13);
            gtxt.setFillColor(p == m_activePlayer
                ? sf::Color(255, 120, 220)   // pink for active
                : sf::Color(100, 180, 255));  // blue for inactive
            gtxt.setString(gbuf);
            gtxt.setPosition(panelX + 12.f, panelY + 12.f + p * 18.f);
            window.draw(gtxt);
        }
    }

    // ── Controls hint ────────────────────────────────────────
    sf::Text ctrl; ctrl.setFont(m_font); ctrl.setCharacterSize(10);
    ctrl.setFillColor(sf::Color(140, 110, 200));
    if (playerCount >= 2)
        ctrl.setString("UP/DOWN nav   CLICK/ENTER = P1 buy   NUM0 = P2 buy   TAB switch   ESC close");
    else
        ctrl.setString("UP/DOWN or hover to navigate   CLICK or ENTER to buy   ESC to close");
    sf::FloatRect cb = ctrl.getLocalBounds();
    ctrl.setPosition(panelX + (panelW - cb.width) / 2.f, panelY + 46.f);
    window.draw(ctrl);

    // Divider
    sf::RectangleShape div(sf::Vector2f(panelW - 20.f, 1.f));
    div.setPosition(panelX + 10.f, panelY + 60.f);
    div.setFillColor(sf::Color(180, 60, 160, 160));
    window.draw(div);

    // ── Item rows ────────────────────────────────────────────
    const float itemH = 46.f;
    float       itemY = panelY + 68.f;

    for (int i = 0; i < SHOP_ITEM_COUNT; i++)
    {
        const ShopItem& item = SHOP_ITEMS[i];
        m_rowBounds[i] = sf::FloatRect(panelX + 10.f, itemY, panelW - 20.f, itemH - 4.f);

        bool hovered = (i == m_hoveredItem);
        bool selected = (i == m_selection);
        bool active = hovered || selected;

        // Row background
        sf::RectangleShape row(sf::Vector2f(panelW - 20.f, itemH - 4.f));
        row.setPosition(panelX + 10.f, itemY);

        if (selected && hovered)
        {
            row.setFillColor(sf::Color(80, 20, 80, 230));
            row.setOutlineColor(sf::Color(255, 120, 220));
            row.setOutlineThickness(2.f);
        }
        else if (hovered)
        {
            row.setFillColor(sf::Color(20, 30, 90, 210));
            row.setOutlineColor(sf::Color(100, 180, 255));
            row.setOutlineThickness(1.5f);
        }
        else if (selected)
        {
            row.setFillColor(sf::Color(60, 15, 65, 210));
            row.setOutlineColor(sf::Color(220, 80, 200));
            row.setOutlineThickness(2.f);
        }
        else
        {
            row.setFillColor(sf::Color(12, 8, 35, 190));
            row.setOutlineColor(sf::Color(60, 40, 100, 140));
            row.setOutlineThickness(1.f);
        }
        window.draw(row);

        // Gloss line inside each row
        sf::RectangleShape rowGloss(sf::Vector2f(panelW - 30.f, 2.f));
        rowGloss.setPosition(panelX + 15.f, itemY + 4.f);
        rowGloss.setFillColor(active ? sf::Color(255, 180, 240, 60)
            : sf::Color(120, 80, 180, 30));
        window.draw(rowGloss);

        // Color swatch (item accent)
        sf::RectangleShape swatch(sf::Vector2f(6.f, itemH - 14.f));
        swatch.setPosition(panelX + 14.f, itemY + 5.f);
        swatch.setFillColor(item.color);
        window.draw(swatch);

        // Item name — pink when active, light blue otherwise
        sf::Text name; name.setFont(m_font); name.setCharacterSize(15);
        name.setStyle(sf::Text::Bold);
        name.setFillColor(active ? sf::Color(255, 140, 230) : sf::Color(210, 180, 255));
        name.setOutlineColor(sf::Color(20, 0, 40, 160));
        name.setOutlineThickness(1.f);
        name.setString(item.name);
        name.setPosition(panelX + 28.f, itemY + 5.f);
        window.draw(name);

        // Description — blue-lavender
        sf::Text desc; desc.setFont(m_font); desc.setCharacterSize(11);
        desc.setFillColor(active ? sf::Color(160, 210, 255) : sf::Color(120, 140, 200));
        desc.setString(item.description);
        desc.setPosition(panelX + 28.f, itemY + 23.f);
        window.draw(desc);

        // Cost badge
        char costBuf[24];
        snprintf(costBuf, sizeof(costBuf), "%d gems", item.cost);

        sf::RectangleShape costBg(sf::Vector2f(72.f, 20.f));
        costBg.setPosition(panelX + panelW - 86.f, itemY + 10.f);
        costBg.setFillColor(active ? sf::Color(180, 40, 160, 220)
            : sf::Color(40, 20, 80, 200));
        costBg.setOutlineColor(active ? sf::Color(255, 120, 220)
            : sf::Color(100, 60, 160));
        costBg.setOutlineThickness(1.f);
        window.draw(costBg);

        sf::Text cost; cost.setFont(m_font); cost.setCharacterSize(12);
        cost.setStyle(sf::Text::Bold);
        cost.setFillColor(active ? sf::Color(255, 220, 255) : sf::Color(200, 160, 255));
        cost.setString(costBuf);
        sf::FloatRect costb = cost.getLocalBounds();
        cost.setPosition(panelX + panelW - 86.f + (72.f - costb.width) / 2.f, itemY + 13.f);
        window.draw(cost);

        // Arrow indicator
        if (active)
        {
            sf::Text arrow; arrow.setFont(m_font); arrow.setCharacterSize(16);
            arrow.setFillColor(hovered ? sf::Color(255, 140, 230)
                : sf::Color(160, 100, 255));
            arrow.setString(">");
            arrow.setPosition(panelX + 2.f, itemY + 8.f);
            window.draw(arrow);
        }

        itemY += itemH;
    }

    // Bottom divider
    sf::RectangleShape divBot(sf::Vector2f(panelW - 20.f, 1.f));
    divBot.setPosition(panelX + 10.f, panelY + panelH - 38.f);
    divBot.setFillColor(sf::Color(100, 40, 160, 140));
    window.draw(divBot);

    // ── Feedback message ─────────────────────────────────────
    if (m_msgTimer > 0.f && m_message[0] != '\0')
    {
        bool isError = (m_message[0] == 'N');
        sf::RectangleShape msgBg(sf::Vector2f(panelW - 20.f, 24.f));
        msgBg.setPosition(panelX + 10.f, panelY + panelH - 32.f);
        msgBg.setFillColor(isError ? sf::Color(80, 10, 10, 210)
            : sf::Color(10, 40, 80, 210));
        window.draw(msgBg);

        sf::Text msg; msg.setFont(m_font); msg.setCharacterSize(14);
        msg.setFillColor(isError ? sf::Color(255, 80, 100)
            : sf::Color(100, 220, 255));
        msg.setString(m_message);
        sf::FloatRect mb = msg.getLocalBounds();
        msg.setPosition(panelX + (panelW - mb.width) / 2.f, panelY + panelH - 30.f);
        window.draw(msg);
    }
}