#pragma once
#include <SFML/Graphics.hpp>

class Obstacle
{
private:
    sf::RectangleShape shape;

public:
    Obstacle(float startX);

    void update(float deltaTime, float speedMultiplier, float obstacleSpeed);

    void draw(sf::RenderWindow& window);

    bool isOffScreen() const;

    sf::FloatRect getBounds() const;
};