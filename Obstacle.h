#pragma once
#include <SFML/Graphics.hpp>

class Obstacle
{
private:
    sf::RectangleShape shape;

public:
    Obstacle(float startX, float platformTopY = -1.f, float gameSpeed = 1);

    void update(float deltaTime, float speedMultiplier, float obstacleSpeed);

    void draw(sf::RenderWindow& window);

    bool isOffScreen() const;

    sf::FloatRect getBounds() const;
};