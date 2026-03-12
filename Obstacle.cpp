#include "Obstacle.h"
#include <cstdlib>

Obstacle::Obstacle(float startX)
{
    int type = rand() % 3;

    if (type == 0)
    {
        shape.setSize({ 20, 70 });
        shape.setFillColor(sf::Color::Yellow);
    }
    else if (type == 1)
    {
        shape.setSize({ 50, 50 });
        shape.setFillColor(sf::Color::Red);
    }
    else
    {
        shape.setSize({ 80, 30 });
        shape.setFillColor(sf::Color::Magenta);
    }

    shape.setPosition({ startX, 340 - shape.getSize().y });
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