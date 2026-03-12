#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class ParallaxLayer
{
public:
    ParallaxLayer(float scrollSpeed, float windowWidth, float windowHeight, sf::Color color);
    void update(float dt, float gameSpeed);
    void draw(sf::RenderWindow& window);

    float speed;
    float width;

private:
    std::vector<sf::RectangleShape> blocks;
};