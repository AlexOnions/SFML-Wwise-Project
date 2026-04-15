#include "Obstacle.h"
#include "WwiseWrapper.h"
#include <iostream>
#include <cstdlib>

void Obstacle::init(uint64_t audioID, uint64_t playerID)
{
    m_audioID = audioID;
    m_playerID = playerID;
    active = false;
    shape.setPosition({ -200.f, 0.f });

    //Register Audio ID - For Efficent Audio Use
    //Each Object has a unique audio ID
    //Re using these avoids repeat registration issues as this can be expensive for the CPU
    AK::SoundEngine::RegisterGameObj(m_audioID);
    AK::SoundEngine::SetListeners(m_audioID, &m_playerID, 1);
    std::cout << "Pool obstacle registered, audioID: " << m_audioID << std::endl;
}

void Obstacle::activate(float startX, float platformTopY, float gameSpeed, float floorY)
{
    // Pick a random obstacle shape based on the games speed
    int shapeType = 0;
    if (gameSpeed >= 2.f)      shapeType = rand() % 3;
    else if (gameSpeed >= 1.5f) shapeType = rand() % 2;

    if (shapeType == 0)
    {
        shape.setSize({ 50, 50 });
        shape.setFillColor(sf::Color::Red);
    }
    else if (shapeType == 1)
    {
        shape.setSize({ 20, 70 });
        shape.setFillColor(sf::Color::Yellow);
    }
    else
    {
        shape.setSize({ 80, 30 });
        shape.setFillColor(sf::Color::Magenta);
    }

    //Place Obstacle on platform
    float y = (platformTopY >= 0.f)
        ? platformTopY - shape.getSize().y
        : floorY - shape.getSize().y;

    shape.setPosition({ startX, y });

    active = true;
    m_soundPlaying = false;

    std::cout << "Obstacle activated, audioID: " << m_audioID << std::endl;
}

void Obstacle::deactivate()
{
    if (m_soundPlaying)
    {
        //Insure sound is turned off when deactivated
        AK::SoundEngine::PostEvent(AKTEXT("Stop_EnemyBuzz"), m_audioID);
        m_soundPlaying = false;
    }
    active = false;
    shape.setPosition({ -200.f, 0.f });
    std::cout << "Obstacle deactivated, audioID: " << m_audioID << std::endl;
}

void Obstacle::update(float deltaTime, float speedMultiplier, float obstacleSpeed, float playerX)
{
    //Dont Update if not actrive
    if (!active) return;

    shape.move({ -obstacleSpeed * deltaTime * speedMultiplier, 0 });

    float x = shape.getPosition().x;

    // Update RTPC
    float distance = std::abs(x - playerX);
    AK::SoundEngine::SetRTPCValue("EnemyDistance", distance, m_audioID);

    // Play sound once when obstacle enters screen
    if (!m_soundPlaying && x < 800.f && x + shape.getSize().x > 0.f)
    {
        AK::SoundEngine::PostEvent(AKTEXT("Play_EnemyBuzz"), m_audioID);
        AK::SoundEngine::PostTrigger(AKTEXT("Obstacle_Spawned_Stinger"), m_playerID);
        m_soundPlaying = true;
        std::cout << "Buzz started on audioID: " << m_audioID << std::endl;
    }

    // Deactivate when offscreen
    if (isOffScreen())
        deactivate();
}

void Obstacle::draw(sf::RenderWindow& window)
{
    if (active)
        window.draw(shape);
}

bool Obstacle::isOffScreen() const
{
    return shape.getPosition().x < -100.f;
}

sf::FloatRect Obstacle::getBounds() const
{
    return shape.getGlobalBounds();
}
