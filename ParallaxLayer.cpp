#include "ParallaxLayer.h"
#include <cstdlib>

ParallaxLayer::ParallaxLayer(float scrollSpeed, float windowWidth, float windowHeight, sf::Color color)
{
    speed = scrollSpeed;
    width = windowWidth;

    int blockCount = 20;
    float slotWidth = (windowWidth * 2.f) / blockCount;  // divide space into even slots

    for (int i = 0; i < blockCount; i++)
    {
        float bw = 50.f + rand() % 51;
        float bh = 50.f + rand() % 51;

        // Place block within its slot with some randomness
        float x = i * slotWidth + (rand() % (int)(slotWidth * 0.6f));
        float y = 50.f + (rand() % (int)(windowHeight - 150));  // keep away from very top/bottom

        sf::RectangleShape block(sf::Vector2f(bw, bh));
        block.setFillColor(color);
        block.setOutlineThickness(2.f);
        block.setOutlineColor(sf::Color(100, 100, 100));
        block.setPosition(sf::Vector2f(x, y));
        blocks.push_back(block);
    }
}

void ParallaxLayer::update(float dt, float gameSpeed)
{
    float move = speed * gameSpeed * dt;
    for (auto& block : blocks)
    {
        sf::Vector2f pos = block.getPosition();
        pos.x -= move;

        // When a block scrolls off the left, wrap it to the far right
        if (pos.x + block.getSize().x < 0.f)
        {
            pos.x = width + (rand() % (int)(width * 0.8f));  // spread respawn across wider range
            pos.y = 50.f + rand() % 300;
        }
        block.setPosition(pos);
    }
}

void ParallaxLayer::draw(sf::RenderWindow& window)
{
    for (auto& b : blocks)
        window.draw(b);
}