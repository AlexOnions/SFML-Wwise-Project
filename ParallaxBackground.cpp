#include "ParallaxBackground.h"

void ParallaxBackground::addLayer(float speed, float windowWidth, float windowHeight, sf::Color color)
{
    layers.emplace_back(speed, windowWidth, windowHeight, color);
}

void ParallaxBackground::update(float dt, float gameSpeed)
{
    for (auto& layer : layers)
        layer.update(dt, gameSpeed);
}

void ParallaxBackground::draw(sf::RenderWindow& window)
{
    for (auto& layer : layers)
        layer.draw(window);
}