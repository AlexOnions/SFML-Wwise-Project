#include "Platform.h"
#include <cstdlib>

Platform::Platform(float startX, float floorY)
{
    //Set Platfrom heights and positions
    float width = 120 + rand() % 200;
    float height = 20;

    shape.setSize({ width, height });
    shape.setFillColor(sf::Color(120, 120, 120));

    float minGap = 20.f;   // minimum safe space above floor
    float maxGap = 120.f;  

    float y = floorY - (minGap + rand() % (int)(maxGap - minGap));

    shape.setPosition({ startX, y });
}

void Platform::update(float deltaTime, float speedMultiplier, float platformSpeed)
{
    shape.move({ -platformSpeed * deltaTime * speedMultiplier, 0 });
}

void Platform::draw(sf::RenderWindow& window)
{
    window.draw(shape);
}

bool Platform::isOffScreen() const
{
    return shape.getPosition().x < -200;
}

sf::FloatRect Platform::getBounds() const
{
    return shape.getGlobalBounds();
}
