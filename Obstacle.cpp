#include "Obstacle.h"
#include <cstdlib>
#include "WwiseWrapper.h"
#include <iostream>


Obstacle::Obstacle(float startX, float platformTopY, float gameSpeed, int ID)
{
    int type = 0;
    if (gameSpeed >= 2) {
         type = rand() % 3;
    }
    else if (gameSpeed >= 1.5) {
        type = rand() % 2;

    }
    if (type == 0)
    {
        shape.setSize({ 50, 50 });
        shape.setFillColor(sf::Color::Red);
       
    }
    else if (type == 1)
    {
        shape.setSize({ 20, 70 });
        shape.setFillColor(sf::Color::Yellow);
    }
    else
    {
        shape.setSize({ 80, 30 });
        shape.setFillColor(sf::Color::Magenta);
    }

    float y = (platformTopY >= 0.f)
        ? platformTopY - shape.getSize().y   // sit on top of platform
        : 340.f - shape.getSize().y;         // original ground level

    shape.setPosition({ startX, y });
    


    m_audioID = ID;



    // Play the sound immediately when spawned
    //AK::SoundEngine::PostEvent(AKTEXT("PlayObstacle"), m_audioID);



}
Obstacle::~Obstacle()
{
    AK::SoundEngine::UnregisterGameObj(m_audioID);
}



void Obstacle::update(float deltaTime, float speedMultiplier, float obstacleSpeed)
{
    shape.move({ -obstacleSpeed * deltaTime * speedMultiplier, 0 });


    float x = shape.getPosition().x;



    // -- - AUDIO POSITION UPDATE-- -
    AkSoundPosition pos;

    pos.SetPosition(shape.getPosition().x, 0, 0);
    pos.SetOrientation(1, 0, 0, 0, 1, 0);
    AK::SoundEngine::SetPosition(m_audioID, pos);

    // --- Play sound only when entering the screen ---
    if (!hasPlayedSound && x < 800 && x + shape.getSize().x > 0)
    {
        AK::SoundEngine::RegisterGameObj(m_audioID);
     
        std::cout << "Playing obstacle sound at x = " << x << std::endl;
        AK::SoundEngine::PostEvent(AKTEXT("PlayObstacle"), m_audioID);
        hasPlayedSound = true;
    }



}

void Obstacle::draw(sf::RenderWindow& window)
{
    window.draw(shape);
}

bool Obstacle::isOffScreen() const
{
    return shape.getPosition().x < -100;
}

sf::FloatRect Obstacle::getBounds() const
{
    return shape.getGlobalBounds();
}