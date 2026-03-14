#pragma once
#include <SFML/Graphics.hpp>

class Obstacle
{
private:
    sf::RectangleShape shape;
    uint64_t m_audioID;
    bool hasPlayedSound = false;

public:
    Obstacle(float startX, float platformTopY = -1.f, float gameSpeed = 1, int ID = 1000);
    ~Obstacle();

    void update(float deltaTime, float speedMultiplier, float obstacleSpeed);

    void draw(sf::RenderWindow& window);

    bool isOffScreen() const;

    sf::FloatRect getBounds() const;

};