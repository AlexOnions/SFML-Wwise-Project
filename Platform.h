#pragma once
#include <SFML/Graphics.hpp>

class Platform
{
public:
    Platform(float startX, float floorY);

    void update(float deltaTime, float speedMultiplier, float platformSpeed);
    void draw(sf::RenderWindow& window);

    bool isOffScreen() const;
    sf::FloatRect getBounds() const;
    sf::RectangleShape shape;

private:
};
