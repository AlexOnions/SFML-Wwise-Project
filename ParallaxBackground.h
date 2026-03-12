#pragma once
#include <vector>
#include "ParallaxLayer.h"

class ParallaxBackground
{
public:
    void addLayer(float speed, float windowWidth, float windowHeight, sf::Color color);
    void update(float dt, float gameSpeed);
    void draw(sf::RenderWindow& window);
private:
    std::vector<ParallaxLayer> layers;
};