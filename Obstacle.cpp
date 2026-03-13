#include "Obstacle.h"
#include <cstdlib>

Obstacle::Obstacle(float startX, float platformTopY, float gameSpeed)
{
    int type = 0;
    if (gameSpeed >= 3) {
         type = rand() % 3;
    }
    else if (gameSpeed >= 2) {
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
}

void Obstacle::update(float deltaTime, float speedMultiplier, float obstacleSpeed)
{
    shape.move({ -obstacleSpeed * deltaTime * speedMultiplier, 0 });
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