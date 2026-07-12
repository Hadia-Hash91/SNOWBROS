
#include <iostream>
#include "Botom.h"
#include "Constants.h"
#include <cstdlib>
#include <cmath>
#include <filesystem>

constexpr int Botom::ANIM_FRAMES[10];
constexpr int Botom::ANIM_ROW_Y[10];


bool Botom::loadAndRecolor(sf::Texture& tex,
    const std::string& path,
    EnemyVariant targetVariant,
    bool skipRecolor)
{
    if (!tex.loadFromFile(path)) return false;

    sf::Image img = tex.copyToImage();

    sf::Color bgColor = img.getPixel(0, 0);
    if (bgColor.a > 10)
        img.createMaskFromColor(bgColor);

    if (!skipRecolor)
        recolorImageToVariant(img, targetVariant);

    tex.loadFromImage(img);
    return true;
}


Botom::Botom(float x, float y, EnemyVariant variant)
    : Enemy(x, y, variant)
{

    std::cout << std::filesystem::current_path() << "\n";
    m_texture.setSmooth(false);
    m_hasTexture = false;
    std::cout << "Botom created with variant: " << static_cast<int>(variant) << "\n";
    switch (variant)
    {
    case EnemyVariant::Blue:
        m_hasTexture = loadAndRecolor(m_texture,
            "assets/images/Botom_Blue.png",
            EnemyVariant::Blue,
            /*skipRecolor=*/true);
        break;

    case EnemyVariant::Orange:
        m_hasTexture = loadAndRecolor(m_texture,
            "assets/images/Botom_Orange.png",
            EnemyVariant::Orange,
            /*skipRecolor=*/true);
        break;

    case EnemyVariant::Red:
        m_hasTexture = loadAndRecolor(m_texture,
            "assets/images/Botom_Pink.png",
            EnemyVariant::Red,
            /*skipRecolor=*/true);
        break;

    case EnemyVariant::Green:
        m_hasTexture = loadAndRecolor(m_texture,
            "assets/images/Botom_Pink.png",
            EnemyVariant::Green);
        break;

    case EnemyVariant::Purple:
        m_hasTexture = loadAndRecolor(m_texture,
            "assets/images/Botom_Pink.png",
            EnemyVariant::Purple);
        break;
    }

    if (m_hasTexture)
    {
        m_sprite.setTexture(m_texture);
        m_sprite.setOrigin(m_frameWidth / 2.f, m_frameHeight / 2.f);
        setAnimState(BotomAnim::Walking);
    }
    else
    {
        std::cout << "Botom: failed to load texture for variant "
            << static_cast<int>(variant) << "\n";
    }

    m_speed = 80.f;
    m_hitsToEncase = 2;
    m_health = 1;
    m_scoreValue = SCORE_BOTOM_MIN + (rand() % (SCORE_BOTOM_MAX - SCORE_BOTOM_MIN));

    applyVariantModifiers();

    m_velocity.x = m_speed;
    m_facingRight = true;
    m_directionInterval = 2.f + static_cast<float>(rand() % 3);

    m_hitBox = new HitBox(24.f, 30.f, 6.f, 4.f);
    m_hitBox->update(m_position);
}


void Botom::applyFrameRect()
{
    int row = static_cast<int>(m_animState);

    int maxFrames = ANIM_FRAMES[row];
    if (m_frame < 0)           m_frame = 0;
    if (m_frame >= maxFrames)  m_frame = 0;

    m_sprite.setTextureRect(sf::IntRect(
        m_frame * m_frameWidth,
        ANIM_ROW_Y[row],
        m_frameWidth,
        m_frameHeight
    ));
}

void Botom::setAnimState(BotomAnim state)
{
    if (m_animState == state) return;
    m_animState = state;
    m_frame = 0;
    m_animTimer = 0.f;
    applyFrameRect();
}

void Botom::updateAnimation(float dt)
{
    BotomAnim desired;

    if (m_snowState == SnowState::Full)
        desired = BotomAnim::Trapped;
    else if (!m_onGround && m_velocity.y < 0.f)
        desired = BotomAnim::Jumping;
    else if (!m_onGround && m_velocity.y > 0.f)
        desired = BotomAnim::Falling;
    else
        desired = BotomAnim::Walking;

    setAnimState(desired);

    m_animTimer += dt;
    if (m_animTimer >= m_animSpeed)
    {
        m_animTimer = 0.f;

        int row = static_cast<int>(m_animState);
        int maxFrames = ANIM_FRAMES[row];

        m_frame++;
        if (m_frame >= maxFrames) m_frame = 0;

        applyFrameRect();
    }
}


void Botom::update(float dt)
{
    if (!m_active) return;

    if (m_snowState == SnowState::Full)
    {
        m_escapeTimer -= dt;
        if (m_escapeTimer <= 0.f)
        {
            m_snowState = SnowState::None;
            m_snowHits = 0;
            m_escapeTimer = 5.f;
            m_velocity.x = m_speed * (m_facingRight ? 1.f : -1.f);
        }
        m_hitBox->update(m_position);
        updateAnimation(dt);
        return;
    }

    if (!m_onGround)
    {
        m_velocity.y += GRAVITY * dt;
        if (m_velocity.y > 1200.f) m_velocity.y = 1200.f;
    }
    else
    {
        m_velocity.y = 0.f;
    }

    m_velocity.x = m_speed * (m_facingRight ? 1.f : -1.f);

    m_position.x += m_velocity.x * dt;
    m_position.y += m_velocity.y * dt;

    if (m_position.x < 0.f)
    {
        m_position.x = 0.f;
        m_facingRight = true;
    }
    if (m_position.x > WINDOW_WIDTH - 32.f)
    {
        m_position.x = static_cast<float>(WINDOW_WIDTH - 32);
        m_facingRight = false;
    }
    if (m_position.y >= WINDOW_HEIGHT - 32.f)
    {
        m_position.y = static_cast<float>(WINDOW_HEIGHT - 32);
        m_velocity.y = 0.f;
        m_onGround = true;
    }
    if (m_position.y < 0.f)
    {
        m_position.y = 0.f;
        m_velocity.y = 0.f;
    }

    m_hitBox->update(m_position);
    updateAnimation(dt);
}


void Botom::tryJump()
{
    if (!m_active) return;
    if (m_snowState == SnowState::Full) return;

    if (m_onGround && (rand() % 80 == 0))
        m_velocity.y = -600.f;
}


void Botom::resolvePlatformCollision(const sf::FloatRect* platforms, int count)
{
    m_onGround = false;

    for (int i = 0; i < count; i++)
    {
        m_hitBox->update(m_position);
        sf::FloatRect eRect = m_hitBox->getRect();
        const sf::FloatRect& plat = platforms[i];

        if (!eRect.intersects(plat)) continue;

        float overlapTop = (eRect.top + eRect.height) - plat.top;
        float overlapBottom = (plat.top + plat.height) - eRect.top;
        float overlapLeft = (eRect.left + eRect.width) - plat.left;
        float overlapRight = (plat.left + plat.width) - eRect.left;

        float minX = (overlapLeft < overlapRight) ? overlapLeft : overlapRight;
        float minY = (overlapTop < overlapBottom) ? overlapTop : overlapBottom;

        if (minY < minX && overlapTop < overlapBottom && m_velocity.y > -200.f)
        {
            m_position.y -= overlapTop;
            m_velocity.y = 0.f;
            m_onGround = true;
            m_hitBox->update(m_position);
        }
        else if (minX < minY)
        {
            if (overlapLeft < overlapRight) m_position.x -= overlapLeft;
            else                            m_position.x += overlapRight;

            m_facingRight = !m_facingRight;
            m_velocity.x = m_speed * (m_facingRight ? 1.f : -1.f);
            m_hitBox->update(m_position);
        }
    }

    if (!m_onGround && m_velocity.y >= 0.f)
    {
        sf::FloatRect probe = m_hitBox->getRect();
        probe.top += probe.height;
        probe.height = 2.f;

        for (int i = 0; i < count; i++)
        {
            if (probe.intersects(platforms[i]))
            {
                m_onGround = true;
                m_velocity.y = 0.f;
                break;
            }
        }
    }
}


void Botom::draw(sf::RenderWindow& window)
{
    if (!m_active) return;

    if (m_snowState == SnowState::Full)
    {
        sf::CircleShape ball(18.f);
        ball.setPosition(m_position.x - 2.f, m_position.y - 2.f);
        ball.setFillColor(sf::Color(240, 240, 255));
        ball.setOutlineColor(sf::Color(150, 200, 255));
        ball.setOutlineThickness(3.f);
        if (m_escapeTimer < 1.5f && static_cast<int>(m_escapeTimer * 8) % 2 == 0)
            ball.setFillColor(sf::Color(255, 150, 150));
        window.draw(ball);
        return;
    }

    if (m_hasTexture)
    {
        const float drawScale = 0.45f;
        float sx = (m_facingRight ? -drawScale : +drawScale);
        m_sprite.setScale(sx, drawScale);
        m_sprite.setPosition(m_position.x + 16.f, m_position.y + 16.f);
        window.draw(m_sprite);
    }
    else
    {
        sf::RectangleShape shape(sf::Vector2f(32.f, 32.f));
        shape.setPosition(m_position);
        shape.setFillColor(getVariantTint());
        window.draw(shape);
    }

    if (m_snowState == SnowState::Partial)
    {
        sf::RectangleShape snow(sf::Vector2f(32.f, 14.f));
        snow.setPosition(m_position);
        snow.setFillColor(sf::Color(255, 255, 255, 180));
        window.draw(snow);
    }
}


void Botom::drawDebug(sf::RenderWindow& window)
{
    if (m_hitBox) m_hitBox->drawDebug(window, sf::Color::Red);
}


bool Botom::takeHit()
{
    if (m_snowState == SnowState::Full) return false;
    m_snowHits++;
    if (m_snowHits >= m_hitsToEncase)
    {
        m_snowState = SnowState::Full;
        m_velocity = sf::Vector2f(0.f, 0.f);
        m_escapeTimer = 5.f;
        return true;
    }
    m_snowState = SnowState::Partial;
    m_velocity.x *= 0.4f;
    return false;
}


void Botom::onChainKill()
{
    m_active = false;
    m_hasDrop = true;
}


DropType Botom::getRandomDrop() const
{
    int r = rand() % 10;
    if (r == 0) return DropType::BottleRed;
    if (r == 1) return DropType::BottleBlue;
    if (r == 2) return DropType::BottleYellow;
    if (r == 3) return DropType::BottleGreen;
    if (r == 4) return DropType::FaceBlue;
    if (r == 5) return DropType::SushiLarge;
    if (r == 6) return DropType::SushiMedium;
    if (r == 7) return DropType::Gem;
    return DropType::SushiSmall;
}